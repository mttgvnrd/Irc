/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgiovana <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 12:15:12 by mgiovana          #+#    #+#             */
/*   Updated: 2024/09/30 12:16:21 by mgiovana         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _authenticated(false) {}

Client::~Client() {}

int Client::getFd() const {
    return _fd;
}

std::string Client::getNickname() const {
    return _nickname;
}

std::string Client::getUsername() const {
    return _username;
}

bool Client::isAuthenticated() const {
    return _authenticated;
}

void Client::setNickname(const std::string& nickname) {
    _nickname = nickname;
}

void Client::setUsername(const std::string& username) {
    _username = username;
}

void Client::authenticate() {
    if (!_nickname.empty() && !_username.empty()) {
        _authenticated = true;
    }
}
