/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTransaction.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:22:21 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/18 17:57:55 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CgiHandler.hpp"
#include "../includes/HttpTransaction.hpp"
#include <cerrno>

HttpTransaction::HttpTransaction(const VirtualServer *vir_server)
	: parse_chuncked_bytes_to_read(0), parse_is_last_chunck(false),
		vir_server(vir_server), location(NULL), state(PARSING_REQ_METHOD)
{
}

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
				if (this->request.add_method(temp) == -1)
					assign_state(PARSING_ERROR, raw.at(i));
				temp.clear();
			}
			else
				temp += raw.at(i);
			break ;
		case PARSING_REQ_URI:
			if (raw.at(i) == ' ')
			{
				this->state = PARSING_REQ_VERSION;
				this->request.uri = temp;
				temp.clear();
			}
			else
				temp += raw.at(i);
			break ;
		case PARSING_REQ_VERSION:
			if (raw.at(i) == '\r')
			{
				this->state = PARSING_REQ_LINE_CR;
				this->request.protocol = temp;
				temp.clear();
			}
			else
				temp += raw.at(i);
			break ;
		case PARSING_REQ_LINE_CR:
			if (raw.at(i) == '\n')
			{
				this->state = PARSING_HEADER_KEY;
				this->location = find_location();
				if (this->location == NULL)
					assign_state(PARSING_ERROR, raw.at(i));
			}
			else
				assign_state(PARSING_ERROR, raw.at(i));
			break ;
		case PARSING_HEADER_KEY:
			if (raw.at(i) == ':')
			{
				this->state = PARSING_HEADER_OWS;
			}
			else if (isspace(raw.at(i)))
				assign_state(PARSING_ERROR, raw.at(i));
			else
				temp_key += raw.at(i);
			break ;
		case PARSING_HEADER_OWS:
			if (raw.at(i) == ' ')
				continue ;
			else if (raw.at(i) == '\r' || raw.at(i) == '\n')
				assign_state(PARSING_ERROR, raw.at(i));
			else
			{
				this->state = PARSING_HEADER_VALUE;
				i--;
			}
			break ;
		case PARSING_HEADER_VALUE:
			if (raw.at(i) == '\r')
			{
				this->state = PARSING_HEADER_CR;
				this->request.headers.insert(std::make_pair(temp_key,
							temp_value));
				temp_value.clear();
				temp_key.clear();
			}
			else
				temp_value += raw.at(i);
			break ;
		case PARSING_HEADER_CR:
			if (raw.at(i) == '\n')
				this->state = PARSING_HEADER_DONE;
			else
				assign_state(PARSING_ERROR, raw.at(i));
			break ;
		case PARSING_HEADER_DONE:
			if (raw.at(i) == '\r')
				this->state = PARSING_HEADER_FINAL_CR;
			else if (isspace(raw.at(i)))
				assign_state(PARSING_ERROR, raw.at(i));
			else
			{
				this->state = PARSING_HEADER_KEY;
				i--;
			}
			break ;
		case PARSING_HEADER_FINAL_CR:
			if (raw.at(i) == '\n')
			{
				if (this->request.headers.count("Content-Length") == 1 &&
					static_cast<size_t>(
						atoi(this->request.headers.at("Content-Length").c_str())) >
						location->client_max_body_size)
				{
					this->state = ERROR_EXCEEDS_LIMIT;
					break ;
				}
				if (this->request.headers.count("Content-Length") == 1)
					this->state = PARSING_BODY;
				else if (this->request.headers.count("Transfer-Encoding") == 1
						&&
							this->request.headers.at("Transfer-Encoding") == "chunked")
				{
					this->state = PARSING_CHUNCKED_SIZE;
					std::cout << "In the chunked\n";
				}
				else
				{
					std::cout << "Exited as processing\n";
					this->state = PROCESSING;
				}
			}
			else
				assign_state(PARSING_ERROR, raw.at(i));
			break ;
		case PARSING_BODY:
			if (this->request.headers.count("Content-Length") == 0)
			{
				this->state = PROCESSING;
				break ;
			}
			else if (this->request.body.size() >=
						static_cast<size_t>(atoi(this->request.headers.at("Content-Length").c_str())))
			{
				assign_state(PARSING_ERROR, raw.at(i));
				break ;
			}
			this->request.body += raw.at(i);
			if (this->request.body.size() ==
				static_cast<size_t>(
					atoi(this->request.headers.at("Content-Length").c_str())))
				this->state = PROCESSING;
			break ;
		case PARSING_CHUNCKED_SIZE:
			if (raw.at(i) == '\r')
			{
				if (parse_chuncked_size_str.size() == 0)
					assign_state(PARSING_ERROR, raw.at(i));
				else
					this->state = PARSING_CHUNCKED_SIZE_CR;
				break ;
			}
			else if (raw.at(i) == ' ')
			{
				break ;
			}
			parse_chuncked_size_str += raw.at(i);
			break ;
		case PARSING_CHUNCKED_SIZE_CR:
			parse_chuncked_bytes_to_read =
				extract_hexa_to_int(parse_chuncked_size_str);
			parse_chuncked_size_str.clear();
			if (static_cast<size_t>(parse_chuncked_bytes_to_read) > location->client_max_body_size)
			{
				state = ERROR_EXCEEDS_LIMIT;
				return;
			}
			
			if (parse_chuncked_bytes_to_read == 0)
				parse_is_last_chunck = true;
			if (raw.at(i) == '\n' && parse_is_last_chunck == false)
				this->state = PARSING_CHUNCKED_BODY;
			else if (raw.at(i) == '\n' && parse_is_last_chunck == true)
				this->state = PARSING_CHUNCKED_DONE;
			else
			{
				assign_state(PARSING_ERROR, raw.at(i));
			}
			break ;
		case PARSING_CHUNCKED_BODY:
			if (location != NULL && request.body.size() >= location->client_max_body_size)
			{
				state = ERROR_EXCEEDS_LIMIT;
				return;
			}
			if (parse_chuncked_bytes_to_read > 0)
			{
				request.body += raw.at(i);
				parse_chuncked_bytes_to_read--;
			}
			else if (parse_chuncked_bytes_to_read == 0 && raw.at(i) == '\r')
				this->state = PARSING_CHUNCKED_CR;
			else
				assign_state(PARSING_ERROR, raw.at(i));
			break ;
		case PARSING_CHUNCKED_CR:
			if (raw.at(i) == '\n')
				this->state = PARSING_CHUNCKED_SIZE;
			else
				assign_state(PARSING_ERROR, raw.at(i));
			break ;
		case PARSING_CHUNCKED_DONE:
			if (request.body.size() > location->client_max_body_size)
			{
				this->state = ERROR_EXCEEDS_LIMIT;
				break ;
			}
			if (raw.at(i) == '\r')
				this->state = PARSING_CHUNCKED_FINAL_CR;
			else
				assign_state(PARSING_ERROR, raw.at(i));
			break ;
		case PARSING_CHUNCKED_FINAL_CR:
			if (raw.at(i) == '\n')
			{
				if (request.body.size() > location->client_max_body_size)
				{
					this->state = ERROR_EXCEEDS_LIMIT;
					break ;
				}
				if (parse_is_last_chunck == true)
				{
					this->state = PROCESSING;
				}
				else
					this->state = PARSING_CHUNCKED_SIZE;
			}
			else
				assign_state(PARSING_ERROR, raw.at(i));
			break ;
		default:
			break ;
		}
	}
}

const Location *HttpTransaction::find_location()
{
	std::string		uri_with_append;
	const Location	*matched_location;
	size_t			size_best_match;

	matched_location = NULL;
	size_best_match = 0;
	uri_with_append = this->request.uri + "/";
	for (size_t i = 0; i < vir_server->locations.size(); i++)
	{
		const std::string &loc_path = vir_server->locations.at(i).path;
		if (this->request.uri.find(loc_path) == 0)
		{
			if (loc_path.length() > size_best_match)
			{
				size_best_match = loc_path.length();
				matched_location = &vir_server->locations.at(i);
			}
		}
		if (uri_with_append.find(loc_path) == 0)
		{
			if (loc_path.length() > size_best_match)
			{
				size_best_match = loc_path.length();
				matched_location = &vir_server->locations.at(i);
				request.uri = uri_with_append;
			}
		}
	}
	return (matched_location);
}

bool	is_allowed_method(const Location *matched_location,
						const std::string &method)
{
	std::vector<std::string>::const_iterator it =
		matched_location->allow_methods.begin();
	std::vector<std::string>::const_iterator it_end =
		matched_location->allow_methods.end();
	std::vector<std::string>::const_iterator result =
		std::find(it, it_end, method);
	if (result == it_end)
		return (false);
	else
		return (true);
}

void HttpTransaction::process_request(int epollfd, int curr_socket)
{
	std::cout << "Inside process_request\n";
	std::cout << "This is the state: " << state << std::endl;
	if (state == PROCESSING)
	{
		// 1. Find Location
		if (location == NULL)
		{
			build_error_response(404);
			change_socket_epollout(epollfd, curr_socket);
			return ;
		}
		// 2. Validate Method
		if (is_allowed_method(location, request.method) == false)
		{
			build_error_response(405);
			change_socket_epollout(epollfd, curr_socket);
			return ;
		}
		// 3. Build the response. The potential options for response are:
		// 3.1. Return Redirections
		if (location->return_redir.first != 0)
		{
			response.add_header("Location", location->return_redir.second);
			response.build_response(location->return_redir.first);
			change_socket_epollout(epollfd, curr_socket);
			return ;
		}
		// 3.2. Alias to get files from different location
		else if (location->alias.size() != 0)
		{
			std::string file_name;
			std::size_t len_file_name;
			len_file_name =
				std::max(static_cast<std::size_t>(0),
							request.uri.size() - request.uri.find_last_of('/') - 1);
			if (len_file_name != 0)
				file_name = std::string(request.uri,request.uri.find_last_of('/') + 1, len_file_name);
			request.final_path = location->alias + file_name;
			prepare_response(epollfd, curr_socket);
		}
		else
		{
			std::string target_resource = request.uri;

			target_resource = target_resource.substr(location->path.length(), target_resource.length() - location->path.length());
			std::cout << "Location path: "<< location->path << std::endl;
			std::cout << "Target resource: "<< target_resource << std::endl;
			request.final_path = location->root + "/" + target_resource;
			std::cout << "This is the final_path: " << request.final_path << std::endl;
			if (!location->cgi_ext.empty() && ft_ends_with(request.final_path, location->cgi_ext) && request.method == "POST")
				request.final_path = build_cgi_path();
			prepare_response(epollfd, curr_socket);
		}
	}
	else if (state == PARSING_ERROR)
	{
		build_error_response(400);
		change_socket_epollout(epollfd, curr_socket);
	}
	else if (state == ERROR_METHOD_NOT_ALLOWED)
	{
		build_error_response(405);
		change_socket_epollout(epollfd, curr_socket);
	}
	else if (state == ERROR_EXCEEDS_LIMIT)
	{
		build_error_response(413);
		change_socket_epollout(epollfd, curr_socket);
	}
	else
	{
		build_error_response(500);
		change_socket_epollout(epollfd, curr_socket);		
	}
}

std::string HttpTransaction::build_cgi_path()
{
	printf("----- Is CGI -----\n");
	std::string file_name;
	std::string final_path;
	std::size_t len_file_name;
	len_file_name =
		std::max(static_cast<std::size_t>(0),
					request.uri.size() - request.uri.find_last_of('/') - 1);
	if (len_file_name != 0)
		file_name = std::string(request.uri,request.uri.find_last_of('/') + 1, len_file_name);
	final_path = location->cgi_script_root + "/" + file_name;
	printf("%s\n\n", final_path.c_str());
	return (final_path);	
}

void HttpTransaction::prepare_response(int epollfd, int curr_socket)
{
	struct stat	s;
	CgiHandler cgi;

	if (stat(request.final_path.c_str(), &s) == 0)
	{
		if(request.method == "GET")
			prepare_response_get(s);
		else if (request.method == "POST")
			prepare_response_post(curr_socket, s);
		else if (request.method == "DELETE") {}
			// To add
		else
			build_error_response(405);
	}
	else
		build_error_response(404);
	if (state != WAITING_CGI)
		change_socket_epollout(epollfd, curr_socket);
}

void HttpTransaction::prepare_response_get(struct stat &s)
{
	build_response_found_resource(location, s);
}

void HttpTransaction::prepare_response_post(int curr_socket, struct stat &s)
{
	CgiHandler cgi;

	if (location->cgi_ext.size() != 0 &&
		ft_ends_with(request.final_path, location->cgi_ext) == true)
	{
		// create temp file
		std::string temp_path = "./var/www/temp/webserv_upload_" + ft_int_to_string(getpid()) + "_" + ft_int_to_string(curr_socket)+ ".tmp";

		int body_fd = open(temp_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0600);
		if (body_fd == -1)
		{
			std::cout << "Problem in first if\n";
			build_error_response(500);
			return;
    	}

		// write request body into file
		ssize_t written_size = write(body_fd, request.body.data(), request.body.size());
		if (written_size != static_cast<ssize_t>(request.body.size()))
		{
			std::cout << "Problem in second if\n";
			unlink(temp_path.c_str());
			build_error_response(500);
			return;
		}
		close(body_fd);
		cgi_info.input_doc_path = temp_path;
		
		// execute cgi
		printf("This is final path: %s\n", request.final_path.c_str());
		printf("This is cgi_info before execute\npipe_fd: %d;\npid: %d;\nis_started: %i;\nbuffer: %s\n;input_doc_path: %s\n", cgi_info.pipe_fd, cgi_info.pid, cgi_info.is_started, cgi_info.buffer.c_str(), cgi_info.input_doc_path.c_str());
		cgi_info = cgi.execute(location->cgi_path, request.final_path,
				cgi_info.input_doc_path, *this, curr_socket);
		state = WAITING_CGI;
		printf("This is cgi_info after execute\npipe_fd: %d;\npid: %d;\nis_started: %i;\nbuffer: %s\n;input_doc_path: %s\n", cgi_info.pipe_fd, cgi_info.pid, cgi_info.is_started, cgi_info.buffer.c_str(), cgi_info.input_doc_path.c_str());
		if (cgi_info.is_started == false)
		{
			unlink(temp_path.c_str());
			build_error_response(500);
		}
	}
	else
		build_response_found_resource(location, s);
}


void HttpTransaction::build_response_found_resource(const Location *matched_location, struct stat &s)
{
	std::ifstream html_file;
	// If the request uri matches a directory
	if (S_ISDIR(s.st_mode))
	{
		std::cout << "Inside the directory section\n";
		if (request.final_path.at(request.final_path.length() - 1) != '/')
			request.final_path += "/";
		
		// check if location has indices to return
		for (size_t i = 0; i < matched_location->index.size(); i++)
		{
			std::string index_path = request.final_path + matched_location->index.at(i);
			std::cout << "This is index_path: " << index_path << std::endl;
			if (open_file(&index_path.at(0), html_file) == true)
			{
				response.set_body(file_to_string(html_file));
				response.build_response(200);
				return ;
			}
		}
		// given there were no indices, check if we can return autoindex
		if (matched_location->autoindex == true)
		{
			response.set_body(build_autoindex_string(request.final_path));
			if (response.get_body().size() == 0)
			{
				build_error_response(500);
				return ;
			}
			response.build_response(200);
		}
		// Without any available indices or autoindex, we need to return an error
		else
			build_error_response(404);
	}
	// If it is not a directory, it is a file
	else
	{
		if (open_file(&request.final_path.at(0), html_file) == true)
		{
			response.set_body(file_to_string(html_file));
			response.build_response(200);
		}
		else
			build_error_response(405);
	}
}

void HttpTransaction::build_error_response(int error_code)
{
	response.set_status(error_code);
	response.set_body(generate_error_page(error_code));
	response.serialize_response();
	state = SENDING;
}

std::string HttpTransaction::generate_error_page(int error_code)
{
	if (vir_server->error_page.count(error_code))
	{
		std::string file_path =
			"." + vir_server->root + vir_server->error_page.at(error_code);
		std::ifstream file_stream(file_path.c_str());
		if (file_stream.is_open())
			return std::string((std::istreambuf_iterator<char>(file_stream)),
								std::istreambuf_iterator<char>());
	}
	return ("<html><body><h1>" + ft_int_to_string(error_code) +
			" Error</h1></body></html>");
}

std::string HttpTransaction::build_autoindex_string(std::string &dir_path)
{
	DIR	*dir;

	dir = opendir(dir_path.c_str());
	std::string page = "<!DOCTYPE html>\n\
    <html>\n\
    <head>\n\
            <title>" +
		dir_path + "</title>\n\
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
		if (std::string(file->d_name) == "."
			|| std::string(file->d_name) == "..")
			continue ;
		page += "<li><a href=http://localhost:";
		page += ft_int_to_string(this->vir_server->listen);
		page += dir_path;
		page += "/";
		page += file->d_name;
		page += ">";
		page += file->d_name;
		page += "</a>";
	}
	page += "\
    </p>\n\
    </body>\n\
    </html>\n";
	return (page);
}

void HttpTransaction::mark_as_complete()
{
	this->state = COMPLETE;
}

void HttpTransaction::assign_state(State s, char c)
{
    std::cerr << "PARSING_ERROR in state " << this->state 
              << " at char '" << c << "' (0x" 
              << std::hex << (int)(unsigned char)c << ")" << std::endl;
    this->state = s;
}
