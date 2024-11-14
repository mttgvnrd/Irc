/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luigi <luigi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 14:47:01 by larmogid          #+#    #+#             */
/*   Updated: 2024/11/08 09:38:35 by luigi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP 
#define SERVER_HPP 

#include <map>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include "HandleErrors.hpp"

class Channel;

class ClientInstance;

class Server
{
	private:
		Server(void);

		bool							_isRunning;
		char							_hostname[256];
		char *							_ip;

		int								_serverSocket;
		u_int16_t						_port; // serve a htons();
		sockaddr_in						_serverAddr;

		const std::string				_serverName;
		std::string						_pw;

		std::map<std::string, Channel*>	_channels;
		std::map<int, ClientInstance*>	_users;
		std::vector<pollfd>				_fds;

		//communication
		void	channelBroadcast(Channel* channel, ClientInstance* user, const std::string& msg) const;
		void	serverBroadcast	(ClientInstance* user, const std::string& msg) const;
		void	joinMsg(Channel* channel, ClientInstance* user);
		void	welcomeMsg(ClientInstance* user);
		void	errorMsg(ClientInstance* user, int code);
		void	handleCommand(std::vector<std::string> argv, ClientInstance* user);
		int		handleMessage(int userFd);

		// Handle Client Messages:
		void	handleInvite(std::vector<std::string> argv, ClientInstance* user);
		void	handleJoin(std::vector<std::string> argv, ClientInstance* user);
		void	handleKick(std::vector<std::string> argv, ClientInstance* user);
		void	handleMode(std::vector<std::string> argv, ClientInstance* user);
		void	handleNick(std::vector<std::string> argv, ClientInstance* user);
		void	handlePart(std::vector<std::string> argv, ClientInstance* user);
		void	handlePass(std::vector<std::string> argv, ClientInstance* user);
		void	handlePing(std::vector<std::string> argv, ClientInstance* user);
		void	handlePrivMsg(std::vector<std::string> argv, ClientInstance* user);
		void	handleQuit(std::vector<std::string> argv, ClientInstance* user);
		void	handleTopic(std::vector<std::string> argv, ClientInstance* user);
		void	handleUser(std::vector<std::string> argv, ClientInstance* user);

	public:
		Server(int port, const std::string& password);					// C'tor where port is given as an int
		Server(const std::string & port, const std::string& password); // C'tor where port is given as a literal int
		~Server(void);

		void	init();
		void	run ();
		void	stop();
        void	initializeErrorMessages();

		// getters
		bool				isRunning(void) const;

		int					getPort(void) const;
		int					getNbrOfUsers(void) const;
		int					nChannels(void) const;

		const std::string&	getName(void) const;
		const std::string&	getPassword(void) const;

		// setters
		void				setPassword	(const std::string& password);

		// channels
		Channel*			getChannelByName(const std::string& channelName) const;
		Channel*			createChannel(const std::string& channelName);
		void				removeChannel(const std::string& channelName);

		// users
		ClientInstance*		getUserByFd(int userFd) const;
		ClientInstance*		getUserByUserName(const std::string& userName) const;
		ClientInstance*		getUserByNickName(const std::string& nickName) const;
		ClientInstance*		createUser(void);
		void				removeUser(int userFd);
};

#endif /* SERVER_HPP */
