#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <list>
#include <vector>
#include <map>
#include <iostream>

#include "../includes/Lexer.hpp"

typedef struct s_directive
{
	std::string name;
	std::vector<std::string> args;
}	t_directive;

typedef struct s_location{
	std::string path;
	std::vector<t_directive> dir;
}	t_location;

typedef struct s_virtual_server{
	std::vector<t_directive> dir;
	std::vector<t_location> locations;
}	t_virtual_server;

typedef struct s_server{
	std::vector<t_directive> dir;
	std::vector<t_virtual_server> vir_servers;
}	t_server;

class Parser
{
private:
public:
	t_server	server;

	Parser(Lexer &lex);
	
	// Utils
	template<typename ExceptionType>
	Token& expect_and_advance(std::list<Token>::iterator &it, std::list<Token>::const_iterator &end, TokenType expected_type);

	void	parse_directive(std::vector<t_directive> &vector, std::list<Token>::iterator &it, std::list<Token>::const_iterator &end);
	void	parse_location(t_virtual_server &vir_serv, std::list<Token>::iterator &it, std::list<Token>::const_iterator &end);
	void	parse_virtual_server(t_server &serv, std::list<Token>::iterator &it, std::list<Token>::const_iterator &end);
	void	parse_server(std::list<Token>::iterator &it, std::list<Token>::const_iterator &end);
	~Parser();

	class InvalidLocation: public std::exception
	{
		virtual const char* what() const throw();
	};
	class InvalidVirtualServer: public std::exception
	{
		virtual const char* what() const throw();
	};
	class InvalidServer: public std::exception
	{
		virtual const char* what() const throw();
	};

};

std::ostream& operator<<(std::ostream& os, const Parser &obj);

#endif
