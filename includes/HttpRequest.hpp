#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <iostream>

class HttpRequest
{
private:
	/* data */
public:
	HttpRequest();
	~HttpRequest();

	static std::vector<std::string>	methods;

	int	add_method(const std::string &method);

	std::string	method;
	std::string	uri;
	std::string	protocol;
	std::map<std::string, std::string> headers;
	std::string	body;
};

std::ostream& operator<<(std::ostream& os, const HttpRequest& req);

#endif