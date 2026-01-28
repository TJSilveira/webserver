#ifndef CONFIGERROR_HPP
#define CONFIGERROR_HPP

#include <stdexcept>
#include <string>
#include <sstream>

class ConfigError : public std::exception
{
	std::string msg;
public:
	ConfigError(const std::string &message, const std::string problematic_token)
	{
		std::stringstream ss;

		ss << "Config Error: " << message << ". Problematic token: '" << problematic_token << "'\n";
		msg = ss.str();
	}
	
	virtual const char* what() const throw()
	{
		return (msg.c_str());
	}

	virtual ~ConfigError() throw() {};
};


#endif