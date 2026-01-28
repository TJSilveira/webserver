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
#include <unordered_map>

#include "../includes/Parser.hpp"

class Server {
private:
public:
	Server(Parser &parser);
	~Server();

	static std::vector<std::string> directives;
	static std::vector<std::string> context;

	std::string root;
	std::vector<std::string> index;
	std::unordered_map<int, std::string> error_page;
	size_t client_max_body_size;
	bool autoindex;
};

#endif
