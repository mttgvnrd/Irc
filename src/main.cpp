/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmogid <larmogid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 12:14:58 by mgiovana          #+#    #+#             */
/*   Updated: 2024/10/16 11:49:24 by larmogid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Server.hpp"
#include "Client.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Add: " << argv[0] << " <Port> <Password>" << std::endl;
        return 1;
    }

    try {
    int port = atoi(argv[1]);
    std::string password = argv[2];
    Server server(port, password);
    server.start();
    }
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

    return 0;
}
