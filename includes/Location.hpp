/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:19:18 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/18 11:34:34 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "../includes/Parser.hpp"
# include <iostream>
# include <list>
# include <map>
# include <string>

class	VirtualServer;

class Location
{
private:
public:
	Location(const t_location &location_config, const VirtualServer &vir_serv);
	~Location();

	std::string path;							// location path
	std::string root;							// root folder
	std::map<int, std::string> error_page;		// Error pages
	size_t client_max_body_size;				// max size of content sent by the client
	std::vector<std::string> index;				// default page to be sent
	bool autoindex;								// default page to be sent
	std::string	alias;							// Replaces the matched location prefix with a specified path
	std::pair<int, std::string> return_redir;	// Redirects to a different page
	std::string upload_store;					/* Specifies the directory where uploaded files
												should be stored when using the PUT method.*/
	std::string cgi_script_root;				// where the cgi scripts are stored
	std::string cgi_ext;						// What cgi extensions to expect
	std::string cgi_path;						// where the cgi binaries are stored
	std::vector<std::string> allow_methods;		// What methods are allowed to be used
};

std::ostream &operator<<(std::ostream &os, const Location &obj);

#endif
