#ifndef VIRTUALSERVER_HPP
#define VIRTUALSERVER_HPP

#include <string>
#include <list>
#include <iostream>

#include "../includes/Parser.hpp"
#include "../includes/Location.hpp"

class Server;

class VirtualServer
{
private:
public:
	VirtualServer(const t_virtual_server &vir_serv_config, const Server &server);
	~VirtualServer();

	std::vector<Location> locations;
	std::string	server_name;
	std::string root;
	int	listen;
	std::map<int, std::string> error_page;
	size_t client_max_body_size; 
	std::vector<std::string> index;
	bool autoindex;
};

std::ostream& operator<<(std::ostream& os, const VirtualServer &obj);

#endif