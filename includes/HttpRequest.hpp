/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:18:09 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/24 11:44:32 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include "../includes/Location.hpp"
# include "../includes/utils.hpp"
# include <algorithm>
# include <iostream>
# include <map>
# include <string>
# include <vector>
# include <sys/stat.h>

class HttpRequest
{
private:
public:
	HttpRequest();
	~HttpRequest();

	static std::vector<std::string> methods;

	int 	add_method(const std::string &method);
	bool	open_body_stream(int pid, int socket_fd);
	bool	write_to_body_stream(const char* data, size_t size);
	void	close_body_stream();
	void	clean_body_file();

	std::string method;
	std::string uri;
	std::string protocol;
	std::map<std::string, std::string> headers;
	
	std::string	body_file_path;					// Path of the temp file where the body of the request is held
	size_t		body_bytes_read;				// Bytes of the body size
	std::ofstream _body_stream;					// File stream where the body of the request is held

	std::string	final_path;						// Actual path of the resource requested
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &req);

#endif
