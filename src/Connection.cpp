/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:21:35 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/18 00:30:25 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Connection.hpp"

Connection::Connection(int socket_fd, const VirtualServer *server_config)
	: keep_alive(true), socket_fd(socket_fd), server_config(server_config),
		current_transaction(NULL)
{
	update_last_activity();
}

Connection::~Connection()
{
		delete current_transaction;

	if (current_transaction)
	{
	}
}

void Connection::insert_keep_alive_header()
{
	if (current_transaction == NULL)
		return ;
	if (current_transaction->request.headers.count("Connection"))
	{
		if (current_transaction->request.headers.at("Connection") == "keep-alive")
			keep_alive = true;
		else
			keep_alive = false;
	}
	if (keep_alive == true)
		current_transaction->response.add_header("Connection", "keep-alive");
	else
		current_transaction->response.add_header("Connection", "close");
}

bool Connection::get_keep_alive()
{
	return (keep_alive);
}

void Connection::update_last_activity()
{
	gettimeofday(&last_activity, NULL);
}

bool Connection::is_timed_out(int timeout_seconds) const
{
	struct timeval now;
	gettimeofday(&now, NULL);

	long elapsed = (now.tv_sec - last_activity.tv_sec);

	return (elapsed > timeout_seconds);
}

void Connection::close_connection()
{
	if (socket_fd != -1)
	{
		close(socket_fd);
		socket_fd = -1;
	}
}

int Connection::read_full_recv()
{
	ssize_t	bytes_received;

	std::string buffer;
	buffer.resize(BUFFER_SIZE);
	bytes_received = recv(socket_fd, &buffer[0], BUFFER_SIZE, 0);

	std::cout << "This is the buffer: '" << buffer << "';" << std::endl;
	// std::cout << "This is the buffer: '";
	// for (size_t i = 0; i < buffer.size(); i++)
	// {
	// 	if (buffer.at(i) == '\r')
	// 	{
	// 		std::cout << "\\r";
	// 	}
	// 	else if (buffer.at(i) == '\n')
	// 	{
	// 		std::cout << "\\n";
	// 	}
	// 	else
	// 		std::cout << buffer.at(i);
	// }
	// std::cout << "';" << std::endl;

	if (bytes_received > 0)
	{
		buffer.resize(bytes_received);
		current_transaction->parse(buffer);
		return (BUFFER_READ);
	}
	else if (bytes_received == 0) // Client closed write side OR there is nothing more to read
		return (SOCKET_FINISHED_READ);
	else
		return (READ_ERROR);
}

void Connection::send_response()
{
	HttpResponse &response = this->current_transaction->response;
	size_t missing_bytes =
		response._response_buffer.size() - response._bytes_sent;
	ssize_t current_sent_bytes =
		send(socket_fd, &response._response_buffer[response._bytes_sent],
				missing_bytes, MSG_NOSIGNAL);
	if (current_sent_bytes > 0)
	{
		response._bytes_sent += current_sent_bytes;
	}
	else
	{
		// Fatal error
		std::cerr << "Send error\n";
		return ;
	}
	if (response._bytes_sent < response._response_buffer.size())
		return ;
	std::cout << "Finalized send\n";
	current_transaction->mark_as_complete();
}
