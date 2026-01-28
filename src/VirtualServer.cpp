#include "../includes/VirtualServer.hpp"
#include "../includes/Server.hpp"
#include "../includes/utils.hpp"

// Constructors
VirtualServer::VirtualServer(const t_virtual_server &vir_serv_config, const Server &server):
	root(server.root),
	error_page(server.error_page),
	client_max_body_size(server.client_max_body_size),
	index(server.index),
	autoindex(server.autoindex)
{
	for (size_t i = 0; i < vir_serv_config.dir.size(); i++)
	{
		t_directive curr_directive = vir_serv_config.dir.at(i);
		if (curr_directive.name == "server_name")
		{
			if (curr_directive.args.size() != 1)
				throw ConfigError("server_name must only have one parameter", curr_directive.name);
			this->server_name = curr_directive.args.at(0);
		}
		else if (curr_directive.name == "root")
		{
			this->root = curr_directive.args.at(0);
		}
		else if(curr_directive.name == "listen")
		{
			if (curr_directive.args.size() != 1)
				throw ConfigError("listen must only have one path assigned", curr_directive.name);
			int valid_int = extract_and_validate_str_to_int(curr_directive.args.at(0));
			this->listen = valid_int;
		}
		else if (curr_directive.name == "error_page")
		{
			int last_index = curr_directive.args.size() - 1;
			std::string error_page_path = curr_directive.args.at(last_index);
			for (size_t j = 0; j < curr_directive.args.size() - 1; j++)
			{
				int	error_num = atoi(curr_directive.args.at(j).c_str());
				this->error_page[error_num] = curr_directive.args.at(last_index);
			}
		}
		else if (curr_directive.name == "client_max_body_size")
		{
			this->client_max_body_size = atoi(curr_directive.args.at(0).c_str());
		}
		else if (curr_directive.name == "index")
		{
			this->index = curr_directive.args;
		}
	
		else if (curr_directive.name == "autoindex")
		{
			bool autoindex_status;
			if (curr_directive.args.size() == 1 && curr_directive.args.at(0) == "on")
				autoindex_status = true;
			else if (curr_directive.args.size() == 1 && curr_directive.args.at(0) == "off")
				autoindex_status = false;
			else
			{
				throw ConfigError("autoindex needs to be either 'on' or 'off'", curr_directive.args.at(0));
			}
			this->autoindex = autoindex_status;
		}
		else
		{
			throw ConfigError("directive not allowed in a virtual_server block", curr_directive.name);
		}
	}
	for (size_t i = 0; i < vir_serv_config.locations.size(); i++)
	{
		this->locations.push_back(Location(vir_serv_config.locations.at(i), *this));
	}
	
}

// Destructor
VirtualServer::~VirtualServer(void) {}


std::ostream& operator<<(std::ostream& os, const VirtualServer& vs) {
    os << "######## Virtual Server ########\n";
    os << "Server Name: " << vs.server_name << "\n";
    os << "Listen Port: " << vs.listen << "\n";
    os << "Root: " << vs.root << "\n";
    os << "Autoindex: " << (vs.autoindex ? "on" : "off") << "\n";
    os << "Client Max Body Size: " << vs.client_max_body_size << "\n";

    os << "Index Files: ";
    for (std::vector<std::string>::const_iterator it = vs.index.begin(); it != vs.index.end(); ++it) {
        os << *it << " ";
    }
    os << "\n";

    os << "Error Pages:\n";
    for (std::map<int, std::string>::const_iterator it = vs.error_page.begin(); it != vs.error_page.end(); ++it) {
        os << "  " << it->first << " -> " << it->second << "\n";
    }

    os << "--- Locations ---\n";
    for (std::vector<Location>::const_iterator it = vs.locations.begin(); it != vs.locations.end(); ++it) {
        os << *it << "\n";
    }
    os << "################################\n";
    return os;
}
