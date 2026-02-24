/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:21:58 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/24 14:09:15 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpRequest.hpp"
#include <string.h>


std::string methods_array[] = {"GET", "HEAD", "OPTIONS", "TRACE", "PUT",
	"DELETE", "POST", "PATCH", "CONNECT"};

std::vector<std::string> HttpRequest::methods(
	methods_array, methods_array + sizeof(methods_array) / sizeof(std::string));

HttpRequest::HttpRequest(): body_bytes_read(0) {}

HttpRequest::~HttpRequest()
{
	clean_body_file();
}

int HttpRequest::add_method(const std::string &input)
{
	std::vector<std::string>::iterator method_found;
	method_found = std::find(HttpRequest::methods.begin(),
								HttpRequest::methods.end(),
								input);
	if (method_found != HttpRequest::methods.end())
	{
		this->method = input;
		return (1);
	}
	else
		return (-1);
}

bool	HttpRequest::open_body_stream(int pid, int socket_fd)
{
	struct stat s;

	if (stat("./var/www/temp/", &s) == -1 && mkdir("./var/www/temp/", 0777) == -1)
	{
		return false;
	}
	body_file_path = "./var/www/temp/webserv_temp_" + ft_int_to_string(pid) + "_" + ft_int_to_string(socket_fd) + ".temp";
	_body_stream.open(body_file_path.c_str(), std::ios::binary | std::ios::trunc | std::ios::out);

	return (_body_stream.is_open());
}

bool	HttpRequest::write_to_body_stream(const char* data, size_t size)
{
	if (!_body_stream.is_open())
		return (false);
	
	_body_stream.write(data, size);
	if (_body_stream.good())
	{
		body_bytes_read += size;
		return (true);
	}
	return (false);
}

void	HttpRequest::close_body_stream()
{
	if (_body_stream.is_open())
		_body_stream.close();	
}

void	HttpRequest::clean_body_file()
{
	close_body_stream();
	if (!body_file_path.empty()) {
		std::remove(body_file_path.c_str());
		body_file_path.clear();
		body_bytes_read = 0;
	}
}

std::ostream &operator<<(std::ostream &os, const HttpRequest &req)
{
	os << req.method << " " << req.uri << " " << req.protocol << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it =
				req.headers.begin();
			it != req.headers.end();
			++it)
	{
		os << it->first << ": " << it->second << "\r\n";
	}
	os << "\r\n";
	// if (!req.body.empty())
	// {
	// 	os << req.body;
	// }
	return (os);
}
