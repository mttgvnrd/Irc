/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientInstance.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmogid <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 14:45:50 by larmogid          #+#    #+#             */
/*   Updated: 2024/11/06 14:45:54 by larmogid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientInstance.hpp"
#include "Channel.hpp"
#include <unistd.h>

ClientInstance::ClientInstance(void) : _isAuthenticated(false), _isVerified(false) {}

ClientInstance::~ClientInstance() {
	close(this->getFd());
}

ClientInstance::ClientInstance(int fd) : _isAuthenticated(false), _isVerified(false) {
	this->_fd = fd;
}

//getters
bool	ClientInstance::isAuthenticated(void) const {
	return (this->_isAuthenticated);
}

bool	ClientInstance::isVerified(void) const {
	return (this->_isVerified);
}

int	ClientInstance::getFd(void) const {
	return (this->_fd);
}


const std::string&	ClientInstance::getUserName(void) const {
	return (this->_userName);
}

const std::string&	ClientInstance::getNickName(void) const {
	return (this->_nickName);
}

const std::string&	ClientInstance::getMessage(void) const {
	return (this->_message);
}

//setters
void	ClientInstance::authenticate() {
	this->_isAuthenticated = true;
}

void	ClientInstance::verifyUser() {
	this->_isVerified = true;
}

void	ClientInstance::setUserName(const std::string & userName) {
	this->_userName = userName;
}

void	ClientInstance::setNickName(const std::string & nickName) {
	this->_nickName = nickName;
}

void	ClientInstance::setMessage(const std::string & message) {
	this->_message = message;
}
