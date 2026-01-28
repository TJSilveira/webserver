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
};

std::ostream& operator<<(std::ostream& os, const VirtualServer &obj);

#endif