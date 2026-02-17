/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:23:47 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/17 16:01:49 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/globals.hpp"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <vector>

#define TIMEOUT_SECONDS 10

std::string directives_array[] = {"listen",
									"server_name",
									"return",
									"upload_store",
									"alias",
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
			bool autoindex_status;

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
			this->client_max_body_size = atoi(curr_directive.args.at(0).c_str());
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
			std::cout << "Successfully bound port: " << port
						<< " with fd: " << fd << std::endl;
		}
		else
		{
			perror("Port already in use");
			exit(EXIT_FAILURE);
		}
	}
	std::cout << "Server waiting for connections...\n";
}

void Server::run_server()
{
	struct epoll_event ev, events[MAX_EVENTS];
	int listen_sock, conn_sock, nfds, epollfd;
	epollfd = epoll_create1(0);
	if (epollfd == -1)
	{
		perror("epoll_create1");
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
			perror("epoll_ctl: listen_sock");
			exit(EXIT_FAILURE);
		}
	}
	while (!g_stop)
	{
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, 100);
		if (nfds == -1)
		{
			if (g_stop == 1)
				break ;
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < nfds; ++i)
		{
			std::map<int, int>::iterator it_cgi =cgi_output_map.find(events[i].data.fd);
			std::map<int, const VirtualServer *>::iterator it_listen_sock =	listening_sockfds.find(events[i].data.fd);
			if (it_listen_sock != listening_sockfds.end())
			{
				listen_sock = it_listen_sock->first;
				conn_sock = accept_conn_socket(listen_sock);
				if (conn_sock != -1)
				{
					add_socket_epoll(epollfd, conn_sock);
					active_connections.insert(std::make_pair(
						conn_sock, Connection(conn_sock,
								it_listen_sock->second)));
				}
			}
			else if (it_cgi != cgi_output_map.end())
			{
				cgi_read_handler(epollfd, it_cgi->first);
			}
			else
			{
				// std::cout << "Connection socket that needs attention: " << events[i].data.fd << std::endl;
				conn_sock = events[i].data.fd;
				if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
				{
					clean_connection(epollfd, conn_sock);
					continue ;
				}
				if (events[i].events & EPOLLIN)
					read_handler(epollfd, conn_sock);
				if (events[i].events & EPOLLOUT)
					send_handler(epollfd, conn_sock);
			}
		}
		close_inactive_connections(epollfd);
	}
	clean_all_connections(epollfd);
}

void Server::read_handler(int epollfd, int socketfd)
{
	int	status;

	// std::cout << "Inside the read\n";
	if (active_connections.find(socketfd) == active_connections.end())
		return ;
	Connection &curr_connection = active_connections.at(socketfd);
	// instanciate current transaction if does not exist yet
	if (curr_connection.current_transaction == NULL)
		curr_connection.current_transaction =
			new HttpTransaction(curr_connection.server_config);
	curr_connection.update_last_activity();
	status = curr_connection.read_full_recv();
	// std::cout << "After read_full_recv\n";
	// std::cout << "The state of the transaction is " << curr_connection.current_transaction->state << std::endl;
	if (status == READ_ERROR)
	{
		std::cout << "CLEANING IN THE 'READ_ERROR'\n";
		clean_connection(epollfd, socketfd);
		return ;
	}
	if (status == SOCKET_FINISHED_READ)
	{
		// I think this is wrong. We need to find a way to send the information before closing.
		std::cout << "Client closed connection (EOF)\n";
		clean_connection(epollfd, socketfd);
		return;
	}

	if (curr_connection.current_transaction->state < HttpTransaction::PARSING_ERROR)
		return;
	
	curr_connection.insert_keep_alive_header();
	curr_connection.current_transaction->process_request(epollfd, socketfd);
	std::cout << "State of the transaction after the process_request: " <<curr_connection.current_transaction->state <<std::endl;
	if (curr_connection.current_transaction->state ==
		HttpTransaction::WAITING_CGI) // The problem is that the program is not entering this part of the code
	{
		std::cout << "IN THIS IF STATEMENT HttpTransaction::WAITING_CGI \n";
		cgi_output_map.insert(
			std::make_pair(curr_connection.current_transaction->cgi_info.pipe_fd,
							curr_connection.socket_fd));
		add_cgifd_epoll(epollfd, curr_connection.current_transaction->cgi_info.pipe_fd);
	}
	print_req_resp(curr_connection);
}

void Server::cgi_read_handler(int epollfd, int cgifd)
{
	int		status;
	char	buf[4096];
	int		client_fd;
	ssize_t	n;

	client_fd = cgi_output_map.at(cgifd);
	Connection &conn = active_connections.at(client_fd);
	n = read(cgifd, buf, sizeof(buf));
	if (n > 0)
	{
		conn.current_transaction->cgi_info.buffer.append(buf, n);
		return ;
	}
	epoll_ctl(epollfd, EPOLL_CTL_DEL, cgifd, NULL);
	std::cout << "input_doc_path: " << conn.current_transaction->cgi_info.input_doc_path.c_str() << std::endl;
	close(cgifd);
	cgi_output_map.erase(cgifd);
	waitpid(cgifd, &status, WNOHANG);
	conn.current_transaction->response._response_buffer = conn.current_transaction->cgi_info.buffer;
	conn.current_transaction->state = HttpTransaction::SENDING;
	change_socket_epollout(epollfd, client_fd);
}

void Server::send_handler(int epollfd, int socketfd)
{
	if (active_connections.find(socketfd) == active_connections.end())
		return ;
	Connection &curr_connection = active_connections.at(socketfd);
	if (curr_connection.current_transaction == NULL)
		return ;
	curr_connection.update_last_activity();
	curr_connection.send_response();
	if (curr_connection.current_transaction->state ==
		curr_connection.current_transaction->COMPLETE)
	{
		if (curr_connection.get_keep_alive() == false)
		{
			clean_connection(epollfd, socketfd);
			return ;
		}
		if (!curr_connection.current_transaction->cgi_info.input_doc_path.empty())
			unlink(curr_connection.current_transaction->cgi_info.input_doc_path.c_str());
		delete curr_connection.current_transaction;
		curr_connection.current_transaction = NULL;
		change_socket_epollin(epollfd, socketfd);
	}
}

void Server::clean_connection(int epollfd, int socketfd)
{
	std::map<int, Connection>::iterator it = active_connections.find(socketfd);
	if (it == active_connections.end())
		return ;
	if (it->second.current_transaction != NULL)
	{
		delete it->second.current_transaction;
		it->second.current_transaction = NULL;
	}
	// if(!it->second.current_transaction->cgi_info.input_doc_path.empty())
	// {
	// 	unlink(it->second.current_transaction->cgi_info.input_doc_path.c_str());
	// 	it->second.current_transaction->cgi_info.input_doc_path.clear();
	// }
	remove_socket_epoll(epollfd, socketfd);
	close(socketfd);
	active_connections.erase(it);
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
			std::cout << "Timeout of Connection in socket " << it->second.socket_fd
						<< std::endl;
			clean_connection(epollfd, it->second.socket_fd);
			if (active_connections.size() == 0)
				break ;
			it = active_connections.begin();
			it_end = active_connections.end();
		}
	}
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


void Server::print_req_resp(const Connection &curr_connection)
{
	std::cout << "==== THIS IS THE REQUEST ====\n";
	std::cout << curr_connection.current_transaction->request;
	std::cout << "==== END OF THE REQUEST ====\n";
	std::cout << "==== THIS IS THE response ====\n";
	std::cout << curr_connection.current_transaction->response;
	std::cout << "==== END OF THE response ====\n";
}