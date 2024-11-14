/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luigi <luigi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 14:46:57 by larmogid          #+#    #+#             */
/*   Updated: 2024/11/11 00:05:30 by luigi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Channel.hpp"
#include "ClientInstance.hpp"
#include "Utils.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <iostream>
#include <string>

#define MSG (MSG_DONTWAIT | MSG_NOSIGNAL)
#define SRV_NAME "ircserv"

Server::Server(void) : _serverName(SRV_NAME) {}

Server::Server(int port, const std::string& password) : _serverName(SRV_NAME) {
	if (port >= 0 && port <= USHRT_MAX) {
		this->_port = port;
	}
	else {
		throw ("invalid port");
	}
	if (isProperPassword(password)) {
		this->_pw = password;
	}
	else {
		throw ("invalid password");
	}
}

Server::Server(const std::string& port, const std::string& password) : _serverName(SRV_NAME) {
	if (isVerifiedPort(port)) {
		this->_port = std::atoi(port.c_str());
	}
	else {
		throw ("invalid port");
	}
	if (isProperPassword(password)) {
		this->_pw = password;
	}
	else {
		throw ("invalid password");
	}
}

Server::~Server(void) {
    close(this->_serverSocket);
    this->_fds.clear();
    
    // Liberare i canali
    for (std::map<std::string, Channel*>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it) {
        delete (it->second);
    }
    this->_channels.clear();
    
    // Liberare gli utenti
    for (std::map<int, ClientInstance*>::iterator it = this->_users.begin(); it != this->_users.end(); ++it) {
        delete (it->second);
    }
    this->_users.clear();
    
    /* Non è necessario liberare memoria per i messaggi di errore:
    la mappa dei messaggi di errore verrà liberata automaticamente */
}

// Operators only:
// format --> KICK <#channel> <nickname> [<message>]
void	Server::handleKick(std::vector<std::string> argv, ClientInstance* user) {
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451); // user isn't authenticated
		return;
	}
	if (argv.size() < 3) {
		this->errorMsg(user, 461); // user didn't provide channel and/or nickname
		return ;
	}
	std::string channelName = toLowerCase(argv[1]);
	if (channelName[0] == '#') { // <-- if there's no "#" symbol then error?
		channelName.erase(0, 1); // remove "#"
	}
	Channel * channel = getChannelByName(channelName);
	if (!channel) {
		this->errorMsg(user, 403); // channel doesn't exist
	}
	else if (!channel->isUserOperator(user->getNickName())) {
		this->errorMsg(user, 482); // user is not operator
	}
	else if (!channel->isUserIn(argv[2])) {
		this->errorMsg(user, 441); // target_user is not in the channel
	}
	else {
		std::string rplKick = ":" + user->getNickName() + "!" + this->getName() + " " + argv[0] + " #" + channelName + " " + argv[2];
		if (argv.size() > 3 && !argv[3].empty()) {
			rplKick += " :" + argv[3]; // append optional message
		}
		rplKick += "\r\n";
		this->channelBroadcast(channel, NULL, rplKick);
		channel->removeUser(argv[2]);
	}
}

// format --> INVITE <nickname> <channel>
void	Server::handleInvite(std::vector<std::string> argv, ClientInstance* user) {
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451); // user isn't authenticated
		return ;
	}
	if (argv.size() < 3) {
		this->errorMsg(user, 461); // user didn't provide nickname and/or channel 
		return ;
	}
	std::string channelName = toLowerCase(argv[2]);
	if (channelName[0] == '#') { // if there's no "#" symbol then error?
		channelName.erase(0, 1); // remove "#"
	}
	Channel * channel = this->getChannelByName(channelName);
	if (!channel) {
		this->errorMsg(user, 403); // channel not found
	}
	else if (!channel->isUserOperator(user->getNickName())) {
		this->errorMsg(user, 482); // user is not operator
	}
	else if (channel->isUserIn(argv[1])) {
		this->errorMsg(user, 443); // :user_to_invite is already in channel
	}
	else if (channel->hasFlag('l') && channel->getNbrOfUsers() >= channel->getUsersLimit()) {
		this->errorMsg(user, 471); // too many users
	}
	else {
		channel->addUser(argv[1]);
		ClientInstance* userInv = getUserByNickName(argv[1]);
		if (userInv) {
			this->joinMsg(channel, userInv);
		}
	}
}

// format --> TOPIC <channel> [<topic>]
void	Server::handleTopic(std::vector<std::string> argv, ClientInstance* user) {
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451); // user isn't authenticated
		return;
	}
	if (argv.size() < 2) {
		this->errorMsg(user, 463); // user didn't send channel
		return ;
	}
	std::string channelName = toLowerCase(argv[1]);
	if (channelName[0] == '#') { // if there's no "#" symbol then error?
		channelName.erase(0, 1); // remove "#"
	}
	Channel * channel = this->getChannelByName(channelName);
	if (!channel) {
		this->errorMsg(user, 403); // channel not found
	}
	else if (channel->hasFlag('t') && !channel->isUserOperator(user->getNickName())) {
		this->errorMsg(user, 482); // restrictions are set but user is not operator
	}
	else if (argv.size() > 2) { // set topic
		channel->setTopic(argv[2]);
		std::string rplTopic;
		rplTopic = ":" + user->getNickName() + "!" + this->getName() + " " + argv[0] + " #" + channel->getName() + " :" + channel->getTopic() + "\r\n";
		this->channelBroadcast(channel, NULL, rplTopic);
	}
	else { // view topic
		std::string rplTopic;
		if (channel->getTopic().empty()) {
			rplTopic = ":" + this->getName() + " 331 " + user->getNickName() + " #" + channel->getName() + " :No topic is set" + "\r\n";
		}
		else {
			rplTopic = ":" + this->getName() + " 332 " + user->getNickName() + " #" + channel->getName() + " :" + channel->getTopic() + "\r\n";
		}
		send(user->getFd(), rplTopic.c_str(), rplTopic.length(), MSG);
	}
}

// format -->  MODE <channel> {[+|-]|i|t|k|o|l} 
void	Server::handleMode(std::vector<std::string> argv, ClientInstance* user) {
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451); // user isn't authenticated
		return;
	}
	if (argv.size() < 2) {
		this->errorMsg(user, 463); // user didn't send channel or flags
		return ;
	}
	std::string channelName = toLowerCase(argv[1]);
	if (channelName[0] == '#') { // if there's no "#" symbol then error?
		channelName.erase(0, 1); // remove "#"
	}
	Channel * channel = this->getChannelByName(channelName);
	if (!channel) {
		this->errorMsg(user, 403); //error: channel not found
	}
	else if (!channel->isUserOperator(user->getNickName())) {
		this->errorMsg(user, 482); //error: user is not operator
	}
	else if (argv.size() < 3) { // view mode
		std::string rplMode = ":" + this->getName() + " 324 " + user->getNickName() + " #" + channelName + " +" + channel->getMode();
		for (size_t i = 0; i < channel->getMode().length(); ++i) {
			if (channel->getMode()[i] == 'l') {
				rplMode += " " + toString(channel->getUsersLimit());
			}
			else if (channel->getMode()[i] == 'k' && channel->getPassword().length() > 0) {
				rplMode += " " + channel->getPassword();
			}
		}
		rplMode += "\r\n";
		send(user->getFd(), rplMode.c_str(), rplMode.length(), MSG);
	}
	else if (argv[2][0] != '+' && argv[2][0] != '-') {
		this->errorMsg(user, 472); // flags dont start with +/-
	}
	else {
		std::string flags = argv[2];
		size_t argIndex = 3;
		for (size_t i = 1; i < flags.length(); i++) {
			if (flags[i] == 'o') {
				if (argv.size() > argIndex) {
					if (channel->isUserIn(argv[argIndex])) {
						if (flags[0] != '-') {
							channel->upgradeUserPriviledges(argv[argIndex]);
						}
						else {
							channel->downgradeUserPriviledges(argv[argIndex]);
						}
						std::string rplMsg = ":" + this->getName() + " " + argv[0] + " #" + channel->getName() + " " + flags[0] + flags[i] + " " + argv[argIndex] + "\r\n"; 
						this->channelBroadcast(channel, NULL, rplMsg);
					}
					else {
						this->errorMsg(user, 441); // user not in channel
					}
					argIndex++;
				}
				else {
					this->errorMsg(user, 407); // missing nickname
				}
			}
			else if (flags[i] == 'l') {
				if (flags[0] != '-') {
					if (argv.size() > argIndex) {
						if (std::atoi(argv[argIndex].c_str()) > 0) {
							channel->setUsersLimit(std::atoi(argv[argIndex].c_str()));
							channel->addMode('l');
							std::string rplMsg = ":" + this->getName() + " " + argv[0] + " #" + channel->getName() + " " + flags[0] + flags[i] + " " + argv[argIndex] + "\r\n"; 
							this->channelBroadcast(channel, NULL, rplMsg);
						}
						else {
							this->errorMsg(user, 461); // invalid users_limit
						}
						argIndex++;
					}
					else {
						this->errorMsg(user, 462); // missing users_limit
					}
				}
				else {
					channel->removeMode(flags[i]);
					std::string rplMsg = ":" + this->getName() + " " + argv[0] + " #" + channel->getName() + " " + flags[0] + flags[i] + "\r\n"; 
					this->channelBroadcast(channel, NULL, rplMsg);
				}
			}
			else if (flags[i] == 'k') {
				if (flags[0] != '-') {
					if (argv.size() > argIndex) {
						if (isProperPassword(argv[argIndex])) {
							channel->setPassword(argv[argIndex]);
							channel->addMode('k');
							std::string rplMsg = ":" + this->getName() + " " + argv[0] + " #" + channel->getName() + " " + flags[0] + flags[i] + " " + argv[argIndex] + "\r\n"; 
							this->channelBroadcast(channel, NULL, rplMsg);
						}
						else {
							this->errorMsg(user, 464); // invalid PW
						}
						argIndex++;
					}
					else {
						this->errorMsg(user, 461); // missing PW (insufficient params)
					}
				}
				else {
					channel->removeMode(flags[i]);
					std::string rplMsg = ":" + this->getName() + " " + argv[0] + " #" + channel->getName() + " " + flags[0] + flags[i] + "\r\n"; 
					this->channelBroadcast(channel, NULL, rplMsg);
				}
			}
			else if (flags[i] == 'i' || flags[i] == 't') {
				if (flags[0] != '-') {
					channel->addMode(flags[i]);
				}
				else {
					channel->removeMode(flags[i]);
				}
				std::string rplMsg = ":" + this->getName() + " " + argv[0] + " #" + channel->getName() + " " + flags[0] + flags[i] + "\r\n"; 
				this->channelBroadcast(channel, NULL, rplMsg);
			}
			else if (flags[i] == 'b') {
				; //ignore flag b
			}
			else {
				this->errorMsg(user, 472); // unknown flag
			}
		}
	}
}

//send a message to all users in channel
void	Server::channelBroadcast(Channel * channel, ClientInstance* user, const std::string & msg) const {
	if (!channel) {
		return;
	}
	// user is to be ignored if not NULL
	for (std::map<std::string, bool>::const_iterator it = channel->getUsers().begin(); it != channel->getUsers().end(); ++it) {
		if (!user || user->getNickName() != it->first) {
			ClientInstance* userDest = this->getUserByNickName(it->first);
			if (userDest) {
				send(userDest->getFd(), msg.c_str(), msg.length(), MSG);
			}
			else {
				; // user not online
			}
		}
	}
}

//send a message to all users in server
// user is to be ignored if not NULL
void	Server::serverBroadcast(ClientInstance* user, const std::string & msg) const {
	for (std::map<int, ClientInstance*>::const_iterator it = this->_users.begin(); it != _users.end(); ++it) {
		if (!user || user != it->second) {
			send(it->second->getFd(), msg.c_str(), msg.length(), MSG);
		}
	}
}

void Server::joinMsg(Channel *channel, ClientInstance* user) {
	std::string rplJoin = ":" + user->getNickName() + "!" + this->getName() + " JOIN #" + channel->getName() + "\r\n";
    std::string	rplUserList = ":" + this->getName() + " 353 " + user->getNickName() + " = #" + channel->getName() + " :";
	//user_list
	for (std::map<std::string, bool>::const_iterator it = channel->getUsers().begin(); it != channel->getUsers().end(); ++it) {
		ClientInstance* userDest = this->getUserByNickName(it->first);
		if (userDest) {
			send(userDest->getFd(), rplJoin.c_str(), rplJoin.length(), MSG);
		}
		rplUserList += it->first;
		if(it != --channel->getUsers().end()) {
			rplUserList += " ";
		}
	}
	rplUserList += "\r\n";
	//topic
	std::string rplTopic;
	if (channel->getTopic().empty()) {
		rplTopic = ":" + this->getName() + " 331 " + user->getNickName() + " #" + channel->getName() + " :No topic is set" + "\r\n";
	}
	else {
		rplTopic = ":" + this->getName() + " 332 " + user->getNickName() + " #" + channel->getName() + " :" + channel->getTopic() + "\r\n";
	}
	//mode
    std::string rplMode = ":" + this->getName() + " 324 " + user->getNickName() + " #" + channel->getName() + " +" + channel->getMode();
	for (size_t i = 0; i < channel->getMode().length(); ++i) {
		if (channel->getMode()[i] == 'l') {
			rplMode += " " + toString(channel->getUsersLimit());
		}
		else if (channel->getMode()[i] == 'k' && channel->getPassword().length() > 0) {
			rplMode += " " + channel->getPassword();
		}
	}
	rplMode += "\r\n";

	send(user->getFd(), rplTopic.c_str(), rplTopic.length(), MSG);
    send(user->getFd(), rplMode.c_str(), rplMode.length(), MSG);
	send(user->getFd(), rplUserList.c_str(), rplUserList.length(), MSG);
	
	for (std::map<std::string, bool>::const_iterator it = channel->getUsers().begin(); it != channel->getUsers().end(); ++it) {
		if (channel->isUserOperator(it->first)) {
   			std::string rplOpList = ":" + this->getName() + " MODE #" + channel->getName() + " +o " + it->first + "\r\n";
			send(user->getFd(), rplOpList.c_str(), rplOpList.length(), MSG);
		}
	}
}

void	Server::welcomeMsg(ClientInstance* user) {
	std::string nickname = user->getNickName();
	int fd = user->getFd();

	std::string RPL_WELCOME =	":" + this->getName() + " 001 " + nickname + " :Welcome to " + nickname + "!\r\n";
	send(fd, RPL_WELCOME.c_str(), RPL_WELCOME.length(), MSG);

	std::string RPL_YOURHOST =	":" + this->getName() + " 002 " + nickname + " :Your host is " + this->_hostname + "\r\n";
	send(fd, RPL_YOURHOST.c_str(), RPL_YOURHOST.length(), MSG);

	std::string RPL_MYINFO = 	":" + this->getName() + " 004 " + nickname + " :Details - User modes: +o, Channel modes: +l+i+k+t\r\n";
	send(fd, RPL_MYINFO.c_str(), RPL_MYINFO.length(), MSG);

	std::string RPL_MOTD =		":" + this->getName() + " 372 " + nickname + " :Message of the Day - Welcome to " + this->getName() + ", part of the IRC 42 Project\r\n";
	send(fd, RPL_MOTD.c_str(), RPL_MOTD.length(), MSG);

	std::string RPL_ENDOFMOTD =	":" + this->getName() + " 376 " + nickname + " :End of Message of the Day (MOTD) command\r\n";
	send(fd, RPL_ENDOFMOTD.c_str(), RPL_ENDOFMOTD.length(), MSG);
}

// format --> :<serv_name> <error_code> <nickname> :<message>
void Server::errorMsg(ClientInstance* user, int code) {
    std::string rplErr = ":" + this->getName() + " " + toString(code);

    if (!user->getNickName().empty()) {
        rplErr += " " + user->getNickName();
    } else {
        rplErr += " you";
    }

    // Utilizza la mappa errorMessages per ottenere il messaggio
    std::string errorMessage = getErrorMessage(code);
    rplErr += " :" + errorMessage + "\r\n";

    send(user->getFd(), rplErr.c_str(), rplErr.length(), MSG);
}

void Server::handleCommand(std::vector<std::string> argv, ClientInstance* user) {
    if (argv.empty()) {
        return;
    }

    // Funzione helper per associare comandi a handler
    void (Server::*handler)(std::vector<std::string>, ClientInstance*) = NULL;

    if (argv[0] == "PING") {
        handler = &Server::handlePing;
    } else if (argv[0] == "QUIT") {
        handler = &Server::handleQuit;
    } else if (argv[0] == "PASS") {
        handler = &Server::handlePass;
    } else if (argv[0] == "USER") {
        handler = &Server::handleUser;
    } else if (argv[0] == "NICK") {
        handler = &Server::handleNick;
    } else if (argv[0] == "JOIN") {
        handler = &Server::handleJoin;
    } else if (argv[0] == "PART") {
        handler = &Server::handlePart;
    } else if (argv[0] == "PRIVMSG") {
        handler = &Server::handlePrivMsg;
    } else if (argv[0] == "INVITE") {
        handler = &Server::handleInvite;
    } else if (argv[0] == "TOPIC") {
        handler = &Server::handleTopic;
    } else if (argv[0] == "KICK") {
        handler = &Server::handleKick;
    } else if (argv[0] == "MODE") {
        handler = &Server::handleMode;
    } else if (argv[0] != "CAP" && argv[0] != "WHO" && argv[0] != "USERHOST") {
        this->errorMsg(user, 421); // Comando sconosciuto
        return;
    }

    // Esegue l'handler, se trovato
    if (handler) {
        (this->*handler)(argv, user);
    }
}

/* Passing an open descriptor between jobs allows one process (typically a server) 
to do everything that is required to obtain the descriptor, such as opening a file, 
establishing a connection, and waiting for the accept() API to complete. It also 
allows another process (typically a worker) to handle all the data transfer operations 
as soon as the descriptor is open. */

int Server::handleMessage(int userFd) {
	ClientInstance* ptr = this->getUserByFd(userFd);
	if (!ptr) {	// you never know how it could happen...
		return (-1);
	}
	char	buffer[1024];
	
	// ssize_t recv(int sockfd, void *buf, size_t len, int flags);
	// buff -->  buffer in cui i dati letti dal socket verranno memorizzati.
	ssize_t bytesRead = recv(userFd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead <= 0) {
		this->removeUser(userFd);
		return (1);
	}
	buffer[bytesRead] = '\0'; // BEWARE!
	//std::cout << "bytes received = " << bytesRead << ", " << buffer << std::endl; // DEBUG
	ptr->setMessage(ptr->getMessage() + buffer);
	if (ptr->getMessage().find('\n') == std::string::npos) {
		return (0);
	}
	std::cout << "FD[" << ptr->getFd() << "]: " << ptr->getMessage() << std::endl;
	std::vector<std::string> argv = parseInput(ptr->getMessage());
	this->handleCommand(argv, ptr);
	if (!argv.empty() && argv[0] == "QUIT") {
		return (1);
	}
	ptr->setMessage("");
	return (0);
}

void	Server::run(void) {
	std::cout << "Server running at " << _ip << ":" << _port << std::endl;

	pollfd	serverPollFd;
	serverPollFd.fd = this->_serverSocket;
	serverPollFd.events = POLLIN; // Data may be read without blocking.
	serverPollFd.revents = 0;
	this->_fds.push_back(serverPollFd);

	this->_isRunning = true;
	while (this->isRunning() == true) {
		poll(this->_fds.data(), this->_fds.size(), -1);
		for (size_t i = 0; i < this->_fds.size(); ++i) {
			if (this->_fds[i].revents & POLLIN) {
				if (this->_fds[i].fd == this->_serverSocket) {
					this->createUser();
				}
				else if (this->handleMessage(this->_fds[i].fd) == 1) {
					--i; // if client disconnects, its pollfd is removed from the vector, so the next pollfd is at the index of the one that got removed. 
				}
			}
		}
	}
}

void	Server::init(void) {
	// Create a socket: int socket(int domain, int socktype, int protocol); 
	// AF_INET --> IPv4 type address; SOCK_STREAM --> TCP socket; 0 --> "auto" (TCP or UDP protocol)
	this->_serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Returns a file descriptor for the new socket, or -1 for errors.
    if (this->_serverSocket == -1) {
		throw ("socket creation failure");
    }
	
	// Set socket options (SO_REUSEADDR ---> bind)
    int opt = 1;
    if (setsockopt(this->_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw ("socket option settings failure");
    }
	
	// Get hostname
	if (gethostname(this->_hostname, sizeof(this->_hostname)) == -1) {
		throw ("gethostname failure");
	}

	// Get IP
	struct hostent *host_entry = gethostbyname(this->_hostname);
	if (host_entry == NULL) {
		throw ("gethostbyname failure");
	}
	
	this->_ip = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    // Set up server address structure
	this->_serverAddr.sin_family = AF_INET;
    this->_serverAddr.sin_addr.s_addr = INADDR_ANY;	// Listen on all available interfaces
    this->_serverAddr.sin_port = htons(this->_port);
 
 	// F_SETFL Sets status flags for the descriptor; 
	// O_NONBLOCK Non-blocking mode. Set flag on fd (O_NONBLOCK) 
	// If this flag is 1, read or write operations on the file will not cause the thread to block.
	if (fcntl(this->_serverSocket, F_SETFL, O_NONBLOCK) < 0) { // Perform File Control Command
		throw ("non-blocking socket setting failure");
	}
	
	// Associates a local address with a socket; returns 0 in case of success, -1 in case of failure
    // int bind(int __fd, const sockaddr *__addr, socklen_t __len)
	if (bind(this->_serverSocket, (struct sockaddr*)&(this->_serverAddr), sizeof(this->_serverAddr)) == -1) {
        throw ("socket binding failure");
    }
	
    // Listen for incoming connections
	//int listen(int __fd, int __n)
    if (listen(this->_serverSocket, SOMAXCONN) == -1) {
        throw ("socket listening failure");
    }
}

//getters
const std::string &	Server::getName(void) const {
	return (this->_serverName);
}

const std::string &	Server::getPassword(void) const {
	return (this->_pw);
}

int	Server::getPort(void) const {
	return (this->_port);
}

int	Server::getNbrOfUsers(void) const {
	return (this->_users.size());
}

int	Server::nChannels(void) const {
	return (this->_channels.size());
}

bool	Server::isRunning(void) const {
	return (this->_isRunning);
}

//setters
void	Server::setPassword(const std::string &password) {
	this->_pw = password;
}

void	Server::stop(void) {
	this->_isRunning = false;
}

//channels
Channel *	Server::getChannelByName(const std::string & channelName) const {
	if (this->_channels.find(channelName) != this->_channels.end()) {
		return (this->_channels.find(channelName)->second);
	}
	else {
		return (NULL);
	}
}
Channel *	Server::createChannel(const std::string & channelName) {
	if (this->_channels.find(channelName) == this->_channels.end()) {
		Channel * newChannel = new Channel(channelName);
		this->_channels.insert(std::make_pair(channelName, newChannel));	// <-- add channel to map
		return (newChannel);
	}
	else {
		return (NULL);
	}
}

void	Server::removeChannel(const std::string & channelName) {
	if (this->_channels.find(channelName) != this->_channels.end()) {
		delete (this->_channels[channelName]);								// <-- delete Channel *
		this->_channels.erase(channelName);									// <-- remove channel from map
	}
}

//users
ClientInstance*	Server::getUserByFd(int userFd) const {
	if (this->_users.find(userFd) != this->_users.end()) {
		return (this->_users.find(userFd)->second);
	}
	else {
		return (NULL);
	}

}

ClientInstance*	Server::getUserByUserName(const std::string & userName) const {
	std::map<int, ClientInstance*>::const_iterator it = this->_users.begin();
	while (it != this->_users.end() && it->second->getUserName() != userName) {
		++it;
	}
	if (it != this->_users.end()) {
		return (it->second);
	}
	else {
		return (NULL);
	}
}

ClientInstance*	Server::getUserByNickName(const std::string & nickName) const {
	std::map<int, ClientInstance*>::const_iterator it = this->_users.begin();
	while (it != this->_users.end() && it->second->getNickName() != nickName) {
		++it;
	}
	if (it != this->_users.end()) {
		return (it->second);
	}
	else {
		return (NULL);
	}
}

ClientInstance*	Server::createUser() {
	int userFd = accept(this->_serverSocket, NULL, NULL);
	if (userFd == -1) {
		std::cerr << "Connection error" << std::endl;
		return (NULL);
	}
	std::cout << "New client connection, FD[" << userFd << "]" << std::endl;
	if (this->_users.find(userFd) == this->_users.end()) {
		ClientInstance* newClientInstance = new ClientInstance(userFd);
		this->_users.insert(std::make_pair(userFd, newClientInstance));		// add user to map

		pollfd userPollFd;
		userPollFd.fd = userFd;
		userPollFd.events = POLLIN;
		userPollFd.revents = 0;
		this->_fds.push_back(userPollFd);					// add user pollfd to vector
		
		return (newClientInstance);
	}
	else {
		return (NULL);
	}
}

void	Server::removeUser(int userFd) {
	if (this->_users.find(userFd) != this->_users.end()) {

		delete (this->_users[userFd]);								// delete User *
		this->_users.erase(userFd);									// remove user from map

		std::vector<pollfd>::iterator it = this->_fds.begin();
		while (it != this->_fds.end() && it->fd != userFd) {
			++it;
		}
		if (it != this->_fds.end()) {
			this->_fds.erase(it);									// <-- remove user pollfd from vector
		}

		std::cout << "Connection with FD[" << userFd << "] closed." << std::endl;
	}
}
