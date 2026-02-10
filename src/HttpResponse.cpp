#include "../includes/HttpResponse.hpp"
#include "../includes/utils.hpp"

HttpResponse::HttpResponse(): _version("HTTP/1.1"), _bytes_sent(0)
{
}

HttpResponse::~HttpResponse()
{
}

void	HttpResponse::serialize_response()
{
	_response_buffer += this->_version;
	_response_buffer += " ";
	_response_buffer += ft_int_to_string(this->_status_code);
	_response_buffer += " ";

	std::map<int, std::string>::iterator it = http_status_codes.find(this->_status_code);
	if (it != http_status_codes.end())
		_response_buffer += it->second;
	else
		_response_buffer += "Internal Server Error";

	_response_buffer += "\r\n";
	_response_buffer += get_contentlength_header();
	_response_buffer += "\r\n";
	_response_buffer += this->_body;
	std::cout << "Response: " << _response_buffer << std::endl;
}

void	HttpResponse::build_response()
{
	set_status(200);
	serialize_response();
}

std::string	HttpResponse::get_contentlength_header()
{
	std::string contentlength_header;

	contentlength_header += "Content-length: ";
	contentlength_header += ft_int_to_string(_body.size());
	contentlength_header += "\r\n";
	return(contentlength_header);
}

void	HttpResponse::set_status(int code)
{
	_status_code = code;
}

void	HttpResponse::set_body(const std::string& body)
{
	this->_body = body;
}

std::string	HttpResponse::get_body()
{
	return (this->_body);
}


void	HttpResponse::add_header(const std::string& key, const std::string& value)
{
	_headers.insert(std::make_pair(key, value));
}

std::map<int, std::string> HttpResponse::initHttpStatusCodes() {
	std::map<int, std::string> m;

	// Informational
	m[100] = "Continue";
	m[101] = "Switching Protocols";
	m[102] = "Processing";
	m[103] = "Early Hints";

	// Success
	m[200] = "OK";
	m[201] = "Created";
	m[202] = "Accepted";
	m[203] = "Non-Authoritative Information";
	m[204] = "No Content";
	m[205] = "Reset Content";
	m[206] = "Partial Content";
	m[207] = "Multi-Status";
	m[208] = "Already Reported";
	m[226] = "IM Used";

	// Redirection
	m[300] = "Multiple Choices";
	m[301] = "Moved Permanently";
	m[302] = "Found";
	m[303] = "See Other";
	m[304] = "Not Modified";
	m[305] = "Use Proxy";
	m[307] = "Temporary Redirect";
	m[308] = "Permanent Redirect";

	// Client Error
	m[400] = "Bad Request";
	m[401] = "Unauthorized";
	m[402] = "Payment Required";
	m[403] = "Forbidden";
	m[404] = "Not Found";
	m[405] = "Method Not Allowed";
	m[406] = "Not Acceptable";
	m[407] = "Proxy Authentication Required";
	m[408] = "Request Timeout";
	m[409] = "Conflict";
	m[410] = "Gone";
	m[411] = "Length Required";
	m[412] = "Precondition Failed";
	m[413] = "Content Too Large";
	m[414] = "URI Too Long";
	m[415] = "Unsupported Media Type";
	m[416] = "Range Not Satisfiable";
	m[417] = "Expectation Failed";
	m[418] = "I'm a teapot";
	m[421] = "Misdirected Request";
	m[422] = "Unprocessable Content";
	m[423] = "Locked";
	m[424] = "Failed Dependency";
	m[425] = "Too Early";
	m[426] = "Upgrade Required";
	m[428] = "Precondition Required";
	m[429] = "Too Many Requests";
	m[431] = "Request Header Fields Too Large";
	m[451] = "Unavailable For Legal Reasons";

	// Server Error
	m[500] = "Internal Server Error";
	m[501] = "Not Implemented";
	m[502] = "Bad Gateway";
	m[503] = "Service Unavailable";
	m[504] = "Gateway Timeout";
	m[505] = "HTTP Version Not Supported";
	m[506] = "Variant Also Negotiates";
	m[507] = "Insufficient Storage";
	m[508] = "Loop Detected";
	m[510] = "Not Extended";
	m[511] = "Network Authentication Required";

	return m;
}

std::map<int, std::string> HttpResponse::http_status_codes = HttpResponse::initHttpStatusCodes();
