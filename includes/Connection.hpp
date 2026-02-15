#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <unistd.h>
#include <string>
#include <ctime>
#include <sys/socket.h>
#include <sys/time.h>
#include "HttpTransaction.hpp"
#include "CgiHandler.hpp"

#define BUFFER_SIZE 8192
#define CLIENT_CLOSE_SOCKET 1
#define BUFFER_READ 0
#define READ_ERROR -1

class Connection
{
private:
	struct timeval	last_activity;
	bool			keep_alive;
public:
	Connection(int socket_fd, const VirtualServer* server_config);
	~Connection();

	void	close_connection();
	void	update_last_activity();
	void	insert_keep_alive_header();
	bool	is_timed_out(int timeout_seconds) const;
	int 	read_full_recv();
	void	send_response();

	// Getters
	bool	get_keep_alive();

	int						socket_fd;
	const VirtualServer*	server_config;

	HttpTransaction*		current_transaction;
};

#endif