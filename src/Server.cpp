#include "../includes/Server.hpp"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <vector>

std::string directives_array[] = {
	"listen",
	"server_name",
	"return",
	"upload_store",
	"alias",
	"cgi_path",
	"cgi_ext",
	"allow_methods",
	"root",
	"error_page",
	"client_max_body_size",
	"index",
	"autoindex"
};

std::vector<std::string>  Server::directives(directives_array, directives_array +  sizeof(directives_array) / sizeof(std::string));

std::string context_array[] = {
	"http",
	"server",
	"location"
};

std::vector<std::string>  Server::context(context_array, context_array +  sizeof(context_array) / sizeof(std::string));


// Constructors
Server::Server(void)
{
	
}

// Destructor
Server::~Server(void) {}
