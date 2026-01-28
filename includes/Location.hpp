#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <list>
#include <iostream>

#include "../includes/VirtualServer.hpp"

class Location
{
private:
public:

	Location(VirtualServer vs,std::string &config_str);
	~Location();

	std::string root; // root folder
	std::unordered_map<int, std::string> error_page; // Error pages 
	size_t client_max_body_size; // max size of content sent by the client
	std::vector<std::string> index; // default page to be sent
	bool autoindex; // default page to be sent
	std::string	alias; // Replaces the matched location prefix with a specified path
	std::string upload_store; // Specifies the directory where uploaded files should be stored when using the PUT method.
	std::string cgi_ext; // What cgi extensions to expect
	std::string cgi_path; // where the cgi binaries are stored 
};

std::ostream& operator<<(std::ostream& os, const Location &obj);

#endif