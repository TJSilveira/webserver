/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 17:29:14 by tsilveir          #+#    #+#             */
/*   Updated: 2026/01/27 22:35:15 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Lexer.hpp"
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

std::string config_file_to_string(std::ifstream &conf_file)
{
	std::string	line;
	std::string	res;

	while (std::getline(conf_file, line))
	{
		res.append(line);
		res.append("\n");
	}
	return (res);
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
	conf_str = config_file_to_string(conf_file);
	conf_file.close();

	std::cout <<conf_str << "\n";
	Lexer lexer(conf_str);
	std::cout << lexer;
	
}
