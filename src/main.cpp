/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 17:29:14 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/03 19:22:16 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Lexer.hpp"
#include "../includes/Parser.hpp"
#include <iostream>

void open_and_validate_file(char *filename, std::ifstream	&conf_file)
{
	conf_file.open(filename, std::ios::in | std::ios::binary);
	if (conf_file.fail())
	{
		std::cerr << "Error: could not open file." << std::endl;
		exit(EXIT_FAILURE);
	}
}

std::string file_to_string(std::ifstream &file)
{
	return (std::string(std::istreambuf_iterator<char>(file),
						std::istreambuf_iterator<char>()));
}

int main(int argc, char* argv[])
{
	std::ifstream	conf_file;
	std::string		conf_str;

	// input validation
	if (argc != 2) {
		std::cerr << "Error: must pass as argument one configuration file\n";
		return 0;
	}

	open_and_validate_file(argv[1], conf_file);
	conf_str = file_to_string(conf_file);
	conf_file.close();

	std::cout << "========== [Lexer] ==========\n\n";

	std::cout <<conf_str << "\n";
	Lexer lexer(conf_str);
	std::cout << lexer;
	
	std::cout << "========== [Parser] ==========\n\n";
	Parser parser(lexer);

	std::cout << parser;

	Server main_server(parser.server);

	std::cout << main_server;
}
