
#include <string>
#include <vector>
#include <sstream>
#include "../includes/Lexer.hpp"
#include "../includes/Server.hpp"

Token token(std::string content, TokenType type)
{
	Token token;
	token.content = content;
	token.type = type;
	return (token);
}

bool	is_delim(std::string &config_str, int loc, std::vector<char> &delim)
{
	for (size_t i = 0; i < delim.size(); i++)
	{
		// std::cout << "*This is a delim* [" << delim[i] <<"]\n";

		if (config_str[loc] == delim[i])
		{
			// std::cout << "*This is a delim* [" << config_str[loc] <<"]\n";
			return true;

		}
	}
	return false;
}

bool	is_end_of_token(std::string &config_str, int curr_i, std::vector<char>& delim)
{
	if (!config_str[curr_i] || is_delim(config_str, curr_i, delim) || isspace(config_str[curr_i]))
		return true;
	return false;
}

Lexer::Lexer(std::string &config_str)
{
	char delim_array[] = {'{', '}', '\"', ';', '#'};
	std::vector<char> delim(delim_array, delim_array + sizeof(delim_array) / sizeof(char));
	for (size_t i = 0; i < delim.size(); i++)
	{
		std::cout << "This is delim ->" << delim[i] <<"\n";
	}
	
	std::string temp_str;

	for (int i = 0; config_str[i]; i++)
	{
		if (config_str[i] == '#')
		{
			i++;
			if (config_str[i] && config_str[i] != '\n')
				i++;
			else if(config_str[i] && config_str[i] == '\n')
				_token_list.push_back(token("\n", Whitespace));
		}
		else if (config_str[i] == '{')
			_token_list.push_back(token("{", OpenBrk));
		else if (config_str[i] == '}')
			_token_list.push_back(token("}", CloseBrk));
		else if (config_str[i] == ';')
			_token_list.push_back(token(";", SemiCol));
		else if (std::isspace(config_str[i]))
			_token_list.push_back(token(" ", Whitespace));
		else if (config_str[i] == '\"'){
			std::string temp_str;
			temp_str += "\"";
			i++;
			while (config_str[i] && config_str[i] == '\"')
			{
				temp_str += config_str[i];
				i++;
			}
			if (!config_str[i])
				throw std::exception();
			temp_str += "\"";
			_token_list.push_back(token(" ", ParamTok));
		}
		else
		{
			std::string temp_str;
			while (is_end_of_token(config_str, i, delim) == false)
			{
				temp_str += config_str[i];
				i++;
			}
			i--;
			if (std::find(Server::directives.begin(), Server::directives.end(), temp_str) != Server::directives.end())
				_token_list.push_back(token(temp_str, KeywordTok));
			else
				_token_list.push_back(token(temp_str, ParamTok));
		}
	}	
}

std::ostream& operator<<(std::ostream& os, const Lexer &obj)
{
	os <<"Beginning of the Lexer\n";
	for (std::list<Token>::const_iterator it = obj._token_list.begin(); it != obj._token_list.end(); it++)
	{
		// Debug
		os << "Content: " <<it->content << "; type: " << it->type << std::endl; 
		// os << it->content ; 
	}
	return(os);
}

Lexer::~Lexer() {}
