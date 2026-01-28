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
};

std::ostream& operator<<(std::ostream& os, const Location &obj);

#endif