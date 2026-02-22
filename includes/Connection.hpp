/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:18:02 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/21 23:10:48 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "CgiHandler.hpp"
# include "HttpTransaction.hpp"
# include <ctime>
# include <sys/socket.h>
# include <sys/time.h>
# include <unistd.h>

# define BUFFER_SIZE 65536
# define SOCKET_FINISHED_READ 0
# define BUFFER_READ 1
# define READ_ERROR -1

class Connection
{
private:
	struct timeval last_activity;
	bool keep_alive;

public:
	Connection(int socket_fd, const VirtualServer *server_config);
	~Connection();

	void close_connection();
	void update_last_activity();
	void insert_keep_alive_header();
	bool is_timed_out(int timeout_seconds) const;
	int read_full_recv();
	void send_response();

	// Getters
	bool get_keep_alive();

	// Setter
	void set_keep_alive(bool status);

	int socket_fd;
	const VirtualServer *server_config;

	HttpTransaction *current_transaction;
};

#endif
