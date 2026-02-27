/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amoiseik <amoiseik@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:23:58 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/27 12:19:02 by amoiseik         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/VirtualServer.hpp"
#include "../includes/utils.hpp"

// Constructors
VirtualServer::VirtualServer(const t_virtual_server &vir_serv_config,
								const Server &server)
	: root(server.root), error_page(server.error_page),
		client_max_body_size(server.client_max_body_size), index(server.index),
		autoindex(server.autoindex)
{
	t_directive	curr_directive;
	int			last_index;
	int			error_num;
	bool	autoindex_status;

	bool is_listen_set = false;
	bool is_server_name_set = false;
	bool is_root_set = false;
	bool is_body_size_set = false;
	bool is_autoindex_set = false;
	bool is_index_set = false;
	bool is_allow_methods_set = false;

	for (size_t i = 0; i < vir_serv_config.dir.size(); i++)
	{
		curr_directive = vir_serv_config.dir.at(i);
		if (curr_directive.name == "server_name")
		{
			if (is_server_name_set)
				throw ConfigError("duplicate directive", curr_directive.name);
			if (curr_directive.args.size() != 1)
				throw ConfigError("server_name must only have one parameter",
									curr_directive.name);
			this->server_name = curr_directive.args.at(0);
			is_server_name_set = true;
		}
		else if (curr_directive.name == "root")
		{
			if (is_root_set)
				throw ConfigError("duplicate directive", curr_directive.name);
			if (curr_directive.args.size() != 1)
				throw ConfigError("root must only have one parameter", curr_directive.name);
			this->root = curr_directive.args.at(0);
			is_root_set = true;
		}
		else if (curr_directive.name == "listen")
		{
			if (is_listen_set)
				throw ConfigError("duplicate directive", curr_directive.name);
			if (curr_directive.args.size() != 1)
				throw ConfigError("listen must only have one path assigned",
									curr_directive.name);
			int valid_int =
				extract_and_validate_str_to_int(curr_directive.args.at(0));
			if (valid_int < 1 || valid_int > 65535)
				throw ConfigError("port out of range [1-65535]", curr_directive.args.at(0));
			this->listen = valid_int;
			is_listen_set = true;
		}
		else if (curr_directive.name == "error_page")
		{
			if (curr_directive.args.size() < 2)
				throw ConfigError("error_page needs at least a code and a path", curr_directive.name);

			last_index = curr_directive.args.size() - 1;
			std::string error_page_path = curr_directive.args.at(last_index);
			for (size_t j = 0; j < (size_t)last_index; j++)
			{
				error_num = extract_and_validate_str_to_int(curr_directive.args.at(j));
				if (error_num < 300 || error_num > 599)
					throw ConfigError("invalid error code", curr_directive.args.at(j));
				this->error_page[error_num] = error_page_path;
			}
		}
		else if (curr_directive.name == "client_max_body_size")
		{
			if (is_body_size_set)
				throw ConfigError("duplicate directive", curr_directive.name);
			if (curr_directive.args.size() != 1)
				throw ConfigError("client_max_body_size must have one parameter", curr_directive.name);

			std::string arg = curr_directive.args.at(0);
			long long size = extract_size_to_bytes(arg);
			if (size < 0)
				throw ConfigError("client_max_body_size has invalid format", arg);
			this->client_max_body_size = static_cast<size_t>(size);

			is_body_size_set = true;
		}
		else if (curr_directive.name == "index")
		{
			if (is_index_set)
				throw ConfigError("duplicate directive", curr_directive.name);
			this->index = curr_directive.args;
			is_index_set = true;
		}
		else if (curr_directive.name == "autoindex")
		{
			if (is_autoindex_set)
				throw ConfigError("duplicate directive", curr_directive.name);

			if (curr_directive.args.size() == 1
				&& curr_directive.args.at(0) == "on")
				autoindex_status = true;
			else if (curr_directive.args.size() == 1 &&
						curr_directive.args.at(0) == "off")
				autoindex_status = false;
			else
			{
				throw ConfigError("autoindex needs to be either 'on' or 'off'",
									curr_directive.args.at(0));
			}
			this->autoindex = autoindex_status;
			is_autoindex_set = true;
		}
		else if (curr_directive.name == "allow_methods")
		{
			if (is_allow_methods_set)
				throw ConfigError("duplicate directive", curr_directive.name);

			this->allow_methods = curr_directive.args;
			is_allow_methods_set = true;
		}
		else
		{
			throw ConfigError("directive not allowed in a virtual_server block",
								curr_directive.name);
		}
	}
	if (this->allow_methods.empty())
	{
		this->allow_methods.push_back("GET");
	}
	for (size_t i = 0; i < vir_serv_config.locations.size(); i++)
	{
		std::string new_path = vir_serv_config.locations.at(i).path;

		// Check if new_path already in our Location vector
		for (size_t j = 0; j < this->locations.size(); j++)
		{
			if (this->locations.at(j).path == new_path)
			{
				throw ConfigError("duplicate location path found", new_path);
			}
		}
		this->locations.push_back(Location(vir_serv_config.locations.at(i),
					*this));
	}
}

// Destructor
VirtualServer::~VirtualServer(void){}

// Operator overload
std::ostream &operator<<(std::ostream &os, const VirtualServer &vs)
{
	os << "######## Virtual Server ########\n";
	os << "Server Name: " << vs.server_name << "\n";
	os << "Listen Port: " << vs.listen << "\n";
	os << "Root: " << vs.root << "\n";
	os << "Autoindex: " << (vs.autoindex ? "on" : "off") << "\n";
	os << "Client Max Body Size: " << vs.client_max_body_size << "\n";
	os << "Index Files: ";
	for (std::vector<std::string>::const_iterator it = vs.index.begin();
			it != vs.index.end();
			++it)
	{
		os << *it << " ";
	}
	os << "\n";
	os << "Error Pages:\n";
	for (std::map<int, std::string>::const_iterator it = vs.error_page.begin();
			it != vs.error_page.end();
			++it)
	{
		os << "  " << it->first << " -> " << it->second << "\n";
	}
	os << "--- Locations ---\n";
	for (std::vector<Location>::const_iterator it = vs.locations.begin();
			it != vs.locations.end();
			++it)
	{
		os << *it << "\n";
	}
	os << "################################\n";
	return (os);
}
