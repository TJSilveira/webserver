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

	void	set_status(int code);
	void	set_body(const std::string& body);
	void	add_header(const std::string& key, const std::string& value);
	std::string	get_contentlength_header();
	void	serialize_response();
	void	build_response();

	std::string	_response_buffer;
	size_t		_bytes_sent;
};

#endif