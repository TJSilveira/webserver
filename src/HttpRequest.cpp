#include "../includes/HttpRequest.hpp"

std::string methods_array[] = {
	"GET",
	"HEAD",
	"OPTIONS",
	"TRACE",
	"PUT",
	"DELETE",
	"POST",
	"PATCH",
	"CONNECT"
};

std::vector<std::string>  HttpRequest::methods(methods_array, methods_array +  sizeof(methods_array) / sizeof(std::string));

HttpRequest::HttpRequest()
{
}

HttpRequest::~HttpRequest()
{
}

int	HttpRequest::add_method(const std::string &input)
{
	std::vector<std::string>::iterator method_found;

	method_found = std::find(HttpRequest::methods.begin(), HttpRequest::methods.end(), input);
	if (method_found != HttpRequest::methods.end())
	{
		this->method = input;
		return (1);
	}
	else
		return (-1);
}

std::ostream& operator<<(std::ostream& os, const HttpRequest& req) {
    // 1. Output the Request Line: METHOD URI PROTOCOL
    os << req.method << " " << req.uri << " " << req.protocol << "\r\n";

    // 2. Output Headers: KEY: VALUE
    for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); 
         it != req.headers.end(); ++it) {
        os << it->first << ": " << it->second << "\r\n";
    }

    // 3. Output the mandatory empty line separating headers from body
    os << "\r\n";

    // 4. Output the Body if it exists
    if (!req.body.empty()) {
        os << req.body;
    }

    return os;
}
