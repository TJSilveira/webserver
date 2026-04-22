/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amoiseik <amoiseik@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:23:47 by tsilveir          #+#    #+#             */
/*   Updated: 2026/04/22 13:50:48 by amoiseik         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/globals.hpp"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <vector>

std::string directives_array[] = {"listen",
									"server_name",
									"return",
									"upload_store",
									"alias",
									"cgi_script_root",
									"cgi_path",
									"cgi_ext",
									"allow_methods",
									"root",
									"error_page",
									"client_max_body_size",
									"index",
									"autoindex"};

std::vector<std::string> Server::directives(directives_array,
											directives_array +
												sizeof(directives_array) /
													sizeof(std::string));

std::string context_array[] = {"http", "server", "location"};

std::vector<std::string> Server::context(
	context_array, context_array + sizeof(context_array) / sizeof(std::string));

// Constructors
Server::Server(t_server &server_config)
	: root("/var/www/html"), client_max_body_size(1000000), autoindex(false)
{
	t_directive	curr_directive;
	int			last_index;
	int			error_num;
	bool		autoindex_status;

	for (size_t i = 0; i < server_config.dir.size(); i++)
	{
		curr_directive = server_config.dir.at(i);
		if (curr_directive.name == "root")
		{
			this->root = curr_directive.args.at(0);
		}
		else if (curr_directive.name == "error_page")
		{
			last_index = curr_directive.args.size() - 1;
			std::string error_page_path = curr_directive.args.at(last_index);
			for (size_t j = 0; j < curr_directive.args.size() - 1; j++)
			{
				error_num = atoi(curr_directive.args.at(j).c_str());
				this->error_page[error_num] = curr_directive.args.at(last_index);
			}
		}
		else if (curr_directive.name == "client_max_body_size")
		{
			std::string arg = curr_directive.args.at(0);
			long long size = extract_size_to_bytes(arg);
			if (size < 0)
				throw ConfigError("client_max_body_size has invalid format", arg);
			this->client_max_body_size = static_cast<size_t>(size);
		}
		else if (curr_directive.name == "index")
		{
			this->index = curr_directive.args;
		}
		else if (curr_directive.name == "autoindex")
		{
			if (curr_directive.args.size() == 1
				&& curr_directive.args.at(0) == "on")
				autoindex_status = true;
			else if (curr_directive.args.size() == 1 &&
						curr_directive.args.at(0) == "off")
				autoindex_status = false;
			else
			{
				throw ConfigError("autoindex needs to be either 'on' or 'off'",
									curr_directive.args.at(0));
			}
			this->autoindex = autoindex_status;
		}
		else
		{
			throw ConfigError("directive not allowed in a server block",
								curr_directive.name);
		}
	}
	for (size_t i = 0; i < server_config.vir_servers.size(); i++)
	{
		this->virtual_servers.push_back(
			VirtualServer(server_config.vir_servers.at(i), *this));
	}
}

// Destructor
Server::~Server(void)
{
}

void Server::init()
{
	int	port;
	int	fd;

	std::set<int> bound_ports;
	for (size_t i = 0; i < virtual_servers.size(); i++)
	{
		port = virtual_servers.at(i).listen;
		if (bound_ports.find(port) == bound_ports.end())
		{
			fd = create_listening_socket(port, "0.0.0.0");
			listening_sockfds.insert(std::make_pair(fd,	&virtual_servers.at(i)));
			bound_ports.insert(port);
			logger(INFO, "Successfully bounded port '" + ft_int_to_string(port) + "' with fd '" + ft_int_to_string(fd) + "'", std::cout);
		}
		else
		{
			logger(INFO, "Port " + ft_int_to_string(port) + " already bound, adding virtual server name", std::cout);
		}
	}
	logger(INFO, "Server waiting for connections...", std::cout);
}

void Server::run_server()
{
	struct epoll_event ev, events[MAX_EVENTS];
	int socketfd, nfds, epollfd;
	epollfd = epoll_create1(0);
	if (epollfd == -1)
	{
		logger(ERROR, "epoll_create1", std::cerr);
		exit(EXIT_FAILURE);
	}
	ev.events = EPOLLIN;
	for (std::map<int, const VirtualServer *>::iterator it =
				listening_sockfds.begin();
			it != listening_sockfds.end();
			it++)
	{
		ev.data.fd = it->first;
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, it->first, &ev) == -1)
		{
			logger(ERROR, "epoll_ctl: listen_sock", std::cerr);
			exit(EXIT_FAILURE);
		}
	}
	while (!g_stop)
	{
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, 1000);
		if (nfds == -1)
		{
			if (g_stop == 1)
				break ;
			logger(ERROR, "epoll_wait", std::cerr);
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < nfds; ++i)
		{
			socketfd = events[i].data.fd;
			std::map<int, int>::iterator it_cgi = cgi_output_map.find(socketfd);
			if (accept_connections(epollfd, socketfd))
				continue;
			else if (it_cgi != cgi_output_map.end())
				cgi_read_handler(epollfd, it_cgi->first);
			else
			{
				socketfd = events[i].data.fd;
				if (events[i].events & (EPOLLERR | EPOLLHUP))
				{
					clean_connection(epollfd, socketfd);
					continue ;
				}
				if (events[i].events & EPOLLIN)
					read_handler(epollfd, socketfd);
				if (events[i].events & EPOLLOUT)
					send_handler(epollfd, socketfd);
			}
		}
		close_inactive_connections(epollfd);
		check_cgi_timeouts(epollfd);
	}
	clean_all_connections(epollfd);
}

void Server::read_handler(int epollfd, int socketfd)
{
	int	status;

	if (active_connections.find(socketfd) == active_connections.end())
		return ;
	Connection &curr_connection = active_connections.at(socketfd);

	// instanciate current transaction if does not exist yet
	if (curr_connection.current_transaction == NULL)
		curr_connection.current_transaction =
			new HttpTransaction(curr_connection.server_config);
	status = curr_connection.read_full_recv();

	curr_connection.update_last_activity();
	if (status == READ_ERROR)
	{
		logger(ERROR, "READ_ERROR fd=" + ft_int_to_string(socketfd), std::cerr);
		clean_connection(epollfd, socketfd);
		return ;
	}
	if (status == SOCKET_FINISHED_READ)
	{
		curr_connection.set_keep_alive(false);
		if (curr_connection.current_transaction->state < HttpTransaction::ERROR_PARSING)
		{
			logger(INFO, "Client closed connection early. Cleaning up fd=" + ft_int_to_string(socketfd), std::cout);
			clean_connection(epollfd, socketfd);
			return;
		}
	}

	if (curr_connection.current_transaction->state < HttpTransaction::ERROR_PARSING)
		return;

	if (curr_connection.current_transaction->state >= HttpTransaction::ERROR_PARSING) {
		std::string host_header = "";
		if (curr_connection.current_transaction->request.headers.count("Host")) {
			host_header = curr_connection.current_transaction->request.headers["Host"];
		}

		const VirtualServer* correct_server = find_virtual_server(
			curr_connection.server_config->listen, 
			host_header
		);

		if (correct_server) {
			curr_connection.server_config = correct_server;
			curr_connection.current_transaction->vir_server = correct_server;
			
			curr_connection.current_transaction->location = curr_connection.current_transaction->find_location();

			logger(INFO, "Virtual server switched to: " + host_header, std::cout);
		}
	}

	curr_connection.insert_keep_alive_header();
	curr_connection.current_transaction->process_request(socketfd);
	if (curr_connection.current_transaction->state ==
		HttpTransaction::WAITING_CGI)
	{
		cgi_output_map.insert(
			std::make_pair(curr_connection.current_transaction->cgi_info.pipe_fd,
							curr_connection.socket_fd));
		add_cgifd_epoll(epollfd, curr_connection.current_transaction->cgi_info.pipe_fd);
	}
}

void Server::cgi_read_handler(int epollfd, int cgifd)
{
	int		status;
	char	buf[65536];
	int		client_fd;
	ssize_t	n;

	client_fd = cgi_output_map.at(cgifd);

	if (active_connections.find(client_fd) == active_connections.end())
	{
		clean_cgi_fd(epollfd, cgifd);
		return;
	}

	Connection &conn = active_connections.at(client_fd);
	n = read(cgifd, buf, sizeof(buf));
	if (n > 0)
	{
		conn.current_transaction->cgi_info.buffer.append(buf, n);
		return ;
	}
	clean_cgi_fd(epollfd, cgifd);
	if (n < 0)
	{
		logger(ERROR, "CGI pipe read returned -1", std::cerr);
		kill(conn.current_transaction->cgi_info.pid, SIGKILL);
		waitpid(conn.current_transaction->cgi_info.pid, NULL, 0);
		conn.current_transaction->build_error_response(500);
		return;
	}
	if (waitpid(conn.current_transaction->cgi_info.pid, &status, WNOHANG) > 0)
	{
		if (WIFEXITED(status) && WEXITSTATUS(status) > 0)
		{
			conn.current_transaction->build_error_response(500);
			return;
		}
	}

	// Requirement to pass the 42 tester;
	if (conn.current_transaction->cgi_info.buffer.length() > 8 && conn.current_transaction->cgi_info.buffer.substr(0, 8) == "Status: ")
	{
		conn.current_transaction->response._response_buffer = "HTTP/1.1 ";
		conn.current_transaction->response._response_buffer += 
			conn.current_transaction->cgi_info.buffer.substr(8, 
				conn.current_transaction->cgi_info.buffer.length() - 8);
	}
	else
		conn.current_transaction->response._response_buffer = conn.current_transaction->cgi_info.buffer;
	conn.set_keep_alive(false);
	conn.current_transaction->state = HttpTransaction::SENDING;
}

void Server::send_handler(int epollfd, int socketfd)
{
	if (active_connections.find(socketfd) == active_connections.end())
		return ;
	Connection &curr_connection = active_connections.at(socketfd);
	if (curr_connection.current_transaction == NULL)
		return ;
	if (curr_connection.current_transaction->state != HttpTransaction::SENDING)
		return ;
	curr_connection.update_last_activity();
	curr_connection.send_response();
	if (curr_connection.current_transaction->state ==
		HttpTransaction::COMPLETE)
	{
		if (curr_connection.get_keep_alive() == false)
		{
			clean_connection(epollfd, socketfd);
			return ;
		}
		curr_connection.current_transaction->request.clean_body_file();
		delete curr_connection.current_transaction;
		curr_connection.current_transaction = NULL;
	}
	else if (curr_connection.current_transaction->state ==
		HttpTransaction::SENDING_ERROR)
	{
		curr_connection.current_transaction->request.clean_body_file();
		clean_connection(epollfd, socketfd);
		return ;
	}
}

void Server::clean_connection(int epollfd, int socketfd)
{
	// Clean cgi pipe fds belonging to the connection
	std::map<int,int>::iterator it_cgi = cgi_output_map.begin();

	for (; it_cgi != cgi_output_map.end();)
	{
		if (it_cgi->second == socketfd)
		{
			std::map<int,int>::iterator to_erase = it_cgi;
			++it_cgi;
			logger(INFO, "Closed cgi_pipe with fd " + ft_int_to_string(to_erase->first), std::cout);
			epoll_ctl(epollfd, EPOLL_CTL_DEL, to_erase->first, NULL);
			close(to_erase->first);
			cgi_output_map.erase(to_erase++);
		}
		else
			++it_cgi;
	}

	// Clean socket connections
	std::map<int, Connection>::iterator it = active_connections.find(socketfd);
	if (it == active_connections.end())
		return ;
	if (it->second.current_transaction != NULL)
	{
		delete it->second.current_transaction;
		it->second.current_transaction = NULL;
	}
	remove_socket_epoll(epollfd, socketfd);
	close(socketfd);
	active_connections.erase(it);
	logger(CLOSE_SOCKET, "[Closed Connection]\t\t socket with fd " + ft_int_to_string(socketfd), std::cout);
}

void Server::clean_all_connections(int epollfd)
{
	// clean cgi fds
	std::map<int, int>::iterator it_cgi = cgi_output_map.begin();
	for (; it_cgi != cgi_output_map.end(); it_cgi++)
	{
		close(it_cgi->first);
	}
	// clean connection
	std::map<int, Connection>::iterator it_conn = active_connections.begin();
	while(it_conn != active_connections.end())
	{
		int fd = it_conn->first;
		it_conn++;
		clean_connection(epollfd, fd);
	}
	// clean listening sockets
	std::map<int, const VirtualServer *>::iterator it_list =
		listening_sockfds.begin();
	for (; it_list != listening_sockfds.end(); it_list++)
	{
		close(it_list->first);
	}
	// close epoll
	close(epollfd);
}

void Server::close_inactive_connections(int epollfd)
{
	std::map<int, Connection>::iterator it = active_connections.begin();
	std::map<int, Connection>::iterator it_end = active_connections.end();
	for (; it != it_end; it++)
	{
		if (it->second.is_timed_out(TIMEOUT_SECONDS))
		{
			logger(INFO, "Timeout of Connection in socket " + ft_int_to_string(it->second.socket_fd), std::cout);
			clean_connection(epollfd, it->second.socket_fd);
			if (active_connections.size() == 0)
				break ;
			it = active_connections.begin();
			it_end = active_connections.end();
		}
	}
}

void Server::clean_cgi_fd(int epollfd, int cgifd)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, cgifd, NULL);
	close(cgifd);
	cgi_output_map.erase(cgifd);
}

bool Server::accept_connections(int epollfd, int socketfd)
{
	int listen_sock, conn_sock;

	std::map<int, const VirtualServer *>::iterator it_listen_sock =	listening_sockfds.find(socketfd);
	if (it_listen_sock != listening_sockfds.end())
	{
		while (true)
		{
			listen_sock = it_listen_sock->first;
			conn_sock = accept_conn_socket(listen_sock);
			if (conn_sock == -1)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					break;
				logger(ERROR, "accept() failed: " + std::string(strerror(errno))
					+ " | active_connections: "
					+ ft_int_to_string(active_connections.size()), std::cerr);
				break;
			}

			if (active_connections.size() >= MAX_CONNECTIONS)
			{
				logger(WARNING, "Max connections reached. Dropping new connection.", std::cerr);
				close(conn_sock);
				continue;
			}

			logger(OPEN_SOCKET, "[New connection]\t\tsocket with fd " + ft_int_to_string(conn_sock) + " from listening socket " + ft_int_to_string(socketfd), std::cout);
			add_socket_epoll(epollfd, conn_sock);
			active_connections.insert(std::make_pair(conn_sock, Connection(conn_sock, it_listen_sock->second)));
		}
		return (true);
	}
	return (false);
}

// Operator overload
std::ostream &operator<<(std::ostream &os, const Server &s)
{
	os << "|||||||||| MAIN SERVER CONFIG ||||||||||\n";
	os << "Root: " << s.root << "\n";
	os << "Autoindex: " << (s.autoindex ? "on" : "off") << "\n";
	os << "Client Max Body Size: " << s.client_max_body_size << "\n";
	os << "Index Files: ";
	for (std::vector<std::string>::const_iterator it = s.index.begin();
			it != s.index.end();
			++it)
	{
		os << *it << " ";
	}
	os << "\n";
	os << "Error Pages:\n";
	for (std::map<int, std::string>::const_iterator it = s.error_page.begin();
			it != s.error_page.end();
			++it)
	{
		os << "  " << it->first << " -> " << it->second << "\n";
	}
	os << "\n--- Virtual Servers List ---\n";
	for (std::vector<VirtualServer>::const_iterator it =
				s.virtual_servers.begin();
			it != s.virtual_servers.end();
			++it)
	{
		os << *it << "\n";
	}
	os << "||||||||||||||||||||||||||||||||||||||||\n";
	return (os);
}


void Server::check_cgi_timeouts(int epollfd) {
	struct timeval now;
	gettimeofday(&now, NULL);

	std::map<int, int>::iterator it = cgi_output_map.begin();
	while (it != cgi_output_map.end()) {
		int cgifd = it->first;
		int client_fd = it->second;
		
		if (active_connections.find(client_fd) != active_connections.end()) {
			Connection &conn = active_connections.at(client_fd);
			if (conn.current_transaction && conn.current_transaction->state == HttpTransaction::WAITING_CGI) {
				
				long elapsed = now.tv_sec - conn.current_transaction->cgi_info.start_time.tv_sec;
				
				if (elapsed > MAX_CGI_RUNTIME) { // Timeout 10 sec
					logger(WARNING, "CGI Timeout for PID " + ft_int_to_string(conn.current_transaction->cgi_info.pid), std::cerr);
					
					// 1. Kill hanging procces
					kill(conn.current_transaction->cgi_info.pid, SIGKILL);
					waitpid(conn.current_transaction->cgi_info.pid, NULL, 0);
					
					// 2. Send 504 Gateway Timeout
					conn.current_transaction->build_error_response(504);

					// 3. Clean pipe
					clean_cgi_fd(epollfd, cgifd);
					
					it = cgi_output_map.begin();
					continue;
				}
			}
		} else {
			// Safety fallback: client is gone, but entry still exists in the map
			logger(WARNING, "Found orphaned CGI pipe fd=" + ft_int_to_string(cgifd) + " for missing client", std::cerr);

			// 1. Close the pipe and remove it from epoll
			clean_cgi_fd(epollfd, cgifd);

			// 2. Erase from the map and reset iterator to avoid invalid access
			cgi_output_map.erase(it);
			it = cgi_output_map.begin();
			continue;
		}
		++it;
	}
}

const VirtualServer* Server::find_virtual_server(int port, const std::string& host_header) {
	std::string host = host_header;
	size_t colon_pos = host.find(':');
	if (colon_pos != std::string::npos)
		host = host.substr(0, colon_pos);

	const VirtualServer* default_server = NULL;

	for (size_t i = 0; i < virtual_servers.size(); ++i) {
		if (virtual_servers[i].listen == port) {
			if (!default_server)
				default_server = &virtual_servers[i];
			
			if (virtual_servers[i].server_name == host)
				return &virtual_servers[i];
		}
	}
	return default_server;
}