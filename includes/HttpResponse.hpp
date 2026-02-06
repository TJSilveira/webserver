#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>

class HttpResponse
{
private:
	std::string _version;
	int 		_status_code;
	std::map<std::string, std::string> _headers;
	std::string _body;
public:
	HttpResponse();
	~HttpResponse();

	static std::map<int, std::string> http_status_codes;

	void	set_status(int code);
	void	set_body(const std::string& body);
	void	add_header(const std::string& key, const std::string& value);
	void	serialize_response();
	void	build_response();

	std::string	_response_buffer;
	size_t		_bytes_sent;
};

#endif