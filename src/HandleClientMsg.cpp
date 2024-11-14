/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HandleClientMsg.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luigi <luigi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 14:46:07 by larmogid          #+#    #+#             */
/*   Updated: 2024/11/08 09:40:49 by luigi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Channel.hpp"
#include "ClientInstance.hpp"
#include "Utils.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <iostream>
#include <string>

#define MSG (MSG_DONTWAIT | MSG_NOSIGNAL)
#define SRV_NAME "ircserv"

// format --> PASS <password>
void	Server::handlePass(std::vector<std::string> argv, ClientInstance* user) {
	if (user->isVerified()) {
		; //error: user is already verified
	}
	else if (argv.size() < 2) {
		this->errorMsg(user, 464); //error: user didn't provide password
	}
	else if (argv[1] != this->_pw) {
		this->errorMsg(user, 464); //error: password doesn't match
	}
	else {
		user->verifyUser();
		//send message to client?
	}
}

// format --> USER <username> (AUTHENTICATION)
void	Server::handleUser(std::vector<std::string> argv, ClientInstance* user) {
	if (!user->isVerified()) {
		this->errorMsg(user, 464); //error: user isn'ttoLowerCase verified
	}
	else if (user->isAuthenticated()) {
		this->errorMsg(user, 462); //error: user is already authenticated
	}
	else if (argv.size() < 2) {
		this->errorMsg(user, 465); //error: user didn't provide username
	}
	else if (!user->getUserName().empty()) {
		; //error: cannot change username
	}
	else if (this->getUserByUserName(argv[1])) {
		this->errorMsg(user, 400); //error: user already exists
	}
	else if (!isValidName(argv[1])) {
		this->errorMsg(user, 468); //error: invalid username
	}
	else {
		user->setUserName(argv[1]);
		if (!user->isAuthenticated() && !user->getNickName().empty()) {
			user->authenticate();
			this->welcomeMsg(user);
		}
	}
}

// format --> NICK <nickname>
void	Server::handleNick(std::vector<std::string> argv, ClientInstance* user) {
	if (!user->isVerified()) {
		this->errorMsg(user, 464); //error: user isn't verified
	}
	else if (argv.size() < 2) {
		this->errorMsg(user, 431); //error: user didn't provide nickname
	}
	else if (!isValidName(argv[1])) {
		this->errorMsg(user, 432); //error: invalid nickname
	}
	else if (this->getUserByNickName(argv[1])) {
		this->errorMsg(user, 433); //error: user already exists
	}
	else {
		std::string rplNick = ":" + user->getNickName() + "!" + this->getName() + " " + argv[0] + " " + argv[1] + "\r\n";
		std::string oldNick = user->getNickName();
		user->setNickName(argv[1]);
		if (user->isAuthenticated()) {
			this->serverBroadcast(NULL, rplNick);
			for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
				it->second->updateNick(oldNick, argv[1]);
			}
		}
		else if (!user->getUserName().empty()) {
			user->authenticate();
			this->welcomeMsg(user);
		}
	}
}

// format --> PRIVMSG <nickname/channel> <message>
void	Server::handlePrivMsg(std::vector<std::string> argv, ClientInstance* user) {
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451);
		return; // user isn't authenticated
	}
	if (argv.size() < 2) {
		this->errorMsg(user, 461); // user didn't provide destination
		return ;
	}
	if (argv.size() < 3 || argv[2].empty()) {
		this->errorMsg(user, 413); //error: user didn't provide message
		return ;
	}
	std::string	destName = argv[1];
	std::string msgText = argv[2];
	msgText = ":" + user->getNickName() + "!" + this->getName() + " " + argv[0] + " " + destName + " :" + msgText + "\r\n";
	if (destName[0] == '#') {
		//send msg to all user of channel
		toLowerCase(destName); // case-insensitive
		destName.erase(0, 1);
		Channel * channelDest = this->getChannelByName(destName);
		if (!channelDest) {
			this->errorMsg(user, 403); //error: channel doesn't exist
		}
		else if (!channelDest->isUserIn(user->getNickName())) {
			this->errorMsg(user, 442); //error: user not in channel
		}
		else {
			this->channelBroadcast(channelDest, user, msgText);
		}
	}
	else {
		//send message to user
		ClientInstance* userDest = this->getUserByNickName(destName);
		if (!userDest) {
			this->errorMsg(user, 400); //error: user_to_text not found
		}
		else if (!userDest->isAuthenticated()) {
			; //error: user_to_text isn't authenticated
		}
		else if (userDest != user) {
			send(userDest->getFd(), msgText.c_str(), msgText.length(), MSG);
		}
	}
}

// format --> JOIN <ch1,ch2,...,chn> [<key1,key2,...,keyn>]
void	Server::handleJoin(std::vector<std::string> argv, ClientInstance* user) {
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451); //error: user isn't authenticated
		return;
	}
	if (argv.size() < 2) {
		this->errorMsg(user, 461); //error: user didn't provide channel(s)
		return;
	}
	std::vector<std::string> channelVec = tokenizeString(argv[1], ',');	// <-- split into channel vector
	std::vector<std::string> keyVec;
	if (argv.size() > 2) {
		keyVec = tokenizeString(argv[2], ','); 							// <-- split into password vector
	}
	for (size_t i = 0; i < channelVec.size(); ++i) {
		channelVec[i] = toLowerCase(channelVec[i]);
		if (channelVec[i][0] == '#') {
			channelVec[i].erase(0, 1);
		}
		Channel * channel = getChannelByName(channelVec[i]);
		if (channel) {	// <-- channel exists
			if (channel->isUserIn(user->getNickName())) {
				this->joinMsg(channel, user); //error: user is already in channel
			}
			else if (channel->hasFlag('i')) {
				this->errorMsg(user, 473); //error: channel is invite only
			}
			else if (channel->hasFlag('k') && (i >= keyVec.size() || channel->getPassword() != keyVec[i])) {
				this->errorMsg(user, 475); //error: password needed, user didn't send it or it doesn't match
			}
			else if (channel->hasFlag('l') && channel->getNbrOfUsers() >= channel->getUsersLimit()) {
				this->errorMsg(user, 471); //error: too many users
			}
			else { //user can join
				channel->addUser(user->getNickName());
				if (channel->getNbrOfUsers() == 1) {
					channel->upgradeUserPriviledges(user->getNickName());
				}
				this->joinMsg(channel, user); //send JOIN messages to users
			}
		}
		else if (isValidName(channelVec[i])) {	// <-- channel doesn't exist
			if (i < keyVec.size() && !keyVec[i].empty() && keyVec[i] != ".") {
				if (isProperPassword(keyVec[i])) {
					channel = createChannel(channelVec[i]);
					channel->setPassword(keyVec[i]);
					channel->addMode('k');
				}
				else {
					this->errorMsg(user, 400); // error: invalid password <-- won't create channel
				}
			}
			else {
				channel = createChannel(channelVec[i]);
			}
			if (channel) {
				channel->addUser(user->getNickName());
				channel->upgradeUserPriviledges(user->getNickName());
				this->joinMsg(channel, user); //send JOIN messages to users
			}
		}
		else {
			this->errorMsg(user, 400); //error: channel doesn't exist but user provided invalid name 
		}
	}
}

// format --> PART <channel> [<message>]
void	Server::handlePart(std::vector<std::string> argv, ClientInstance* user) {
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451); //error: user isn't authenticated
		return;
	}
	if (argv.size() < 2) {
		this->errorMsg(user, 461); //error: user did't provide channel
		return ;
	}
	std::string channelName = toLowerCase(argv[1]);
	if (channelName[0] == '#') {
		channelName.erase(0, 1);
	}
	Channel * channel = this->getChannelByName(channelName);
	if (!channel) {
		this->errorMsg(user, 403); //error: channel doesn't exist
	}
	else if (!channel->isUserIn(user->getNickName())) {
		this->errorMsg(user, 442); //error: user not in channel
	}
	else {
		std::string rplMsg = ":" + user->getNickName() + "!" + this->getName() + " " + argv[0] + " #" + channelName;
		if (argv.size() > 2) {
			rplMsg += " :" + argv[2] + "\r\n";
		}
		rplMsg += "\r\n";
		this->channelBroadcast(channel, NULL, rplMsg);
		channel->removeUser(user->getNickName());
	}
}

// QUIT [<message>]
void	Server::handleQuit(std::vector<std::string> argv, ClientInstance* user) {
	if (user->isAuthenticated()) {
		std::string rplMsg = ":" + user->getNickName() + "!" + this->getName() + " " + argv[0];
		if (argv.size() > 1) {
			rplMsg += " :" + argv[1];
		}
		else {
			rplMsg += " :";
		}
		rplMsg += "\r\n";
		this->serverBroadcast(NULL, rplMsg);
	}
	this->removeUser(user->getFd());
}

void	Server::handlePing(std::vector<std::string> argv, ClientInstance* user) {
	std::string rplMsg = "PONG";
	if (argv.size() > 1) {
		rplMsg += " " + argv[1];
	}
	rplMsg += "\r\n";
	send(user->getFd(), rplMsg.c_str(), rplMsg.length(), MSG);
}
