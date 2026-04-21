/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amoiseik <amoiseik@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 22:41:05 by tiago             #+#    #+#             */
/*   Updated: 2026/03/03 18:36:03 by amoiseik         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP
# include "../includes/ConfigError.hpp"
# include "../includes/Connection.hpp"
# include "../includes/Parser.hpp"
# include "../includes/VirtualServer.hpp"
# include <cstddef>
# include <cstdlib>
# include <iostream>
# include <map>
# include <set>
# include <sys/epoll.h>
# include <sys/wait.h>
# include <vector>

# define MAX_EVENTS 8192
# define MAX_CONNECTIONS 1024
# define TIMEOUT_SECONDS 600
# define MAX_CGI_RUNTIME 10

class Server
{
private:
public:
	Server(t_server &server_config);
	~Server();

	void init();
	void run_server();
	void clean_connection(int epollfd, int socketfd);
	void clean_all_connections(int epollfd);
	void close_inactive_connections(int epollfd);
	void clean_cgi_fd(int epollfd, int cgifd);
	bool accept_connections(int epollfd, int sockfd);
	void check_cgi_timeouts(int epollfd);

	// Handlers
	void read_handler(int epollfd, int socketfd);
	void send_handler(int epollfd, int socketfd);
	void cgi_read_handler(int epollfd, int cgifd);

	void print_req_resp(const Connection &curr_connection);

	static std::vector<std::string> directives;
	static std::vector<std::string> context;

	// Virtual Servers
	std::vector<VirtualServer> virtual_servers;
	const VirtualServer* find_virtual_server(int port, const std::string& host_header);


	// Listening sockets
	std::map<int, const VirtualServer *> listening_sockfds;
	std::map<int, Connection> active_connections; // socket_fd -> Connection
	std::map<int, int> cgi_output_map;            // cgi_fd -> socket_fd

	// Server Configs
	std::string root;
	std::map<int, std::string> error_page;
	size_t client_max_body_size;
	std::vector<std::string> index;
	bool autoindex;
};

std::ostream &operator<<(std::ostream &os, const Server &s);

#endif
