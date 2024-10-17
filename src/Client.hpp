/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgiovana <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 12:15:17 by mgiovana          #+#    #+#             */
/*   Updated: 2024/09/30 12:16:30 by mgiovana         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
public:
    Client(int fd);
    ~Client();

    int getFd() const;
    std::string getNickname() const;
    std::string getUsername() const;
    bool isAuthenticated() const;
    bool isVerified() const;
    bool isWelcomeMessageSent() const;
    void setWelcomeMessageSent(bool value);

    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username);
    void authenticate();
    void verify();

private:
    int _fd;
    std::string _nickname;
    std::string _username;
    bool _authenticated;
    bool _verified;
    bool _welcomeMessageSent;
    
};

#endif
