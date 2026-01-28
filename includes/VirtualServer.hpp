#ifndef VIRTUALSERVER_HPP
#define VIRTUALSERVER_HPP

#include <string>
#include <list>
#include <iostream>

#include "../includes/Server.hpp"

class VirtualServer
{
private:
public:

	VirtualServer(Server server, std::string &config_str);
	~VirtualServer();

	std::string root;
	std::unordered_map<int, std::string> error_page;
	size_t client_max_body_size; 
	std::vector<std::string> index;
	bool autoindex;
	int	listen;
	std::string	server_name;
};

std::ostream& operator<<(std::ostream& os, const VirtualServer &obj);

#endif