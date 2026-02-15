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
#include <set>
#include <map>
#include <sys/epoll.h>
#include <sys/wait.h>

#include "../includes/Parser.hpp"
#include "../includes/ConfigError.hpp"
#include "../includes/VirtualServer.hpp"
#include "../includes/socket.hpp"
#include "../includes/Connection.hpp"

#define MAX_EVENTS 128

class Server {
private:
public:
	Server(t_server &server_config);
	~Server();

	void	init();
	void	run_server();
	void 	clean_connection(int epollfd, int socketfd);
	void	close_inactive_connections(int epollfd);

	// Handlers
	void	read_handler(int epollfd, int socketfd);
	void	send_handler(int epollfd,  int socketfd);
	void	cgi_read_handler(int epollfd,  int cgifd);

	static std::vector<std::string> directives;
	static std::vector<std::string> context;
	
	// Virtual Servers
	std::vector<VirtualServer> virtual_servers;

	// Listening sockets
	std::map<int, const VirtualServer*> listening_sockfds;
	std::map<int, Connection>	active_connections; // cgi_fd -> socket_fd
	std::map<int, int>	cgi_output_map; // cgi_fd -> socket_fd

	// Server Configs
	std::string root;
	std::map<int, std::string> error_page;
	size_t client_max_body_size;
	std::vector<std::string> index;
	bool autoindex;
};

std::ostream& operator<<(std::ostream& os, const Server& s);

#endif
