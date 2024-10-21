/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgiovana <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/11 14:20:26 by mgiovana          #+#    #+#             */
/*   Updated: 2024/10/11 14:20:28 by mgiovana         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "Client.hpp"
#include <algorithm>

// Costruttore del canale
Channel::Channel(const std::string& name, Client* creator) 
    : _name(name), _userLimit(0), _inviteOnly(false) {
    _members.push_back(creator);
    _operators[creator] = true;  // Il creatore è l'operatore iniziale
}

// Aggiunge un membro al canale
bool Channel::addMember(Client* client) {
    if (_userLimit > 0 && _members.size() >= _userLimit) {
        return false;  // Il canale ha raggiunto il limite di utenti
    }
    _members.push_back(client);
    return true;
}

// Verifica se un client è membro del canale
bool Channel::isMember(Client* client) const {
    return std::find(_members.begin(), _members.end(), client) != _members.end();
}

// Rimuove un membro dal canale
void Channel::removeMember(Client* client) {
    _members.erase(std::remove(_members.begin(), _members.end(), client), _members.end());
    _operators.erase(client);  // Rimuovi anche l'utente dagli operatori, se presente
}

// Invia un messaggio a tutti i membri del canale
void Channel::broadcastMessage(const std::string& message, Client* sender) {
    for (Client* member : _members) {
        if (member != sender) {
            send(member->getFd(), message.c_str(), message.size(), 0);
        }
    }
}

    // Funzione che controlla se un client è operatore
    bool Channel::isOperator(Client* client) const {
    auto it = _operators.find(client);  // Trova il client nella mappa
    return it != _operators.end() && it->second;  // Restituisce true se è operatore
}

void Channel::setTopic(const std::string& topic, Client* client) {
    if (isOperator(client)) {
        _topic = topic;
        std::cout << "Il client " << client->getNickname() << " ha cambiato il topic del canale in: " << topic << std::endl;
    } else {
        std::cerr << "Errore: Il client " << client->getNickname() << " non ha i permessi per cambiare il topic." << std::endl;
    }
}

// Comando KICK
void Channel::kick(Client* client, Client* target) {
    if (isOperator(client)) {
        removeMember(target);
        std::string kickMsg = ":" + client->getNickname() + " KICK " + _name + " " + target->getNickname() + "\n";
        broadcastMessage(kickMsg, client);
        std::cout << "Client " << target->getNickname() << " kicked from channel " << _name << std::endl;
    } else {
        std::cerr << "Errore: " << client->getNickname() << " non ha i permessi per eseguire il comando KICK." << std::endl;
    }
}

// Comando INVITE
void Channel::invite(Client* client, Client* target) {
    if (isOperator(client)) {
        std::string inviteMsg = ":" + client->getNickname() + " INVITE " + target->getNickname() + " " + _name + "\n";
        send(target->getFd(), inviteMsg.c_str(), inviteMsg.size(), 0);
        std::cout << "Client " << target->getNickname() << " è stato invitato al canale " << _name << std::endl;
    } else {
        std::cerr << "Errore: " << client->getNickname() << " non ha i permessi per eseguire il comando INVITE." << std::endl;
    }
}

// Comando MODE
void Channel::changeMode(Client* client, char mode, bool enable) {
    if (isOperator(client)) {
        if (mode == 'i') {
            _inviteOnly = enable;
            std::cout << "La modalità invite-only del canale " << _name << " è stata " << (enable ? "abilitata" : "disabilitata") << std::endl;
        } else if (mode == 't') {
            _topicRestriction = enable;
            std::cout << "La restrizione del topic del canale " << _name << " è stata " << (enable ? "abilitata" : "disabilitata") << std::endl;
        }
    } else {
        std::cerr << "Errore: " << client->getNickname() << " non ha i permessi per eseguire il comando MODE." << std::endl;
    }
}

