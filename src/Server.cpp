#include "../includes/Server.hpp"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <vector>

#define BUFFER_SIZE 8192
#define CLIENT_CLOSE_SOCKET 1
#define BUFFER_READ 0
#define READ_ERROR -1


std::string directives_array[] = {
	"listen",
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
	"autoindex"
};

std::vector<std::string>  Server::directives(directives_array, directives_array +  sizeof(directives_array) / sizeof(std::string));

std::string context_array[] = {
	"http",
	"server",
	"location"
};

std::vector<std::string>  Server::context(context_array, context_array +  sizeof(context_array) / sizeof(std::string));

// Constructors
Server::Server(t_server &server_config):
	root("/var/www/html"),
	client_max_body_size(1000000),
	autoindex (false)
{
	for (size_t i = 0; i < server_config.dir.size(); i++)
	{
		t_directive curr_directive = server_config.dir.at(i);
		if (curr_directive.name == "root")
		{
			this->root = curr_directive.args.at(0);
		}
		else if (curr_directive.name == "error_page")
		{
			int last_index = curr_directive.args.size() - 1;
			std::string error_page_path = curr_directive.args.at(last_index);
			for (size_t j = 0; j < curr_directive.args.size() - 1; j++)
			{
				int	error_num = atoi(curr_directive.args.at(j).c_str());
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
			bool autoindex_status;
			if (curr_directive.args.size() == 1 && curr_directive.args.at(0) == "on")
				autoindex_status = true;
			else if (curr_directive.args.size() == 1 && curr_directive.args.at(0) == "off")
				autoindex_status = false;
			else
			{
				throw ConfigError("autoindex needs to be either 'on' or 'off'", curr_directive.args.at(0));
			}
			this->autoindex = autoindex_status;
		}
		else
		{
			throw ConfigError("directive not allowed in a server block", curr_directive.name);
		}
	}
	for (size_t i = 0; i < server_config.vir_servers.size(); i++)
	{
		this->virtual_servers.push_back(VirtualServer(server_config.vir_servers.at(i), *this));
	}
}

// Destructor
Server::~Server(void) {}

void Server::init()
{
	std::set<int> bound_ports;

	for (size_t i = 0; i < virtual_servers.size(); i++)
	{
		int port = virtual_servers.at(i).listen;

		if(bound_ports.find(port) == bound_ports.end())
		{
			int fd = create_listening_socket(port, "0.0.0.0");
			listening_sockfds.insert(std::make_pair(fd, &virtual_servers.at(i)));
			bound_ports.insert(port);
			std::cout << "Successfully bound port: " << port << " with fd: " << fd << std::endl;
		}
		else
		{
			perror("Port already in use");
			exit(EXIT_FAILURE);
		}
	}
	std::cout << "Server waiting for connections...\n";
}

void	Server::run_server()
{
	struct epoll_event ev, events[MAX_EVENTS];
	int listen_sock, conn_sock, nfds, epollfd;

	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;

	for (std::map<int, const VirtualServer*>::iterator it = listening_sockfds.begin(); it != listening_sockfds.end(); it++)
	{
		ev.data.fd = it->first;
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, it->first, &ev) == -1) {
			perror("epoll_ctl: listen_sock");
			exit(EXIT_FAILURE);
		}
	}

	// TODO: Add limits to the number of sockets that are checked by epoll
	while(true) {
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < nfds; ++i) {
			std::map<int, const VirtualServer*>::iterator it_listen_sock = listening_sockfds.find(events[i].data.fd);
			if (it_listen_sock != listening_sockfds.end()){
				listen_sock = it_listen_sock->first;
				conn_sock = accept_conn_socket(listen_sock);
				add_socket_epoll(epollfd, conn_sock);
				active_connections.insert(std::make_pair(conn_sock, Connection(conn_sock, it_listen_sock->second)));
			} else {
				std::cout << "Connection socket that needs attention: " << events[i].data.fd << std::endl;
				int curr_socket = events[i].data.fd;
				Connection &curr_connection = active_connections.at(curr_socket);
				// Read buffer
				if(events[i].events & EPOLLIN)
				{
					std::cout << "Inside the read\n";
					// instanciate current transaction if does not exist yet
					if (curr_connection.current_transaction == NULL)
						curr_connection.current_transaction = new HttpTransaction(curr_connection.server_config);
					
					int status = read_full_recv(curr_socket, curr_connection);
					if(status == READ_ERROR)
					{
						std::cout << "CLEANING IN THE 'READ_ERROR'\n";
						clean_connection(epollfd, curr_socket, curr_connection);
						continue;
					}
					curr_connection.current_transaction->process_request(epollfd, curr_socket);
				}
				if(events[i].events & EPOLLOUT)
				{
					if (curr_connection.current_transaction == NULL)
						continue;
					curr_connection.current_transaction->send_response(curr_socket);
					if (curr_connection.current_transaction->state == curr_connection.current_transaction->COMPLETE)
					{
						delete curr_connection.current_transaction;
						curr_connection.current_transaction = NULL;
						change_socket_epollin(epollfd, curr_socket);
					}
				}
				if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
					clean_connection(epollfd, curr_socket, curr_connection);
			}
		}
	}
}

// TODO: Breakdown the function further
int Server::read_full_recv(int curr_socket, Connection &curr_connection)
{
	std::string buffer;
	while (true)
	{
		buffer.clear();
		buffer.resize(BUFFER_SIZE);
		ssize_t bytes_received = recv(curr_socket, &buffer[0], BUFFER_SIZE, 0);
		std::cout << "This is the buffer: " << buffer << std::endl; 
		if (bytes_received > 0)
		{
			buffer.resize(bytes_received);
			curr_connection.current_transaction->parse(buffer);
		}
		else if(bytes_received == 0) // Client closed write side OR there is nothing more to read
		{
			return (CLIENT_CLOSE_SOCKET);
		}
		else if (bytes_received < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
		{
			std::cout << "Errno Break\n";
			return (BUFFER_READ);
		}
		else
		{
			// To add the correct exception
			std::cout << "Else break\n";
			return (READ_ERROR);
		}
	}
	std::cout << "At the end of read\n";
	std::cout << curr_connection.current_transaction->request;
	return (-1);
}

void Server::clean_connection(int epollfd, int curr_socket, Connection &curr_connection)
{
	if (curr_connection.current_transaction != NULL)
		delete curr_connection.current_transaction;
	remove_socket_epoll(epollfd, curr_socket);
	active_connections.erase(curr_socket);
	close(curr_socket);
}

// Operator overload
std::ostream& operator<<(std::ostream& os, const Server& s) {
	os << "|||||||||| MAIN SERVER CONFIG ||||||||||\n";
	os << "Root: " << s.root << "\n";
	os << "Autoindex: " << (s.autoindex ? "on" : "off") << "\n";
	os << "Client Max Body Size: " << s.client_max_body_size << "\n";

	os << "Index Files: ";
	for (std::vector<std::string>::const_iterator it = s.index.begin(); it != s.index.end(); ++it) {
		os << *it << " ";
	}
	os << "\n";

	os << "Error Pages:\n";
	for (std::map<int, std::string>::const_iterator it = s.error_page.begin(); it != s.error_page.end(); ++it) {
		os << "  " << it->first << " -> " << it->second << "\n";
	}

	os << "\n--- Virtual Servers List ---\n";
	for (std::vector<VirtualServer>::const_iterator it = s.virtual_servers.begin(); it != s.virtual_servers.end(); ++it) {
		os << *it << "\n";
	}
	os << "||||||||||||||||||||||||||||||||||||||||\n";
	return os;
}

