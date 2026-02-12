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


	void	add_header(const std::string& key, const std::string& value);
	std::string	serialize_headers();
	void	serialize_response();
	void	build_response(int status_code);

	std::string	_response_buffer;
	size_t		_bytes_sent;
};

std::ostream& operator<<(std::ostream& os, const HttpResponse& resp);

#endif