/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:19:05 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/16 12:19:08 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
# define LEXER_HPP

# include <iostream>
# include <list>
# include <string>

enum			TokenType
{
	ContextTok,
	DirectTok,
	ParamTok,
	OpenBrk,
	CloseBrk,
	SemiCol,
	Whitespace
};

typedef struct Token
{
	std::string content;
	TokenType	type;
}				Token;

class Lexer
{
  private:
  public:
	std::list<Token> _token_list;

	Lexer(std::string &config_str);
	~Lexer();
};

std::ostream &operator<<(std::ostream &os, const Lexer &obj);

#endif
