/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luigi <luigi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 14:45:20 by larmogid          #+#    #+#             */
/*   Updated: 2024/11/08 09:41:05 by luigi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "ClientInstance.hpp"

Channel::Channel(void) : _usersLimit(0) {}

Channel::Channel(const std::string& channelName) : _name(channelName), _usersLimit(0) {}

Channel::~Channel(void) {}

// getters
const std::string	Channel::getName(void) const {
	return (this->_name);
}

const std::string	Channel::getPassword(void) const {
	return (this->_pw);
}

const std::string	Channel::getTopic(void) const {
	return (this->_topic);
}

const std::string	Channel::getMode(void) const {
	return (this->_mode);
}

int	Channel::getUsersLimit(void) const {
	return (_usersLimit);
}

int	Channel::getNbrOfUsers(void) const {
	return (_users.size());
}

bool	Channel::hasFlag(char flag) const {
	return (this->_mode.find(flag) != std::string::npos);
}

const std::map <std::string, bool>&	Channel::getUsers(void) const {
	return (this->_users);
}

//settters
void	Channel::setName(const std::string& name) {
	this->_name = name;
}

void	Channel::setPassword(const std::string& password) {
	this->_pw = password;
}

void	Channel::setTopic(const std::string& topic) {
	this->_topic = topic;
}

void	Channel::setUsersLimit(int userLimit) {
	this->_usersLimit = userLimit;
}

void	Channel::addMode(const std::string& mode) {
	for (size_t i = 0; i < mode.length(); i++) {
		if (this->_mode.find(mode[i]) == std::string::npos) {
			this->_mode.push_back(mode[i]);
		}
	}
}

void	Channel::addMode(char flag) {
	if (this->_mode.find(flag) == std::string::npos) {
		this->_mode.push_back(flag);
	}
}

void	Channel::removeMode(const std::string& mode) {
	for (size_t i = 0; i < mode.length(); i++) {
		size_t pos = this->_mode.find(mode[i]);
		if (pos != std::string::npos) {
			this->_mode.erase(pos, 1);
		}
	}
}

void	Channel::removeMode(char flag) {
	size_t pos = this->_mode.find(flag);
	if (pos != std::string::npos) {
		this->_mode.erase(pos, 1);
	}
}

//users
bool	Channel::isUserOperator(const std::string& nickName) const {
	std::map<std::string, bool>::const_iterator it = _users.begin();
	while (it != this->_users.end() && it->first != nickName) {
		++it;
	}
	return (it != this->_users.end() && it->second);
}

void	Channel::addUser(const std::string& nickname) {
	if (this->_users.find(nickname) == this->_users.end()) 	{
		this->_users.insert(std::make_pair(nickname, false));
	}
}

bool	Channel::isUserIn(const std::string& nickName) const {
	return (_users.find(nickName) != this->_users.end());
}

void	Channel::updateNick(const std::string& oldNick, const std::string& newNick) {
	std::map<std::string, bool>::iterator it = this->_users.find(oldNick);
	if (it != this->_users.end()) {
		this->_users.insert(std::make_pair(newNick, it->second));
		this->_users.erase(it);
	}
}

void	Channel::upgradeUserPriviledges(const std::string& nickName) {
	std::map<std::string, bool>::iterator it = _users.begin();
	while (it != this->_users.end() && it->first != nickName) {
		++it;
	}
	if (it != _users.end()) {
		it->second = true;;
	}
}

void	Channel::downgradeUserPriviledges(const std::string& nickName) {
	std::map<std::string, bool>::iterator it = _users.begin();
	while (it != this->_users.end() && it->first != nickName) {
		++it;
	}
	if (it != _users.end()) {
		it->second = false;;
	}
}

void	Channel::removeUser(const std::string& nickName) {
	std::map<std::string, bool>::iterator it = _users.begin();
	while (it != this->_users.end() && it->first != nickName) {
		++it;
	}
	if (it != _users.end()) {
		this->_users.erase(it);
	}
}
