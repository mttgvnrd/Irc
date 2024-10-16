/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmogid <larmogid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 12:14:47 by mgiovana          #+#    #+#             */
/*   Updated: 2024/10/16 15:25:54 by larmogid         ###   ########.fr       */
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

// Creazione della socket NON BLOCCANTE (IPv4)
void Server::createSocket() {
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1) {
        std::cerr << "Error creating socket." << std::endl;
        exit(EXIT_FAILURE);
    }
        int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error in setsockopt." << std::endl;
        exit(EXIT_FAILURE);
    }
    // Configurazione dell'indirizzo del server
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_port);
    
    // Configurare la socket come non bloccante
    int flags = fcntl(_server_fd, F_SETFL, O_NONBLOCK);
    if (flags == -1) {
        std::cerr << "Error configuring non-blocking socket." << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Associazione della socket a una porta
void Server::bindSocket() {
    if (bind(_server_fd, (struct sockaddr*)&_address, sizeof(_address)) < 0) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
}

// Ascolto delle connessioni
void Server::listenConnections() {
    if (listen(_server_fd, 10) < 0) {
        std::cerr << "Error listening for connections." << std::endl;
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
            std::cerr << "Error in poll()." << std::endl;
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

void Server::handleJoinCommand(Client* client, const std::string& channelName) {
    // Controlla se il client è autenticato
    if (!client->isAuthenticated()) {
        std::string errorMsg = "ERR_NOTREGISTERED :You have not registered (NICK/USER missing)\n";
        send(client->getFd(), errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

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
        std::string joinMsg = ":" + client->getNickname() + " JOIN " + channelName + "\n";
        channel->broadcastMessage(joinMsg, client);  // Invia il messaggio di JOIN agli altri membri
        std::cout << " Client " << client->getNickname() << " joined to channel " << channelName << std::endl;
    } else {
        std::string errorMsg = "ERR_CHANNELFULL " + channelName + " :Cannot join channel, it is full.\n";
        send(client->getFd(), errorMsg.c_str(), errorMsg.size(), 0);
    }
}

// Accettare nuove connessioni
void Server::acceptNewClient() {
    int new_client_fd = accept(_server_fd, NULL, NULL);
    if (new_client_fd == -1) {
        std::cerr << "Error accepting connection." << std::endl;
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
void Server::handleClientMessage(int client_fd) {
    char buffer[1024];
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) {
        std::cout << "Client disconnected. FD: " << client_fd << std::endl;
        close(client_fd);
        _poll_fds.erase(
            std::remove_if(
                _poll_fds.begin(), _poll_fds.end(),
                [client_fd](const pollfd &pfd) { return pfd.fd == client_fd; }
            ),
            _poll_fds.end()
        );
        delete _clients_map[client_fd];  // Libera la memoria
        _clients_map.erase(client_fd);
        return;
    }

    buffer[bytes_received] = '\0';  // Termina la stringa ricevuta
    std::string command(buffer);

    // Processa il comando ricevuto
    handleCommand(command, _clients_map[client_fd]);
}

// Funzione per inviare un messaggio di errore
void Server::sendError(Client* client, const std::string& message) {
    std::string error_msg = "ERROR : " + message + "\n";
    send(client->getFd(), error_msg.c_str(), error_msg.size(), 0);
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

// Gestione dei comandi (NICK e USER)
void Server::handleCommand(const std::string& command, Client* client) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;
    
    if (cmd == "PASS") {
        std::string password;
        iss >> password;  // Estrai la password dal comando

        if (client->isAuthenticated()) {
            std::cerr << "Error: user " << client->getNickname() << " is already verified." << std::endl;
            std::string error_msg = "Error: user " + client->getNickname() + " is already verified.\n";
            send(client->getFd(), error_msg.c_str(), error_msg.size(), 0);
            return;  // Interrompi l'esecuzione se l'utente è già autenticato
        }
        else if (password.empty()) {
            std::cerr << "Error: no password provided." << std::endl;
            return;  // Interrompi se la password è vuota
        }
        else if (password != this->_password) {
            std::cerr << "Error: password does not match." << std::endl;
            return;  // Interrompi se la password non corrisponde
        }
        else {
            client->authenticate();  // Autenticazione del client
            std::cout << "Client " << client->getNickname() << " authenticated successfully." << std::endl;

            // Invia un messaggio di benvenuto o conferma al client
            std::string welcome_msg = "001 " + client->getNickname() + " :Welcome to the IRC server!\n";
            send(client->getFd(), welcome_msg.c_str(), welcome_msg.size(), 0);
        }
    }
    
    if (cmd == "NICK") {
        std::string nickname;
        iss >> nickname;

        if (isNicknameInUse(nickname)) {
            sendError(client, "Nickname " + nickname + " is already in use. Please choose a different name.");
            std::cerr << "Error: Nickname " << nickname << " already in use." << std::endl;
            return;
        }

        client->setNickname(nickname);
        _nickname_map[nickname] = client;
        std::cout << "Client " << client->getFd() << " has set nickname to: " << nickname << std::endl;
    }
    else if (cmd == "USER") {
        std::string username;
        iss >> username;  // Solo il primo argomento è considerato il nome utente
        client->setUsername(username);
        std::cout << "Client " << client->getFd() << " has set username to: " << username << std::endl;

        // Autenticazione del client (richiede sia NICK che USER)
        client->authenticate();
        if (client->isAuthenticated()) {
            std::cout << "Client " << client->getFd() << " authenticated!" << std::endl;
            std::string welcome_msg = "001 " + client->getNickname() + " :Benvenuto sul server IRC!\n";
            send(client->getFd(), welcome_msg.c_str(), welcome_msg.size(), 0);
        }
    }
    else if (cmd == "JOIN") {
        std::string channelName;
        iss >> channelName;
        handleJoinCommand(client, channelName);
    }
    else if (cmd == "PRIVMSG") {
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
                std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\n";
                channel->broadcastMessage(fullMsg, client);  // Invia il messaggio a tutti i membri del canale
                std::cout << "Messaggio da " << client->getNickname() << " nel canale " << target << ": " << message << std::endl;
            } else {
                std::string errorMsg = "ERR_CANNOTSENDTOCHAN " + target + " :You are not on that channel.\n";
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
            std::string full_message = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\n";
            send(target_client->getFd(), full_message.c_str(), full_message.size(), 0);
            std::cout << "Messaggio privato da " << client->getNickname() << " a " << target << ": " << message << std::endl;
        }
    }
    else if (cmd == "CAP") {
    // Ignora il comando CAP senza alcun output
    ;
    }
    else {
        std::cerr << "Comando non riconosciuto: " << cmd << std::endl;
    }
}