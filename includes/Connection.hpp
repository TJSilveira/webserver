#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include "HttpTransaction.hpp"

class Connection
{
private:
public:
	Connection(int socket_fd, const VirtualServer* server_config);
	~Connection();

	void	close_connection();

	int			socket_fd;
	const VirtualServer* server_config;

	HttpTransaction*	current_transaction;
};

#endif