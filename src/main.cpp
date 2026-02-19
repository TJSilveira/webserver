/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 17:29:14 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/19 17:44:03 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Lexer.hpp"
#include "../includes/Parser.hpp"
#include "../includes/Server.hpp"
#include "../includes/globals.hpp"
#include "../includes/utils.hpp"
#include <iostream>

// For socket
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

volatile sig_atomic_t	g_stop = 0;

void	sig_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM)
		g_stop = 1;
	std::cout << "\033[32m\nGracefully closing webserv. "
					"Thank you very much, come again!\033[0m"
				<< std::endl;
}

int	main(int argc, char *argv[])
{
	std::ifstream conf_file;
	std::string conf_str;
	// input validation
	if (argc != 2)
	{
		std::cerr << "Error: must pass as argument one configuration file\n";
		return (0);
	}
	open_and_validate_file(argv[1], conf_file);
	conf_str = file_to_string(conf_file);
	conf_file.close();
	Lexer lexer(conf_str);
	Parser parser(lexer);
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	Server main_server(parser.server);
	std::cout << main_server;
	main_server.init();
	main_server.run_server();
	return (0);
}
