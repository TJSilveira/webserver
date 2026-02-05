#include "../includes/Connection.hpp"

Connection::Connection(int socket_fd, const VirtualServer* server_config):
	socket_fd(socket_fd),
	server_config(server_config),
	current_transaction(NULL) {}

Connection::~Connection()
{
	if (current_transaction)
	{
		delete current_transaction;
	}
}

void	Connection::close_connection()
{
	if (socket_fd != -1)
	{
		close(socket_fd);
		socket_fd = -1;
	}
}