#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <list>
#include <iostream>

enum TokenType{
	KeywordTok,
	ParamTok,
	OpenBrk,
	CloseBrk,
	SemiCol,
	Whitespace
};

typedef struct Token
{
	std::string content;
	TokenType type;
} Token;


class Lexer
{
private:
public:
	std::list<Token> _token_list;

	Lexer(std::string &config_str);
	~Lexer();
};

std::ostream& operator<<(std::ostream& os, const Lexer &obj);

#endif