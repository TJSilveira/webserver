<<<<<<< Updated upstream
=======
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTransaction.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amoiseik <amoiseik@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:22:21 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/20 16:40:28 by amoiseik         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CgiHandler.hpp"
>>>>>>> Stashed changes
#include "../includes/HttpTransaction.hpp"
#include <cerrno>

HttpTransaction::HttpTransaction(const VirtualServer *vir_server):
			parse_chuncked_bytes_to_read(0),
			parse_is_last_chunck(false), 
			vir_server(vir_server),
			state(PARSING_REQ_METHOD) {}

HttpTransaction::~HttpTransaction()
{
}

void HttpTransaction::parse(const std::string &raw)
{
	for (size_t i = 0; i < raw.size(); i++)
	{
		switch (this->state)
		{
		case PARSING_REQ_METHOD:
			if (raw.at(i) == ' ')
			{
				this->state = PARSING_REQ_URI;
				this->request.add_method(temp);
				temp.clear();
			}
			else
				temp += raw.at(i);
			break;
		case PARSING_REQ_URI:
			if (raw.at(i) == ' ')
			{
				this->state = PARSING_REQ_VERSION;
				this->request.uri = temp;
				temp.clear();
			}
			else
				temp += raw.at(i);
			break;
		case PARSING_REQ_VERSION:
			if (raw.at(i) == '\r')
			{
				this->state = PARSING_REQ_LINE_CR;
				this->request.protocol = temp;
				temp.clear();
			}
			else
				temp += raw.at(i);
			break;
		case PARSING_REQ_LINE_CR:
			if (raw.at(i) == '\n')
				this->state = PARSING_HEADER_KEY;
			else
				this->state = PARSING_ERROR;
			break;
		case PARSING_HEADER_KEY:
			if (raw.at(i) == ':')
			{
				this->state = PARSING_HEADER_OWS;
			}
			else if(isspace(raw.at(i)))
				this->state = PARSING_ERROR;
			else
				temp_key += raw.at(i);
			break;
		case PARSING_HEADER_OWS:
			if (raw.at(i) == ' ')
				continue;
			else if(raw.at(i) == '\r' || raw.at(i) == '\n')
				this->state = PARSING_ERROR;
			else
			{
				this->state = PARSING_HEADER_VALUE;
				i--;
			}
			break;
		case PARSING_HEADER_VALUE:
			if (raw.at(i) == '\r')
			{
				this->state = PARSING_HEADER_CR;
				this->request.headers.insert(std::make_pair(temp_key, temp_value));
				temp_value.clear();
				temp_key.clear();
			}
			else
				temp_value += raw.at(i);
			break;
		case PARSING_HEADER_CR:
			if (raw.at(i) == '\n')
				this->state = PARSING_HEADER_DONE;
			else
				this->state = PARSING_ERROR;
			break;
		case PARSING_HEADER_DONE:
			if (raw.at(i) == '\r')
				this->state = PARSING_HEADER_FINAL_CR;
			else if(isspace(raw.at(i)))
				this->state = PARSING_ERROR;
			else
			{
				this->state = PARSING_HEADER_KEY;
				i--;
			}
			break;
		case PARSING_HEADER_FINAL_CR:
			if (raw.at(i) == '\n')
			{
				if(this->request.headers.count("Content-Length") == 1)
					this->state = PARSING_BODY;
				else if(this->request.headers.count("Transfer-Encoding") == 1 &&
						this->request.headers.at("Transfer-Encoding") == "chunked")
				{
					this->state = PARSING_CHUNCKED_SIZE;
					std::cout << "In the chunked\n";
				}
				else
					this->state = PROCESSING;
			}
			else
				this->state = PARSING_ERROR;
			break;
		case PARSING_BODY:
			if (this->request.headers.count("Content-Length") == 0)
			{
				this->state = PROCESSING;
				break;
			}
			else if (this->request.body.size() >= static_cast<size_t>(atoi(this->request.headers.at("Content-Length").c_str())))
			{
				this->state = PARSING_ERROR;
				break;
			}
			this->request.body += raw.at(i);
			if (this->request.body.size() == static_cast<size_t>(atoi(this->request.headers.at("Content-Length").c_str())))
				this->state = PROCESSING;
			break;
		case PARSING_CHUNCKED_SIZE:
			if (raw.at(i) == '\r')
			{
				if (parse_chuncked_size_str.size() == 0)
					this->state = PARSING_ERROR;
				else
					this->state = PARSING_CHUNCKED_SIZE_CR;
				break;
			}
			else if (raw.at(i) == ' ')
			{
				break;
			}
			parse_chuncked_size_str += raw.at(i);
			break;
		case PARSING_CHUNCKED_SIZE_CR:
			parse_chuncked_bytes_to_read = extract_hexa_to_int(parse_chuncked_size_str);
			std::cout <<"This is the size: " << parse_chuncked_bytes_to_read << "\n";
			if (parse_chuncked_bytes_to_read == 0)
				parse_is_last_chunck = true;
			
			if (raw.at(i) == '\n' && parse_is_last_chunck == false)
				this->state = PARSING_CHUNCKED_BODY;
			else if (raw.at(i) == '\n' && parse_is_last_chunck == true)
				this->state = PARSING_CHUNCKED_DONE;
			else
				this->state = PARSING_ERROR;
			break;
		case PARSING_CHUNCKED_BODY:
			if (parse_chuncked_bytes_to_read > 0)
			{
				request.body += raw.at(i);
				parse_chuncked_bytes_to_read--;
			}
			else if (parse_chuncked_bytes_to_read == 0 && raw.at(i) == '\r')
				this->state = PARSING_CHUNCKED_CR;
			else
				this->state = PARSING_ERROR;
			break;
		case PARSING_CHUNCKED_CR:
			if (raw.at(i) == '\n')
				this->state = PARSING_CHUNCKED_DONE;
			else
				this->state = PARSING_ERROR;
			break;
		case PARSING_CHUNCKED_DONE:
			if (raw.at(i) == '\r')
				this->state = PARSING_CHUNCKED_FINAL_CR;
			else
				this->state = PARSING_ERROR;
			break;
		case PARSING_CHUNCKED_FINAL_CR:
			if (raw.at(i) == '\n')
			{
				std::cout << "Inside the final PARSING_CHUNCKED_FINAL_CR\n";
				if (parse_is_last_chunck == true)
				{
					this->state = PROCESSING;
					std::cout << "This is the state of body at the end of the chuncked parsing" << request.body << std::endl;
				}
				else
					this->state = PARSING_CHUNCKED_SIZE;
			}
			else
				this->state = PARSING_ERROR;
			break;
		default:
			break;
		}
	}
}

const Location *HttpTransaction::find_location()
{
	const Location *matched_location = NULL;
	size_t size_best_match = 0;

	for (size_t i = 0; i < vir_server->locations.size(); i++)
	{
		const std::string& loc_path = vir_server->locations.at(i).path;
		
		if (this->request.uri.find(loc_path) == 0)
		{
			if (loc_path.length() > size_best_match)
			{
				size_best_match = loc_path.length();
				matched_location = &vir_server->locations.at(i);
			}
		}
	}
	return (matched_location);
}

bool is_allowed_method(const Location *matched_location, const std::string& method)
{
	std::vector<std::string>::const_iterator it = matched_location->allow_methods.begin();
	std::vector<std::string>::const_iterator it_end = matched_location->allow_methods.end();

	std::vector<std::string>::const_iterator result = std::find(it, it_end, method);
	
	if (result == it_end)
		return (false);
	else
		return (true);	
}

void	HttpTransaction::process_request(int epollfd, int curr_socket)
{
	std::cout << "Inside process_request\n";
	if (state == PROCESSING)
	{
		// 1. Find Location
		const Location *matched_location = find_location();
		if (matched_location == NULL)
		{
			build_error_reponse(404);
			change_socket_epollout(epollfd, curr_socket);
			return;
		}
		// 2. Validate Method
		if (is_allowed_method(matched_location, request.method) == false)
		{
<<<<<<< Updated upstream
			build_error_reponse(405);
=======
			response.add_header("Location", location->return_redir.second);
			response.set_head_method(request.method == "HEAD"); //added for seting flag is_head_method
			response.build_response(location->return_redir.first);
>>>>>>> Stashed changes
			change_socket_epollout(epollfd, curr_socket);
			return;
		}
		// 3. Resolve Path
		std::string		final_path;
		struct stat		s;
		final_path = "." + matched_location->root + request.uri;
		std::cout << "This is the final_path: " << final_path << std::endl;

		if (stat(final_path.c_str(), &s) == 0)
			build_response_found_resource(matched_location, s, final_path);
		else
			build_error_reponse(404);
		change_socket_epollout(epollfd, curr_socket);
	}
	else if (state == PARSING_ERROR)
	{
		std::cout << "Inside Parsing Error\n";
		build_error_reponse(400);
		change_socket_epollout(epollfd, curr_socket);
	}
}

void	HttpTransaction::build_response_found_resource(const Location* matched_location, struct stat &s, std::string &final_path)
{
	std::ifstream	html_file;

	// If the request uri matches a directory
	if (S_ISDIR(s.st_mode))
	{
		// check if location has indices to return
		for (size_t i = 0; i < matched_location->index.size(); i++)
		{
			std::string	index_path = final_path + matched_location->index.at(i);
			if (open_file(&index_path.at(0), html_file) == true)
			{
				response.set_body(file_to_string(html_file));
				response.set_head_method(request.method == "HEAD"); //added for seting flag is_head_method
				response.build_response(200);
				return;
			}
		}
		// given there were no indices, check if we can return autoindex
		if (matched_location->autoindex == true)
		{
			response.set_body(build_autoindex_string(final_path));
			if (response.get_body().size() == 0)
			{
				build_error_reponse(500);
				return;
			}
			response.build_response(200);
		}
		// Without any available indices or autoindex, we need to return an error
		else
			build_error_reponse(403);
	}
	// If it is not a directory, it is a file
	else
	{
		if (open_file(&final_path.at(0), html_file) == true)
		{
			response.set_body(file_to_string(html_file));
			response.set_head_method(request.method == "HEAD"); //added for seting flag is_head_method
			response.build_response(200);
		}
		else
			build_error_reponse(405);
	}
}

void HttpTransaction::build_error_reponse(int error_code)
{
	response.set_status(error_code);
	response.set_body(generate_error_page(error_code));
	response.set_head_method(request.method == "HEAD"); //added for seting flag is_head_method
	response.serialize_response();
	state = SENDING;
}

std::string	HttpTransaction::generate_error_page(int error_code)
{
	if (vir_server->error_page.count(error_code))
	{
		std::string		file_path = "." + vir_server->root + vir_server->error_page.at(error_code);
		std::ifstream	file_stream(file_path.c_str());

		if (file_stream.is_open())
			return std::string((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
	}
	return ("<html><body><h1>" + ft_int_to_string(error_code) + " Error</h1></body></html>");
}

std::string	HttpTransaction::build_autoindex_string(std::string &dir_path)
{
	DIR *dir = opendir(dir_path.c_str());
    std::string page =\
    "<!DOCTYPE html>\n\
    <html>\n\
    <head>\n\
            <title>" + dir_path + "</title>\n\
    </head>\n\
    <body>\n\
    <h1>INDEX</h1>\n\
    <p>\n";

	if (dir == NULL)
	{
		std::cerr << "Error opening directory in autoindex function\n";
		return ("");
	}

	for (struct dirent *file = readdir(dir); file; file = readdir(dir))
	{
		if (std::string(file->d_name) == "." || std::string(file->d_name) == "..")
			continue;
		page += "<li><a href=http://localhost:";
		page += ft_int_to_string(this->vir_server->listen);
		page += dir_path;
		page += "/";
		page += file->d_name;
		page += ">";
		page += file->d_name;
		page += "</a>";
	}

	page +="\
    </p>\n\
    </body>\n\
    </html>\n";
	return (page);
}

void HttpTransaction::mark_as_complete()
{
	this->state = COMPLETE;
}
