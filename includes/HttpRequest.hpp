/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:18:09 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/18 18:00:18 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <algorithm>
# include <iostream>
# include <map>
# include <string>
# include <vector>

class HttpRequest
{
private:
public:
  HttpRequest();
	~HttpRequest();

	static std::vector<std::string> methods;

	int add_method(const std::string &method);

	std::string method;
	std::string uri;
	std::string protocol;
	std::map<std::string, std::string> headers;
	std::string body;
	std::string	final_path;						// Actual path of the resource requested
	std::string	final_dir;						// Actual path of the resource requested
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &req);

#endif
