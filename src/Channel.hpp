/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgiovana <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/11 14:20:29 by mgiovana          #+#    #+#             */
/*   Updated: 2024/10/11 14:20:31 by mgiovana         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <string>
#include <vector>
#include <map>
#include <sys/socket.h>
#include "Client.hpp"
#include <iostream>

class Channel {
private:
    std::string _name;
    std::string _topic;
    std::vector<Client*> _members;
    std::map<Client*, bool> _operators;
    int _userLimit;
    bool _inviteOnly;
    bool _topicRestriction;  // modalità t

public:
    Channel(const std::string& name, Client* creator);

    const std::string& getName() const;
    
    // Aggiunge un membro al canale
    bool addMember(Client* client);

    // Rimuove un membro dal canale
    void removeMember(Client* client);

    // Verifica se un utente è operatore
    bool isOperator(Client* client) const;

    // Imposta il topic del canale
    void setTopic(const std::string& topic, Client* client);

    // Ottieni il topic
    const std::string& getTopic() const { return _topic; };

    // Invia un messaggio a tutti i membri
    void broadcastMessage(const std::string& message, Client* sender);

    // Funzioni per gestire comandi specifici agli operatori
    void kick(Client* client, Client* target);
    void invite(Client* client, Client* target);
    void changeMode(Client* client, char mode, bool enable);

    // Verifica se un canale è a invito
    bool isInviteOnly() const;

    bool isMember(Client* client) const;

    // Altre funzioni...
};

