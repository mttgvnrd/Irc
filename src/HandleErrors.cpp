/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HandleErrors.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmogid <larmogid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 14:46:21 by larmogid          #+#    #+#             */
/*   Updated: 2024/11/07 15:14:23 by larmogid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HandleErrors.hpp"
#include "Server.hpp"

// Definizione della mappa per i messaggi di errore
std::map<int, std::string> _errorMsg;

// Funzione per inizializzare i messaggi di errore
void Server::initializeErrorMessages() {
    _errorMsg[ERR_INVALID_INPUT] = "Invalid input: please check your command syntax.";
    _errorMsg[ERR_NICK_NOT_FOUND] = "No such nickname found: verify the nickname and try again.";
	_errorMsg[ERR_SERVER_NOT_FOUND] = "Server not found: the specified server does not exist.";
	_errorMsg[ERR_CHANNEL_NOT_FOUND] = "Channel not found: please check the channel name.";
	_errorMsg[ERR_CANNOT_SEND] = "Cannot send message to this channel.";
	_errorMsg[ERR_TOO_MANY_CHANNELS] = "You have joined too many channels.";
	_errorMsg[ERR_CHANNEL_LIMIT_REACHED] = "Channel limit reached: you have joined the maximum number of channels.";
	_errorMsg[ERR_NICKNAME_NOT_FOUND] = "Nickname not found: the specified user was not found.";
	_errorMsg[ERR_EMPTY_MESSAGE] = "Message content is empty: please provide text to send.";
	_errorMsg[ERR_UNKNOWN_COMMAND] = "Unknown command: please check the command and try again.";
	_errorMsg[ERR_NO_NICKNAME_GIVEN] = "Nickname not provided: please specify a nickname.";
	_errorMsg[ERR_NICKNAME_IN_USE] = "User already exists: please select a different one.";
	_errorMsg[ERR_USER_NOT_ON_CHANNEL] = "User not on specified channel.";
	_errorMsg[ERR_NOT_MEMBER_OF_CHANNEL] = "You are not a member of this channel.";
	_errorMsg[ERR_USER_ALREADY_MEMBER] = "User is already a member of this channel.";
	_errorMsg[ERR_USER_NOT_LOGGED_IN] = "User is not logged in.";
	_errorMsg[ERR_REGISTRATION_REQUIRED] = "Registration required: please register to access this feature.";
	_errorMsg[ERR_INSUFFICIENT_PARAMS] = "Insufficient parameters provided for this command.";
	_errorMsg[ERR_INVALID_USER_LIMITS]= "User didn't send channel or flags";
	_errorMsg[ERR_REREGISTRATION_DENIED] = "Reregistration denied: you may not re-register.";
	_errorMsg[ERR_PASSWDMISMATCH] = "Authentication failed: incorrect or missing password.";
	_errorMsg[ERR_CHANNEL_KEY_SET] = "Channel key already set: please check the existing key.";
	_errorMsg[ERR_INVALID_USERNAME_FORMAT] = "Invalid username format.";
	_errorMsg[ERR_CHANNEL_FULL] = "Cannot join or invite to channel: channel is full (+l mode).";
	_errorMsg[ERR_INVALID_MODE] = "Invalid mode: unknown mode character.";
	_errorMsg[ERR_INVITE_ONLY] = "Cannot join: invite-only channel (+i mode).";
	_errorMsg[ERR_PASSWORD_PROTECTED] = "Cannot join: this channel is password-protected (+k mode).";
	_errorMsg[ERR_PERMISSION_DENIED] = "Permission denied: you are not an IRC operator.";
	_errorMsg[ERR_NOT_CHANNEL_OPERATOR] = "Permission denied: you are not a channel operator.";
	_errorMsg[ERR_CHANNEL_OPERATOR_ONLY] = "Permission denied: only the original channel operator can perform this action.";
	_errorMsg[ERR_NO_ACCESS_LINES] = "No operator access lines for your host.";
	_errorMsg[ERR_UNKNOWN_MODE_FLAG] = "Unknown MODE flag specified.";
	_errorMsg[ERR_CANNOT_CHANGE_MODE] = "Cannot change mode for another user.";
}

// Funzione per recuperare i messaggi di errore
std::string getErrorMessage(int errorCode) {
    // Specifica esplicitamente il tipo di iterator
    std::map<int, std::string>::iterator it = _errorMsg.find(errorCode);
    if (it != _errorMsg.end()) {
        return it->second;
    }
    return "Unable to retrieve error details."; // Messaggio di default
}
