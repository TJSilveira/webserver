<<<<<<< Updated upstream
=======
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amoiseik <amoiseik@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:18:20 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/20 16:33:00 by amoiseik         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

>>>>>>> Stashed changes
#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>
#include <iostream>

class HttpResponse
{
private:
	std::string _version;
	int 		_status_code;
	std::map<std::string, std::string> _headers;
	std::string _body;
	bool _is_head_method; //added
	

	static std::map<int, std::string> initHttpStatusCodes();
public:
	HttpResponse();
	~HttpResponse();

	static std::map<int, std::string> http_status_codes;

	// Setters
	void	set_status(int code);
	void	set_body(const std::string& body);

	// Getters
	std::string	get_body();


<<<<<<< Updated upstream
	void	add_header(const std::string& key, const std::string& value);
	std::string	serialize_headers();
	void	serialize_response();
	void	build_response(int status_code);
=======
	std::string _response_buffer;
	size_t _bytes_sent;

	void set_head_method(bool val) {
		_is_head_method = val;
	}
};
>>>>>>> Stashed changes

	std::string	_response_buffer;
	size_t		_bytes_sent;
};

#endif