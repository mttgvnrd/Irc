/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HandleErrors.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmogid <larmogid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 14:46:26 by larmogid          #+#    #+#             */
/*   Updated: 2024/11/07 15:14:16 by larmogid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <map>
#include <string>

// Definizione dei codici di errore
#define ERR_INVALID_INPUT            400
#define ERR_NICK_NOT_FOUND           401
#define ERR_SERVER_NOT_FOUND         402
#define ERR_CHANNEL_NOT_FOUND        403
#define ERR_CANNOT_SEND              404
#define ERR_TOO_MANY_CHANNELS        405
#define ERR_CHANNEL_LIMIT_REACHED    406
#define ERR_NICKNAME_NOT_FOUND       407
#define ERR_EMPTY_MESSAGE            413
#define ERR_UNKNOWN_COMMAND          421 
#define ERR_NO_NICKNAME_GIVEN        431
#define ERR_NICKNAME_IN_USE          433
#define ERR_USER_NOT_ON_CHANNEL      441
#define ERR_NOT_MEMBER_OF_CHANNEL    442
#define ERR_USER_ALREADY_MEMBER      443
#define ERR_USER_NOT_LOGGED_IN       444
#define ERR_REGISTRATION_REQUIRED    451
#define ERR_INSUFFICIENT_PARAMS      461 //
#define ERR_REREGISTRATION_DENIED    462
#define ERR_INVALID_USER_LIMITS      463
#define ERR_PASSWDMISMATCH           464
#define ERR_CHANNEL_KEY_SET          467
#define ERR_INVALID_USERNAME_FORMAT  468
#define ERR_CHANNEL_FULL             471
#define ERR_INVALID_MODE             472
#define ERR_INVITE_ONLY              473
#define ERR_PASSWORD_PROTECTED       475
#define ERR_PERMISSION_DENIED        481
#define ERR_NOT_CHANNEL_OPERATOR     483
#define ERR_CHANNEL_OPERATOR_ONLY    485
#define ERR_NO_ACCESS_LINES          491
#define ERR_UNKNOWN_MODE_FLAG        501
#define ERR_CANNOT_CHANGE_MODE       502

std::string getErrorMessage(int errorCode);

#endif /* ERRORS_HPP */
