/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luigi <luigi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 14:47:18 by larmogid          #+#    #+#             */
/*   Updated: 2024/11/08 09:35:51 by luigi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

std::string	toString(int n) {
	std::stringstream ss;
	ss << n;
	return (ss.str());
}

std::string strTrim(const std::string& s) {
	std::string str = s;
	size_t i = 0;
	while (std::isspace(str[i])) {
		i++;
	}
	if (i == str.length()) {
		str.clear();
		return (str);
	}
	size_t j = str.length() - 1;
	while (std::isspace(str[j])) {
		j--;
	}
	str = str.substr(i, j - i + 1);
	return (str);
}
//senza delimitatore (spazi bianchi)
std::vector<std::string>	tokenizeString(const std::string& input) {
    std::vector<std::string> result;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        result.push_back(token);
    }
    return result;
}

// overloading 
std::vector<std::string>	tokenizeString(const std::string& input, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        result.push_back(strTrim(token));
    }
	return (result);
}

std::string	toUpperCase(const std::string& input) {
	std::string result = input;
	for (size_t i = 0; i < result.length(); ++i) {
		result[i] = std::toupper(result[i]);
	}
	return (result);
}

std::string	toLowerCase(const std::string& input) {
	std::string result = input;
	for (size_t i = 0; i < result.length(); ++i) {
		result[i] = std::tolower(result[i]);
	}
	return (result);
}

// analyzes input string and extracts cmds and messages; returns vector containing cmds and, if exists, message
std::vector<std::string>	parseInput(const std::string& input) {
	std::string	str = input; // Copy
	size_t colonPos = str.find(':'); // Position of the 1st character ':'
	std::string	spacedString; // text after ':'

	if (colonPos != std::string::npos) { // if ':' is found
		spacedString = strTrim(str.substr(colonPos + 1)); // removes spaces and takes text after ':' 
		str = str.substr(0, colonPos); // keeps only text before ':'
	}
	std::vector<std::string> vec = tokenizeString(str); // divides text before ':' in words
	if (!vec.empty()) {
		vec[0] = toUpperCase(vec[0]); // to upper 
	}
	if (!spacedString.empty()) {
		vec.push_back(spacedString); // adds text after ':' to vector
	}
	return (vec);
}

bool	isValidName(const std::string& name) {
	if (name.empty() || std::isspace(name[0]) || std::isspace(name[name.length() - 1])) {
		return (false);
	}
	for (size_t i = 0; i < name.length(); i++) {
		if (!std::isalnum(name[i]) && name[i] != '_' && name[i] != '-') {
			return (false);
		}
	}
	return (true);
}

bool	isProperPassword(const std::string& password) {
	if (password.empty() || std::isspace(password[0]) || std::isspace(password[password.length() - 1])) {
		return (false);
	}
	else if (password.find(',') != std::string::npos || password == ".") {
		return (false);
	}
	else {
		return (true);
	}
}

bool	isVerifiedPort(const std::string& port) {
	if (port.empty()) {
		return (false);
	}
	for (size_t i = 0; i < port.length(); ++i) {
		if (!std::isdigit(port[i])) {
			return (false);
		}
	}
	int res = std::atoi(port.c_str());
	return (res >= 0 && res < USHRT_MAX);
}
