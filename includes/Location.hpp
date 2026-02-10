#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <list>
#include <iostream>

#include "../includes/Parser.hpp"

class VirtualServer;

class Location
{
private:
public:

	Location(const t_location &location_config, const VirtualServer &vir_serv);
	~Location();

	std::string path; // location path
	std::string root; // root folder
	std::map<int, std::string> error_page; // Error pages 
	size_t client_max_body_size; // max size of content sent by the client
	std::vector<std::string> index; // default page to be sent
	bool autoindex; // default page to be sent
	std::string	alias; // Replaces the matched location prefix with a specified path
	std::string upload_store; // Specifies the directory where uploaded files should be stored when using the PUT method.
	std::vector<std::string> cgi_ext; // What cgi extensions to expect
	std::string cgi_path; // where the cgi binaries are stored 
	std::vector<std::string> allow_methods; // What methods are allowed to be used
};

std::ostream& operator<<(std::ostream& os, const Location &obj);

#endif