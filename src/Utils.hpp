/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luigi <luigi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 14:47:21 by larmogid          #+#    #+#             */
/*   Updated: 2024/11/08 09:35:51 by luigi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <climits>

bool						isValidName(const std::string& name);
bool						isProperPassword(const std::string& password);
bool						isVerifiedPort(const std::string& port);

std::string					toLowerCase(const std::string&);
std::string					toUpperCase(const std::string&);

std::string					toString(int);
std::string					strTrim(const std::string&);
std::vector<std::string>    tokenizeString(const std::string&);
std::vector<std::string>	tokenizeString(const std::string&, char); //OVERLOADING!!!
std::vector<std::string>	parseInput(const std::string&);

#endif
