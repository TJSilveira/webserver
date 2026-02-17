/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:18:20 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/17 09:06:54 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <iostream>
# include <map>
# include <string>

class HttpResponse
{
private:
	std::string _version;
	int _status_code;
	std::map<std::string, std::string> _headers;
	std::string _body;

	static std::map<int, std::string> initHttpStatusCodes();

public:
	HttpResponse();
	~HttpResponse();

	static std::map<int, std::string> http_status_codes;

	// Setters
	void set_status(int code);
	void set_body(const std::string &body);

	// Getters
	std::string get_body();

	void add_header(const std::string &key, const std::string &value);
	std::string serialize_headers();
	void serialize_response();
	void build_response(int status_code);

	std::string _response_buffer;
	size_t _bytes_sent;
};

std::ostream &operator<<(std::ostream &os, const HttpResponse &resp);

#endif
