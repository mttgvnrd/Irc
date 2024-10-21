/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmogid <larmogid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 12:14:52 by mgiovana          #+#    #+#             */
/*   Updated: 2024/10/16 15:54:04 by larmogid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <limits.h>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>
#include <fcntl.h>
#include <cstdio>
#include <algorithm>
#include <map>
#include "Client.hpp"
#include "Channel.hpp"

class Server {
public:
    Server(int port, const std::string& password);
    ~Server();

    void start();
    void stop();
    void handleClientMessage(int client_fd);
    void handleJoinCommand(Client* client, const std::string& channelName);
    void handlePrivMsgCommand(Client* client, const std::string& target, const std::string& message);
    void handleQuitCommand(Client* client);

private:
    int _server_fd;
    int _port;
    std::string _password;
    struct sockaddr_in _address;
    
    std::vector<pollfd> _poll_fds;   // Lista dei file descriptor per il polling
    std::vector<int> _clients;       // Lista dei client connessi
    std::map<std::string, Channel*> _channels;  // Mappa per gestire i canali
    std::map<std::string, Client*> _nickname_map; // Per gestire i nickname
    std::map<int, Client*> _clients_map;   // Associa file descriptor a Client

    void handleCommand(const std::string& command, Client* client);
    void sendError(Client* client, const std::string& message);
    bool isNicknameInUse(const std::string &nickname); 
    void handle_part_command(int client_fd, const std::string& channel_name);
    void handle_privmsg_command(int client_fd, const std::string& target, const std::string& message);

    void createSocket();
    void bindSocket();
    void listenConnections();
    void acceptNewClient();
    //void handleClientMessage(int client_fd);
    void pollConnections();

    
};

#endif
