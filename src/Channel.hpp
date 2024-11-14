/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luigi <luigi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 14:45:26 by larmogid          #+#    #+#             */
/*   Updated: 2024/11/08 09:40:49 by luigi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <map>

class ClientInstance;

class Channel
{
	private:
		/*  Gli oggetti std::string (di dimensione variabile) contengono un puntatore interno, 
		una dimensione (concreta) e una capacità (quanti caratteri la stringa può contenere 
		prima di dover riallocare memoria). */
		std::map<std::string, bool>		_users;

		std::string		_name;
		std::string		_pw;
		std::string		_topic;
		std::string		_mode;

		int	_usersLimit;

		Channel(void);

	public:
		Channel(const std::string& channelName);
		~Channel(void);
		
		const std::map <std::string, bool>&	getUsers(void) const;
		int					getUsersLimit(void) const;
		const std::string	getName(void) const;
		const std::string	getPassword(void) const;
		const std::string	getMode(void) const;
		const std::string	getTopic(void) const;


		int		getNbrOfUsers(void) const;
		bool	hasFlag(char flag) const;

	//set
		void	setName(const std::string& name);
		void	setPassword(const std::string& password);
		void	setTopic(const std::string& topic);
		void	setUsersLimit(int);
		void	removeMode(const std::string& mode);
		void	removeMode(char flag);
		void	addMode(const std::string& mode);
		void	addMode(char flag);

	//user
		bool	isUserOperator(const std::string& nickName) const;
		bool	isUserIn(const std::string& nickName) const;
		
		void	upgradeUserPriviledges(const std::string& nickName);
		void	downgradeUserPriviledges(const std::string& nickName);
		void	addUser(const std::string& nickname);
		void	removeUser(const std::string& nickName);
		void	updateNick(const std::string& oldNick, const std::string& newNick);
};

#endif 
