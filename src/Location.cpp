#include "../includes/Location.hpp"
#include "../includes/VirtualServer.hpp"
#include "../includes/ConfigError.hpp"
#include "../includes/utils.hpp"

// Constructors
Location::Location(const t_location &location_config, const VirtualServer &vir_serv):
	root(vir_serv.root),
	error_page(vir_serv.error_page),
	client_max_body_size(vir_serv.client_max_body_size),
	index(vir_serv.index),
	autoindex(vir_serv.autoindex)
{
	this->path = location_config.path;
	for (size_t i = 0; i < location_config.dir.size(); i++)
	{
		t_directive curr_directive = location_config.dir.at(i);

		if (curr_directive.name == "root")
		{
			if (curr_directive.args.size() != 1)
				throw ConfigError("root must only have one path assigned", curr_directive.name);
			this->root = curr_directive.args.at(0);
		}
		else if (curr_directive.name == "error_page")
		{
			int last_index = curr_directive.args.size() - 1;
			std::string error_page_path = curr_directive.args.at(last_index);
			for (size_t j = 0; j < curr_directive.args.size() - 1; j++)
			{
				int	error_num = extract_and_validate_str_to_int(curr_directive.args.at(j));
				this->error_page[error_num] = curr_directive.args.at(last_index);
			}
		}
		else if (curr_directive.name == "client_max_body_size")
		{
			if (curr_directive.args.size() != 1)
				throw ConfigError("client_max_body_size must only have one int assigned", curr_directive.name);
			this->client_max_body_size = extract_and_validate_str_to_int(curr_directive.args.at(0));
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
				throw ConfigError("autoindex needs to be either 'on' or 'off'", curr_directive.args.at(0));
			this->autoindex = autoindex_status;
		}
		else if (curr_directive.name == "alias")
		{
			if (curr_directive.args.size() != 1)
				throw ConfigError("alias must only have one path assigned", curr_directive.name);
			this->alias = curr_directive.args.at(0);
		}
		else if (curr_directive.name == "upload_store")
		{
			if (curr_directive.args.size() != 1)
				throw ConfigError("upload_store must only have one path assigned", curr_directive.name);
			this->upload_store = curr_directive.args.at(0);
		}
		else if (curr_directive.name == "cgi_ext")
		{
			this->cgi_ext = curr_directive.args;
		}
		else if (curr_directive.name == "cgi_path")
		{
			if (curr_directive.args.size() != 1)
				throw ConfigError("cgi_path must only have one path assigned", curr_directive.name);
			this->cgi_path = curr_directive.args.at(0);
		}
		else
		{
			throw ConfigError("directive not allowed in a location block", curr_directive.name);
		}
	}
}



// Destructor
Location::~Location(void) {}


std::ostream& operator<<(std::ostream& os, const Location& loc) {
	os << "====== Location Block ======\n";
	os << "Path: " << loc.path << "\n";
	os << "Root: " << loc.root << "\n";
	os << "Alias: " << loc.alias << "\n";
	os << "Autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
	os << "Client Max Body Size: " << loc.client_max_body_size << "\n";
	os << "Upload Store: " << loc.upload_store << "\n";
	os << "CGI Path: " << loc.cgi_path << "\n";

	os << "Index Files: ";
	for (std::vector<std::string>::const_iterator it = loc.index.begin(); it != loc.index.end(); ++it) {
		os << *it << " ";
	}
	os << "\n";

	os << "CGI Extensions: ";
	for (std::vector<std::string>::const_iterator it = loc.cgi_ext.begin(); it != loc.cgi_ext.end(); ++it) {
		os << *it << " ";
	}
	os << "\n";

	os << "Error Pages:\n";
	for (std::map<int, std::string>::const_iterator it = loc.error_page.begin(); it != loc.error_page.end(); ++it) {
		os << "  " << it->first << " -> " << it->second << "\n";
	}
	os << "==========================\n";
	return os;
}