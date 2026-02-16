/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:23:39 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/16 12:23:42 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Parser.hpp"

template <typename ExceptionType>
Token &Parser::expect_and_advance(std::list<Token>::iterator &it,
									std::list<Token>::const_iterator &end,
									TokenType expected_type)
{
	if (it == end || it->type != expected_type)
		throw ExceptionType();
	Token &return_token = *it;
	it++;
	return (return_token);
}

void Parser::parse_directive(std::vector<t_directive> &vector,
								std::list<Token>::iterator &it,
								std::list<Token>::const_iterator &end)
{
	t_directive	temp_dir;

	temp_dir.name =
		expect_and_advance<InvalidLocation>(it, end, DirectTok).content;
	while (it != end && it->type == ParamTok)
	{
		temp_dir.args.push_back(it->content);
		it++;
	}
	expect_and_advance<InvalidLocation>(it, end, SemiCol);
	vector.push_back(temp_dir);
}

void Parser::parse_location(t_virtual_server &vir_serv,
							std::list<Token>::iterator &it,
							std::list<Token>::const_iterator &end)
{
	t_location	temp_loc;

	if (it->content.compare("location") != 0 || it->type != ContextTok)
	{
		throw InvalidLocation();
	}
	it++;
	temp_loc.path =
		expect_and_advance<InvalidLocation>(it, end, ParamTok).content;
	expect_and_advance<InvalidLocation>(it, end, OpenBrk);
	while (it != end && it->type != CloseBrk)
	{
		parse_directive(temp_loc.dir, it, end);
	}
	if (it->type != CloseBrk)
		throw InvalidLocation();
	it++;
	vir_serv.locations.push_back(temp_loc);
}

void Parser::parse_virtual_server(t_server &server,
									std::list<Token>::iterator &it,
									std::list<Token>::const_iterator &end)
{
	t_virtual_server	temp_virtual_server;

	if (it->content.compare("server") != 0 || it->type != ContextTok)
	{
		throw InvalidVirtualServer();
	}
	it++;
	expect_and_advance<InvalidVirtualServer>(it, end, OpenBrk);
	while (it != end && it->type != CloseBrk)
	{
		// Either add Location;
		if (it->content.compare("location") == 0 && it->type == ContextTok)
		{
			parse_location(temp_virtual_server, it, end);
			continue ;
		}
		// Or add Directive;
		parse_directive(temp_virtual_server.dir, it, end);
	}
	if (it == end)
		throw InvalidVirtualServer();
	it++;
	server.vir_servers.push_back(temp_virtual_server);
}

void Parser::parse_server(std::list<Token>::iterator &it,
							std::list<Token>::const_iterator &end)
{
	if (it->content.compare("http") != 0 || it->type != ContextTok)
	{
		throw InvalidServer();
	}
	it++;
	expect_and_advance<InvalidServer>(it, end, OpenBrk);
	while (it != end && it->type != CloseBrk)
	{
		// Either add Virtual Server;
		if (it->content.compare("server") == 0 && it->type == ContextTok)
		{
			parse_virtual_server(this->server, it, end);
			continue ;
		}
		// Or add Directive;
		parse_directive(this->server.dir, it, end);
	}
	if (it->type != CloseBrk)
		throw InvalidServer();
	it++;
	if (it != end)
		throw InvalidServer();
}

Parser::Parser(Lexer &lex)
{
	std::list<Token>::iterator iterator = lex._token_list.begin();
	std::list<Token>::const_iterator end = lex._token_list.end();
	parse_server(iterator, end);
}

Parser::~Parser()
{
}

std::ostream &operator<<(std::ostream &os, t_directive dir)
{
	os << "Name: " << dir.name << "; Args: ";
	for (size_t i = 0; i < dir.args.size(); i++)
	{
		os << dir.args[i] << " ";
	}
	os << ";";
	return (os);
}

std::ostream &operator<<(std::ostream &os, t_location location)
{
	os << "Location Path: " << location.path << "\n";
	for (size_t i = 0; i < location.dir.size(); i++)
	{
		os << "[Location Directive]: " << location.dir[i] << "\n";
	}
	return (os);
}

std::ostream &operator<<(std::ostream &os, t_virtual_server virtual_server)
{
	for (size_t i = 0; i < virtual_server.dir.size(); i++)
	{
		os << "[Virtual Server Directive]: " << virtual_server.dir[i] << "\n";
	}
	for (size_t i = 0; i < virtual_server.locations.size(); i++)
	{
		os << "=== [Location " << i << "] ===\n\n";
		os << virtual_server.locations[i];
	}
	return (os);
}

std::ostream &operator<<(std::ostream &os, const Parser &obj)
{
	for (size_t i = 0; i < obj.server.dir.size(); i++)
	{
		os << "[Server Directive]: " << obj.server.dir[i] << "\n";
	}
	for (size_t i = 0; i < obj.server.vir_servers.size(); i++)
	{
		os << "=== [VIRTUAL " << i << " SERVER] ===\n\n";
		os << obj.server.vir_servers[i];
	}
	return (os);
}

// Exceptions

const char *Parser::InvalidLocation::what() const throw()
{
	return ("Location block is invalid");
}

const char *Parser::InvalidVirtualServer::what() const throw()
{
	return ("Virtual Server block is invalid");
}

const char *Parser::InvalidServer::what() const throw()
{
	return ("Server block is invalid");
}
