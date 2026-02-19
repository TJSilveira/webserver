/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:22:12 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/19 15:54:05 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpResponse.hpp"
#include "../includes/utils.hpp"

HttpResponse::HttpResponse()
	: _version("HTTP/1.1"), _bytes_sent(0)
{
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::serialize_response()
{
	add_header("Content-length", ft_int_to_string(_body.size()));
	_response_buffer += this->_version;
	_response_buffer += " ";
	_response_buffer += ft_int_to_string(this->_status_code);
	_response_buffer += " ";
	std::map<int, std::string>::iterator it =
		http_status_codes.find(this->_status_code);
	if (it != http_status_codes.end())
		_response_buffer += it->second;
	else
		_response_buffer += "Internal Server Error";
	_response_buffer += "\r\n";
	_response_buffer += serialize_headers();
	_response_buffer += "\r\n";
	_response_buffer += this->_body;
}

void HttpResponse::build_response(int status_code)
{
	set_status(status_code);
	serialize_response();
}

std::string HttpResponse::serialize_headers()
{
	std::string headers_str;
	std::map<std::string, std::string>::iterator it = _headers.begin();
	std::map<std::string, std::string>::iterator it_end = _headers.end();
	for (; it != it_end; it++)
	{
		headers_str += it->first;
		headers_str += ": ";
		headers_str += it->second;
		headers_str += "\r\n";
	}
	return (headers_str);
}

void HttpResponse::set_status(int code)
{
	_status_code = code;
}

void HttpResponse::set_body(const std::string &body)
{
	this->_body = body;
}

std::string HttpResponse::get_body()
{
	return (this->_body);
}

void HttpResponse::add_header(const std::string &key,
								const std::string &value)
{
	_headers.insert(std::make_pair(key, value));
}

std::map<int, std::string> HttpResponse::initHttpStatusCodes()
{
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
	return (m);
}

std::map<int, std::string> HttpResponse::http_status_codes =
	HttpResponse::initHttpStatusCodes();

#include <fcntl.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

std::ostream &operator<<(std::ostream &os, const HttpResponse &resp)
{
	os << resp._response_buffer.substr(0, std::min(resp._response_buffer.size(), (size_t)500));
	return (os);
}
