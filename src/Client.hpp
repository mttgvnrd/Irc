/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmogid <larmogid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 12:15:17 by mgiovana          #+#    #+#             */
/*   Updated: 2024/10/17 17:43:21 by larmogid         ###   ########.fr       */
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
    void setVerified(bool verified);
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
