/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larmogid <larmogid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 14:46:44 by larmogid          #+#    #+#             */
/*   Updated: 2024/11/06 14:48:13 by larmogid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>

Server * globalServerPtr = NULL;

static void	sigHandler(int signal, siginfo_t *info, void *ucontext) {
	if (info->si_pid == 0) {					// <-- sender is server
		if (signal == SIGINT) {					// <-- signal is SIGINT (CTRL + C)
			if (globalServerPtr != NULL) {		// <-- server has been set
				globalServerPtr->stop();		// <-- interrupt server loop
			}
		}
	}
	(void)ucontext;
}

int main(int ac, char **av)
{
	if (ac != 3) {
		std::cerr << "Error: Invalid number of arguments. You provided " << ac - 1 << ", but expected 2." << std::endl;
		std::cout << "To start the server, run: ./ircserv <port> <password>" << std::endl;
		return (1);
	}

	/* --- Signal Handler for SIGINT: handles Ctrl+C signal -----
	------ to ensure proper cleanup before exiting the server. */
	struct sigaction	sa;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGINT);
	sa.sa_flags = SA_SIGINFO | SA_NODEFER;
	sa.sa_sigaction = sigHandler;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
    std::cerr << "Error setting up signal handler." << std::endl;
		return (1);
	}
	
	/* Server */
 	try {
		Server server(av[1], av[2]);
		globalServerPtr = &server;

		// Inizializza i messaggi di errore
        server.initializeErrorMessages();
	
		server.init();
		server.run(); // starts the main event loop to handle incoming client connections and messages.
	}
	catch (std::exception& e) {
		std::cerr << "Standard Exception: " << e.what() << std::endl;
	}
	catch (const char* str) {
		std::cerr << "Custom Error: " << str << std::endl;
	}
	return (0);
}
