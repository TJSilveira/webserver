/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 22:41:05 by tiago             #+#    #+#             */
/*   Updated: 2026/01/27 12:02:10 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include <map>

#include "../includes/Parser.hpp"
#include "../includes/ConfigError.hpp"
#include "../includes/VirtualServer.hpp"

class Server {
private:
public:
	Server(t_server &server_config);
	~Server();

	static std::vector<std::string> directives;
	static std::vector<std::string> context;

	std::vector<VirtualServer> virtual_servers;
	std::string root;
	std::map<int, std::string> error_page;
	size_t client_max_body_size;
	std::vector<std::string> index;
	bool autoindex;
};

std::ostream& operator<<(std::ostream& os, const Server& s);

#endif
