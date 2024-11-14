/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientInstance.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmogid <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 14:45:57 by larmogid          #+#    #+#             */
/*   Updated: 2024/11/06 14:45:59 by larmogid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_INSTANCE_HPP
#define CLIENT_INSTANCE_HPP

#include <iostream>
#include <map>
#include <poll.h>

class Channel;

class ClientInstance
{
	private:
		ClientInstance(void);

		bool         _isAuthenticated;
		bool         _isVerified;
		int          _fd;

		std::string  _userName;
		std::string  _nickName;
		std::string  _message;

	public:
		ClientInstance(int fd);
		~ClientInstance(void);

		//getters
		bool				isAuthenticated(void) const;
		bool				isVerified(void) const;
		int					getFd(void) const;
		
		const std::string&	getUserName(void) const;
		const std::string&	getNickName(void) const;
		const std::string&	getMessage(void) const;

		//setters
		void				authenticate(void);
		void				verifyUser(void);
		void				setUserName(const std::string & userName);
		void				setNickName(const std::string & nickName);
		void				setMessage(const std::string & message);

};

#endif /* CLIENT_INSTANCE_HPP */
