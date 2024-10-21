/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luigi <luigi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 12:14:47 by mgiovana          #+#    #+#             */
/*   Updated: 2024/10/21 08:59:50 by luigi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

// Costruttore
Server::Server(int port, const std::string& password) 
    : _port(port), _password(password) {
    
    // Controllo sulla porta
    if (port < 0 || port > USHRT_MAX) {
        throw std::invalid_argument("Invalid port.");
    } 
    // Controllo sulla password
    if (password.empty() || std::isspace(password[0]) || std::isspace(password[password.length() - 1])) {
        throw std::invalid_argument("The password cannot start or end with a space.");
    }
    // Verifica che la password non contenga ',' o '.'
    if (password.find(',') != std::string::npos || password.find('.') != std::string::npos) {
        throw std::invalid_argument("The password cannot contain ',' or '.'.");
    }
    // Inizializzazione della socket
    createSocket();
    bindSocket();
}


// Distruttore
Server::~Server() {
    stop();
}

// Funzione per inviare un messaggio di errore

/* Elimina std::cerr da messaggi di errore: Invece di stampare 
errori sul terminale con std::cerr, potresti considerare di 
registrare questi eventi in un file di log per facilitare il 
debugging e l'analisi post-mortem. */
void Server::sendError(Client* client, const std::string& message) {
    std::string error_msg = "ERROR : " + message + "\r\n"; // IRC requires \r\n at the end
    std::cerr << "Sending error to client FD " << client->getFd() << ": " << error_msg; // Logging error
    send(client->getFd(), error_msg.c_str(), error_msg.size(), 0);
}

// Creazione della socket NON BLOCCANTE (IPv4)
void Server::createSocket() {
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1) {
        std::cerr << "Failed to create socket." << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error in setsockopt." << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    // Configurazione dell'indirizzo del server
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_port);
    
    // Configurare la socket come non bloccante
    int flags = fcntl(_server_fd, F_SETFL, O_NONBLOCK);
    if (flags == -1) {
        std::cerr << "Failed to configure non-blocking socket." << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Associazione della socket a una porta
void Server::bindSocket() {
    if (bind(_server_fd, (struct sockaddr*)&_address, sizeof(_address)) < 0) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Ascolto delle connessioni
void Server::listenConnections() {
    if (listen(_server_fd, 10) < 0) {
        std::cerr << "Failed to listen for connections." << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on Port " << _port << std::endl;

    // Aggiungi il server_fd alla lista di file descriptor monitorati con poll()
    struct pollfd server_poll_fd;
    server_poll_fd.fd = _server_fd;
    server_poll_fd.events = POLLIN;
    _poll_fds.push_back(server_poll_fd);
}

// Ciclo di polling per gestire connessioni e I/O
void Server::pollConnections() {
    while (true) {
        int poll_count = poll(_poll_fds.data(), _poll_fds.size(), -1);
        if (poll_count == -1) {
            std::cerr << "Error in poll()." << strerror(errno) << std::endl;
            return;
        }

        // Scansiona i file descriptor
        for (size_t i = 0; i < _poll_fds.size(); ++i) {
            if (_poll_fds[i].revents & POLLIN) {
                if (_poll_fds[i].fd == _server_fd) {
                    // Nuova connessione da accettare
                    acceptNewClient();
                } else {
                    // Messaggio da un client esistente
                    handleClientMessage(_poll_fds[i].fd);
                }
            }
        }
    }
}

// Avvio del server
void Server::start() {
    listenConnections();
    pollConnections();
}

// Arresto del server
void Server::stop() {
    for (std::map<int, Client*>::iterator it = _clients_map.begin(); it != _clients_map.end(); ++it) {
        close(it->first); // Chiude il file descriptor del client
        delete it->second; // Libera la memoria del client
    }
    _clients_map.clear(); // Pulisci la mappa

    close(_server_fd); // Chiude il file descriptor del server
    std::cout << "Server closed." << std::endl;
}

void Server::handlePartCommand(Client* client, const std::string& channelName) {
    auto it = _channels.find(channelName);
    if (it == _channels.end()) {
        sendError(client, "No such channel: " + channelName);
        return;
    }

    Channel* channel = it->second;
    if (!channel->isMember(client)) {
        sendError(client, "You are not a member of " + channelName);
        return;
    }

    channel->removeMember(client);
    std::string partMsg = ":" + client->getNickname() + " PART " + channelName + "\r\n";
    channel->broadcastMessage(partMsg, client);
    std::cout << "Client " << client->getNickname() << " left channel " << channelName << std::endl;
}

void Server::handleJoinCommand(Client* client, const std::string& channelName) {

    Channel* channel;

    // Se il canale non esiste, crealo
    if (_channels.find(channelName) == _channels.end()) {
        channel = new Channel(channelName, client);  // Il creatore è il primo membro e l'operatore
        _channels[channelName] = channel;
        _channels.insert(std::make_pair(channelName, channel));
        std::cout << "Channel " << channelName << " created by " << client->getNickname() << std::endl;
    } else {
        channel = _channels[channelName];
    }

    // Aggiungi il client al canale se non è pieno
    if (channel->addMember(client)) {
        std::string joinMsg = ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
        channel->broadcastMessage(joinMsg, client);  // Invia il messaggio di JOIN agli altri membri
        std::cout << "Client " << client->getNickname() << " joined to channel " << channelName << std::endl;
    } else {
        std::string errorMsg = "ERR_CHANNELFULL " + channelName + " :Cannot join channel, it is full.\r\n";
        send(client->getFd(), errorMsg.c_str(), errorMsg.size(), 0);
    }
}

// Accettare nuove connessioni
void Server::acceptNewClient() {
    int new_client_fd = accept(_server_fd, NULL, NULL);
    if (new_client_fd == -1) {
        std::cerr << "Failed to accept connection." << std::endl;
        return;
    }

    // Aggiungi il nuovo client alla lista di file descriptor monitorati
    struct pollfd client_poll_fd;
    client_poll_fd.fd = new_client_fd;
    client_poll_fd.events = POLLIN;
    _poll_fds.push_back(client_poll_fd);

    // Creare e aggiungere un nuovo client
    Client* new_client = new Client(new_client_fd);
    _clients_map[new_client_fd] = new_client;

    std::cout << "New client connected! FD: " << new_client_fd << std::endl;
}

// Gestione dei messaggi del client

/* Gestione della disconnessione: Nella funzione handleClientMessage, 
si potrebbe gestire meglio la disconnessione dei client, assicurandoti che tutti i 
membri del canale siano avvisati. */
void Server::handleClientMessage(int client_fd) {
    char buffer[1024];
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received < 0) {
        // Errore durante la ricezione dei dati
        std::cerr << "Failed to receive data from client FD " << client_fd << ": " << strerror(errno) << std::endl;
        sendError(_clients_map[client_fd], "An error occurred while receiving your message.");
        return;
    } else if (bytes_received >= sizeof(buffer) - 1) {
        sendError(_clients_map[client_fd], "Message too long.");
        return;
    } else if (bytes_received == 0) {
        // Il client ha chiuso la connessione
        std::cout << "Client disconnected. FD: " << client_fd << std::endl;
        close(client_fd);
        _poll_fds.erase(std::remove_if(_poll_fds.begin(), _poll_fds.end(),
            [client_fd](const pollfd &pfd) { return pfd.fd == client_fd; }),
                _poll_fds.end());
        delete _clients_map[client_fd];  // Libera la memoria
        _clients_map.erase(client_fd);
        return;
    }

    buffer[bytes_received] = '\0';  // Termina la stringa ricevuta
    std::string command(buffer);

    //---------------------------------------------------------------------------------------> Stampa il messaggio ricevuto come debug, limitato ai byte ricevuti
    std::cout << "Debug: Received message from client FD " << client_fd << ": " << buffer << std::endl;
    
    // Processa il comando ricevuto
    try {
        handleCommand(command, _clients_map[client_fd]);
    } catch (const std::exception& e) {
        // Gestione di eventuali eccezioni lanciate durante la elaborazione del comando
        std::cerr << "Failed to process command from client FD " << client_fd << ": " << e.what() << std::endl;
        sendError(_clients_map[client_fd], "An error occurred while processing your request.");
    }
}


void Server::handleQuitCommand(Client* client) {
    int client_fd = client->getFd();

    // Notifica i canali della disconnessione
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        Channel* channel = it->second;
        if (channel->isMember(client)) {
            std::string partMessage = ":" + client->getNickname() + " QUIT\n";
            channel->broadcastMessage(partMessage, client);  // Notifica agli altri membri
            channel->removeMember(client);  // Rimuovi il client dal canale
        }
    }

    std::cout << "Client " << client->getNickname() << " has disconnected" << std::endl;

    // Chiudi il file descriptor del client
    close(client_fd);

    // Rimuovi il client dalla lista di pollfd
    _poll_fds.erase(std::remove_if(_poll_fds.begin(), _poll_fds.end(),
        [client_fd](const pollfd &pfd) { return pfd.fd == client_fd; }),
            _poll_fds.end());

    // Libera la memoria del client
    delete client;
    //delete _clients_map[client_fd];
    _clients_map.erase(client_fd);

    std::cout << "Client FD " << client_fd << " removed from server." << std::endl;
}

// Gestione dei comandi (NICK e USER)

/* Controllo di stato: Puoi considerare di avere un metodo per 
verificare lo stato del client per semplificare la gestione dei 
comandi in handleCommand. */

/* Gestione delle eccezioni: La gestione delle eccezioni in 
handleCommand è un buon inizio. Assicurati che tutte le funzioni 
critiche (come send, recv, ecc.) abbiano una gestione delle 
eccezioni adeguata per evitare crash inaspettati. */
void Server::handleCommand(const std::string& command, Client* client) {
    // Separazione del comando dal resto del messaggio
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd; // Estrae il comando principale
    
    // Verifica se il client è verificato tramite password
    if (!client->isVerified()) {
        if (cmd == "PASS") {
            std::string password;
            iss >> password;

            if (password.empty()) {
                sendError(client, "No password provided.");
                return;
            }

            if (password == this->_password) {
                client->setVerified(true); // Verifica il client
                const char* msg = "Password accepted. Please provide your nickname.\r\n";
                send(client->getFd(), msg, strlen(msg), 0);
            } else {
                sendError(client, "Password does not match.");
            }
        } else if (!client->isPasswordRequestSent()) {
            std::string invite_msg = "464 :Please enter your password to verify.\r\n";
            send(client->getFd(), invite_msg.c_str(), invite_msg.size(), 0);
            client->setPasswordRequestSent(true);
        }
        return;
        } else if (cmd == "NICK") { // Gestione del comando NICK
            std::string nickname;
            iss >> nickname;

            if (nickname.empty()) {
                sendError(client, "No nickname provided. Please choose a nickname.");
                return;
            }

            if (isNicknameInUse(nickname)) {
                sendError(client, "Nickname " + nickname + " is already in use. Please choose a different name.");
                return;
            }

            client->setNickname(nickname);
            _nickname_map[nickname] = client;
            std::cout << "Client " << client->getFd() << " has set nickname to: " << nickname << std::endl;

            // Invita il client a inserire lo username dopo il nickname
            std::string invite_msg = "461 " + nickname + " :Please provide your Username to proceed.\r\n";
            send(client->getFd(), invite_msg.c_str(), invite_msg.size(), 0);

            // Controllo se il client è già autenticato (richiede anche lo USER)
            client->authenticate();
        } else if (cmd == "USER") { // Gestione del comando USER
            if (!client->isNicknameSet()) {
                std::string invite_msg = "461 " + client->getNickname() + " :Please insert your Nickname before providing a username.\r\n";
                send(client->getFd(), invite_msg.c_str(), invite_msg.size(), 0);
                return;
            }

            std::string username;
            iss >> username;

            if (username.empty()) {
                sendError(client, "No username provided. Please provide a username.");
                return;
            }

            client->setUsername(username);
            std::cout << "Client " << client->getFd() << " has set username to: " << username << std::endl;

            // Controllo se il client è autenticato
            client->authenticate();

            if (client->isAuthenticated() && !client->isWelcomeMessageSent()) {
                if (client->isAuthenticated() && !client->isWelcomeMessageSent()) {
                std::string welcome_msg = ":server 001 " + client->getNickname() + " :Welcome to the IRC server, " + client->getNickname() + "!\r\n";
                std::cout << "Welcome message: " << welcome_msg << std::endl; // Debug

                send(client->getFd(), welcome_msg.c_str(), welcome_msg.size(), 0);
                client->setWelcomeMessageSent(true);
            }
        } else if (cmd == "JOIN") {
            std::string channelName;
            iss >> channelName;
            if (channelName.empty()) {
                sendError(client, "No channel name provided.");
                return;
            }
            handleJoinCommand(client, channelName);
        } else if (cmd == "PART") {//USCITA DAL CANALE
            std::string channelName;
            iss >> channelName;

            if (channelName.empty()) {
                sendError(client, "No channel name provided.");
                return;
                }
            
            // Gestisci la logica di uscita dal canale
            //handlePartCommand(client, channelName);
        } else if (cmd == "PRIVMSG") {

            if (!client->isVerified()){
                std::cerr << "Error: client " << client->getFd() << " is not verified." << std::endl;
                std::string error_msg = "Error: you aren't verified.\n";
                send(client->getFd(), error_msg.c_str(), error_msg.size(), 0);
                return;
            }

            std::string target;
            std::string message;

            // Estrazione del target (nickname o canale)
            iss >> target;
            
            // Controllo se è stato specificato un destinatario
            if (target.empty()) {
                std::cerr << "Errore: destinatario non specificato per PRIVMSG." << std::endl;
                return;
            }

            // Estrazione del messaggio
            iss >> message;
            if (!message.empty() && message[0] == ':') {
                message.erase(0, 1);
                std::string rest_of_message;
                std::getline(iss, rest_of_message);
                message += rest_of_message;
            }

            // Gestione messaggi verso un canale
            if (_channels.find(target) != _channels.end()) {
                Channel* channel = _channels[target];

                // Verifica se il client è un membro del canale
                if (channel->isMember(client)) {
                    std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
                    channel->broadcastMessage(fullMsg, client);  // Invia il messaggio a tutti i membri del canale
                    std::cout << "Messaggio da " << client->getNickname() << " nel canale " << target << ": " << message << std::endl;
                } else {
                    std::string errorMsg = "ERR_CANNOTSENDTOCHAN " + target + " :You are not on that channel.\r\n";
                    send(client->getFd(), errorMsg.c_str(), errorMsg.size(), 0);
                }
            }
            // Gestione messaggi verso un altro client
            else {
                Client* target_client = nullptr;

                // Cerca il client destinatario
                for (std::map<int, Client*>::iterator it = _clients_map.begin(); it != _clients_map.end(); ++it) {
                    if (it->second->getNickname() == target) {
                        target_client = it->second;
                        break;
                    }
                }

                // Verifica se il destinatario esiste
                if (target_client == nullptr) {
                    std::cerr << "Errore: Il destinatario " << target << " non esiste." << std::endl;
                    return;
                }

                // Invia il messaggio al client
                std::string full_message = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
                send(target_client->getFd(), full_message.c_str(), full_message.size(), 0);
                std::cout << "Messaggio privato da " << client->getNickname() << " a " << target << ": " << message << std::endl;
            }
        }
        else if (cmd == "CAP") {
            if (!client->isVerified()){
                std::cerr << "Error: client " << client->getFd() << " is not verified." << std::endl;
                std::string error_msg = "Error: you aren't verified.\n";
                send(client->getFd(), error_msg.c_str(), error_msg.size(), 0);
                return;
            }
            // Ignora il comando CAP senza alcun output
            ;
        }
        else if (cmd == "QUIT") {
            handleQuitCommand(client);
        }
        else {
            sendError(client, "Unknown command.");
        }
    }
}

// Controllo se un nickname è già in uso
bool Server::isNicknameInUse(const std::string& nickname) {
    for (std::map<int, Client*>::iterator it = _clients_map.begin(); it != _clients_map.end(); ++it) {
        if (it->second->getNickname() == nickname) {
            return true;  // Il nickname è già in uso
        }
    }
    return false;  // Il nickname non è in uso
}
