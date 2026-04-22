/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTransaction.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amoiseik <amoiseik@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:22:21 by tsilveir          #+#    #+#             */
/*   Updated: 2026/03/02 17:48:50 by amoiseik         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CgiHandler.hpp"
#include "../includes/HttpTransaction.hpp"
#include <cerrno>
#include <deque>
#include <vector>

HttpTransaction::HttpTransaction(const VirtualServer *vir_server)
	: parse_chuncked_bytes_to_read(0), parse_is_last_chunck(false), is_directory(false), 
		is_cgi(false), vir_server(vir_server), location(NULL), state(PARSING_REQ_METHOD)
{
}

HttpTransaction::~HttpTransaction()
{
}

void HttpTransaction::parse(const std::string &raw, int socketfd)
{	
	for (size_t i = 0; i < raw.size(); i++)
	{
		switch (this->state)
		{
		case PARSING_REQ_METHOD:
			if (raw.at(i) == ' ')
			{
				assign_state(PARSING_REQ_URI);
				if (this->request.add_method(temp) == -1)
					assign_state(ERROR_PARSING);
				temp.clear();
			}
			else
				temp += raw.at(i);
			break ;
		case PARSING_REQ_URI:
			if (raw.at(i) == ' ')
			{
				assign_state(PARSING_REQ_VERSION);
				this->request.uri = temp;
				temp.clear();
			}
			else
				temp += raw.at(i);
			break ;
		case PARSING_REQ_VERSION:
			if (raw.at(i) == '\r')
			{
				assign_state(PARSING_REQ_LINE_CR);
				this->request.protocol = temp;
				temp.clear();
			}
			else
				temp += raw.at(i);
			break ;
		case PARSING_REQ_LINE_CR:
			if (raw.at(i) == '\n')
			{
				assign_state(PARSING_HEADER_KEY);
				this->location = find_location();
				// if (this->location == NULL)
				// 	assign_state(ERROR_PARSING);
			}
			else
				assign_state(ERROR_PARSING);
			break ;
		case PARSING_HEADER_KEY:
			if (raw.at(i) == ':')
			{
				assign_state(PARSING_HEADER_OWS);
			}
			else if (isspace(raw.at(i)))
				assign_state(ERROR_PARSING);
			else
				temp_key += raw.at(i);
			break ;
		case PARSING_HEADER_OWS:
			if (raw.at(i) == ' ')
				continue ;
			else if (raw.at(i) == '\r' || raw.at(i) == '\n')
				assign_state(ERROR_PARSING);
			else
			{
				assign_state(PARSING_HEADER_VALUE);
				i--;
			}
			break ;
		case PARSING_HEADER_VALUE:
			if (raw.at(i) == '\r')
			{
				assign_state(PARSING_HEADER_CR);
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
				assign_state(PARSING_HEADER_DONE);
			else
				assign_state(ERROR_PARSING);
			break ;
		case PARSING_HEADER_DONE:
			if (raw.at(i) == '\r')
				assign_state(PARSING_HEADER_FINAL_CR);
			else if (isspace(raw.at(i)))
				assign_state(ERROR_PARSING);
			else
			{
				assign_state(PARSING_HEADER_KEY);
				i--;
			}
			break ;
		case PARSING_HEADER_FINAL_CR:
			if (raw.at(i) == '\n')
			{
				if (this->request.headers.count("Content-Length") == 1) {
					if (!request.open_body_stream(getpid(), socketfd))
						assign_state(ERROR_PARSING);
					else
						assign_state(PARSING_BODY);
				}
				else if (this->request.headers.count("Transfer-Encoding") == 1 &&
							this->request.headers.at("Transfer-Encoding") == "chunked")
				{
					if (!request.open_body_stream(getpid(), socketfd))
						assign_state(ERROR_PARSING);
					else
						assign_state(PARSING_CHUNKED_SIZE);
				}
				else
				{
					assign_state(PROCESSING);
				}
			}
			else
				assign_state(ERROR_PARSING);
			break ;
		case PARSING_BODY:
		{
			if (this->request.headers.count("Content-Length") == 0)
			{
				assign_state(PROCESSING);
				break ;
			}

			size_t content_length = static_cast<size_t>(atoi(this->request.headers.at("Content-Length").c_str()));

			size_t missing_bytes = content_length - this->request.body_bytes_read;
			size_t bytes_available = raw.size() - i;
			size_t bytes_to_append = std::min(missing_bytes, bytes_available);

			if (!this->request.write_to_body_stream(raw.data() + i, bytes_to_append))
			{
				assign_state(ERROR_DISK_WRITTING);
				break;
			}
			i += bytes_to_append - 1;
			
			if(request.body_bytes_read > location->client_max_body_size)
			{
				assign_state(ERROR_EXCEEDS_LIMIT);
				break;
			}
			else if (this->request.body_bytes_read == content_length)
			{
				this->request.close_body_stream();
				assign_state(PROCESSING);
			}
			else if (this->request.body_bytes_read >= content_length)
				assign_state(ERROR_PARSING);
			break ;
		}
		case PARSING_CHUNKED_SIZE:
			if (raw.at(i) == '\r')
			{
				if (parse_chuncked_size_str.size() == 0)
					assign_state(ERROR_PARSING);
				else
					assign_state(PARSING_CHUNKED_SIZE_CR);
				break ;
			}
			else if (raw.at(i) == ' ')
			{
				break ;
			}
			parse_chuncked_size_str += raw.at(i);
			break ;
		case PARSING_CHUNKED_SIZE_CR:
			parse_chuncked_bytes_to_read =
				extract_hexa_to_int(parse_chuncked_size_str);
			parse_chuncked_size_str.clear();
			if (static_cast<size_t>(parse_chuncked_bytes_to_read) > location->client_max_body_size)
			{
				assign_state(ERROR_EXCEEDS_LIMIT);
				return;
			}
			
			if (parse_chuncked_bytes_to_read == 0)
				parse_is_last_chunck = true;
			if (raw.at(i) == '\n' && parse_is_last_chunck == false)
				assign_state(PARSING_CHUNKED_BODY);
			else if (raw.at(i) == '\n' && parse_is_last_chunck == true)
				assign_state(PARSING_CHUNKED_DONE);
			else
			{
				assign_state(ERROR_PARSING);
			}
			break ;
		case PARSING_CHUNKED_BODY:
		{
			if (location && request.body_bytes_read >= location->client_max_body_size && parse_chuncked_bytes_to_read > 0)
			{
				assign_state(ERROR_EXCEEDS_LIMIT);
				return;
			}
			if (parse_chuncked_bytes_to_read > 0)
			{
				size_t bytes_available = raw.size() - i;
				size_t bytes_to_append = std::min(static_cast<size_t>(parse_chuncked_bytes_to_read), bytes_available);

				if (!this->request.write_to_body_stream(raw.data() + i, bytes_to_append))
				{
					assign_state(ERROR_DISK_WRITTING);
					break;
				}

				parse_chuncked_bytes_to_read -= bytes_to_append;

				i += (bytes_to_append - 1);
				if(request.body_bytes_read > location->client_max_body_size)
				{	
					assign_state(ERROR_EXCEEDS_LIMIT);
					break;
				}
			}
			else if (parse_chuncked_bytes_to_read == 0 && raw.at(i) == '\r')
				assign_state(PARSING_CHUNKED_CR);
			else
				assign_state(ERROR_PARSING);
			break ;
		}
		case PARSING_CHUNKED_CR:
			if (raw.at(i) == '\n')
				assign_state(PARSING_CHUNKED_SIZE);
			else
				assign_state(ERROR_PARSING);
			break ;
		case PARSING_CHUNKED_DONE:
			if (request.body_bytes_read > location->client_max_body_size)
			{
				assign_state(ERROR_EXCEEDS_LIMIT);
				break ;
			}
			if (raw.at(i) == '\r')
				assign_state(PARSING_CHUNKED_FINAL_CR);
			else
				assign_state(ERROR_PARSING);
			break ;
		case PARSING_CHUNKED_FINAL_CR:
			if (raw.at(i) == '\n')
			{
				if (request.body_bytes_read > location->client_max_body_size)
				{
					assign_state(ERROR_EXCEEDS_LIMIT);
					break ;
				}
				if (parse_is_last_chunck == true)
				{
					this->request.close_body_stream();
					assign_state(PROCESSING);
				}
				else
					assign_state(PARSING_CHUNKED_SIZE);
			}
			else
				assign_state(ERROR_PARSING);
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

bool is_allowed_method(const Location *matched_location, const VirtualServer *vir_server, const std::string &method)
{
	// 1. If we have a location and it has defined methods, use them
	if (matched_location && !matched_location->allow_methods.empty())
	{
		std::vector<std::string>::const_iterator it = std::find(matched_location->allow_methods.begin(), matched_location->allow_methods.end(), method);
		return (it != matched_location->allow_methods.end());
	}

	// 2. If no location match OR location has no methods, check the Server's methods
	if (vir_server && !vir_server->allow_methods.empty())
	{
		std::vector<std::string>::const_iterator it = std::find(vir_server->allow_methods.begin(), vir_server->allow_methods.end(), method);
		return (it != vir_server->allow_methods.end());
	}

	// 3. Default behavior: only GET is allowed if nothing is specified
	return (method == "GET");
}

void HttpTransaction::normalize_uri()
{
	std::string res;
	std::vector<std::string> uri_stack;
	std::vector<std::string> uri_split = split_string(request.uri,'/');

	std::vector<std::string>::iterator it = uri_split.begin();
	
	for (; it != uri_split.end(); it++)
	{
		if (it->compare("..") == 0)
		{
			if (!uri_stack.empty())
				uri_stack.pop_back();		
		}
		else if(it->compare(".") == 0 || it->empty())
			continue;
		else
			uri_stack.push_back((*it).c_str());
	}
	
	res += '/';
	
	for (size_t i = 0; i < uri_stack.size(); i++)
	{
		res += uri_stack[i];
		if (i < uri_stack.size() - 1)
			res += '/';
	}

	if (request.uri.size() > 2 && request.uri.at(request.uri.size() - 1) == '/' && res != "/")
		res += '/';
	
	request.uri = res;
}

void HttpTransaction::process_request(int curr_socket)
{
	if (state == PROCESSING)
	{
		normalize_uri();
		// 1. Handle location-specific rules (only if location context exists)
		if (location)
		{
			// 2.1. Handle Redirections (e.g., return 301 http://example.com)
			if (location->return_redir.first != 0)
			{
				response.add_header("Location", location->return_redir.second);
				response.set_head_method(request.method == "HEAD");
				response.build_response(location->return_redir.first);
				state = SENDING;
				return;
			}

			// 2.2. Handle Alias (replaces root/URI logic)
			if (!location->alias.empty())
			{
				std::string file_name = request.uri;
				if (file_name.find_last_of('/') != std::string::npos)
					file_name = file_name.substr(file_name.find_last_of('/') + 1);
				request.final_request_path = location->alias + file_name;
				resolve_resource();
				prepare_response();
				return;
			}
		}

		// 3. Find the correct path for the resource requested
		resolve_resource();

		// 4. Create response
		// 4.1 CGI Handling (check extension only if location and cgi settings exist)
		if (is_cgi)
		{
			request.final_request_path = build_cgi_path();
			prepare_response_cgi(curr_socket);
			return;
		}

		// 4.2 Execute non-CGI Response Preparation (GET/POST/DELETE logic)
		prepare_response();
	}
	else if (state == ERROR_PARSING)
	{
		build_error_response(400);
	}
	else if (state == ERROR_METHOD_NOT_ALLOWED)
	{
		build_error_response(405);
	}
	else if (state == ERROR_EXCEEDS_LIMIT)
	{
		response.add_header("Connection", "close");
		build_error_response(413);
	}
	else
		build_error_response(500);
}

void HttpTransaction::resolve_resource()
{
	struct stat	s;
	CgiHandler cgi;

	// 1. Determine base parameters (fallback to server settings if location is NULL)
	std::string root_path = (location) ? location->root : vir_server->root;

	// 2. Normal Request Path Construction
	std::string target_resource = request.uri;
	std::string loc_path = (location) ? location->path : "";

	// Remove the location prefix from the URI if it exists
	if (!loc_path.empty() && target_resource.find(loc_path) == 0)
		target_resource = target_resource.substr(loc_path.length());

	// Concatenate root and resource path safely to avoid missing slashes
	if (!target_resource.empty() && target_resource[0] == '/')
		request.final_request_path = root_path + target_resource;
	else
		request.final_request_path = root_path + "/" + target_resource;

	// 3. Add index if it is warranted
	if (stat(request.final_request_path.c_str(), &s) == 0)
	{
		// If the request uri matches a directory
		if (S_ISDIR(s.st_mode))
		{
			if (location)
			{
				if (request.final_request_path.at(request.final_request_path.length() - 1) != '/')
					request.final_request_path += "/";
				
				std::string	initial_str = request.final_request_path;
				// check if location has indices to return
				for (size_t i = 0; i < location->index.size(); i++)
				{
					std::string index_path = request.final_request_path + location->index.at(i);
					if (access(index_path.c_str(), R_OK) == 0)
					{
						request.final_request_path = index_path;
					}
				}
				if (request.final_request_path == initial_str)
					is_directory = true;
			}
			else
			{
				if (request.final_request_path.at(request.final_request_path.length() - 1) != '/')
					request.final_request_path += "/";
				
				std::string	initial_str = request.final_request_path;
				// check if virtual server has indices to return
				for (size_t i = 0; i < vir_server->index.size(); i++)
				{
					std::string index_path = request.final_request_path + vir_server->index.at(i);
					if (access(index_path.c_str(), R_OK) == 0)
					{
						request.final_request_path = index_path;
					}
				}
				if (request.final_request_path == initial_str)
					is_directory = true;
			}
		}
	}
	// 4. Update resource path if we are looking at a cgi request 
	// For 42 tester
	if (location && (ft_ends_with(request.final_request_path, ".bla") && request.method == "POST"))
	{
		request.final_request_path = build_cgi_path();
		is_cgi = true;
		return;
	}
	else if (location && !location->cgi_ext.empty() && ft_ends_with(request.final_request_path, location->cgi_ext) &&
		!ft_ends_with(request.final_request_path, ".bla"))
	{
		request.final_request_path = build_cgi_path();
		is_cgi = true;
		return;
	}
}

std::string HttpTransaction::build_cgi_path()
{
	std::string file_name;
	std::string final_path;
	std::size_t len_file_name;
	len_file_name =
		std::max(static_cast<std::size_t>(0),
					request.final_request_path.size() - request.final_request_path.find_last_of('/') - 1);
	if (len_file_name != 0)
		file_name = std::string(request.final_request_path,request.final_request_path.find_last_of('/') + 1, len_file_name);
	final_path = location->cgi_script_root + "/" + file_name;
	return (final_path);	
}

void HttpTransaction::prepare_response()
{
	struct stat	s;
	CgiHandler cgi;
	
	// Before checking if the file exists, we MUST check if the method is permitted.
	// This ensures a 405 error takes precedence over a 404 error.
	if (is_allowed_method(location, vir_server, request.method) == false)
	{
		build_error_response(405);
		return;
	}

	if (stat(request.final_request_path.c_str(), &s) == 0)
	{
		if (request.method == "GET")
			prepare_response_get();
		else if (request.method == "POST")
			prepare_response_post();
		else if (request.method == "DELETE")
			prepare_response_delete(s);
		else
			build_error_response(405);
	}
	else
		build_error_response(404);
	if (state == PROCESSING)
		state = SENDING;
}

void HttpTransaction::prepare_response_get()
{
	if (!is_directory &&
		access(request.final_request_path.c_str(), R_OK) != 0)
	{
		build_error_response(403);
		return;
	}
	build_response_get_resource();
}

void HttpTransaction::prepare_response_post()
{
	build_bodyless_response(200);
}

void HttpTransaction::prepare_response_delete(struct stat &s)
{
	if (S_ISDIR(s.st_mode))
	{
		build_error_response(403);
		return;
	}

	const std::string &p = request.final_request_path;
	std::string::size_type slash = p.find_last_of('/');
	std::string parent = (slash == std::string::npos) ? "." : p.substr(0, slash);
	if (parent.empty())
		parent = "/";

	if (access(parent.c_str(), W_OK | X_OK) != 0)
	{
		build_error_response(403);
		return;
	}

	if (unlink(p.c_str()) == 0)
		build_bodyless_response(204);
	else if (errno == EACCES || errno == EPERM)
		build_error_response(403);
	else
		build_error_response(500);
}

void HttpTransaction::prepare_response_cgi(int curr_socket)
{
	CgiHandler cgi;

	// execute cgi
	cgi_info = cgi.execute(location->cgi_path, request.final_request_path,
			request.body_file_path, *this, curr_socket);

	cgi_info.input_doc_path = request.body_file_path;
	state = WAITING_CGI;

	if (cgi_info.is_started == false)
	{
		request.clean_body_file();
		build_error_response(500);
	}
}

void HttpTransaction::build_response_get_resource()
{
	std::ifstream html_file;
	// If the request uri matches a directory
	if (is_directory)
	{
		// given there were no indices, check if we can return autoindex
		if (location && location->autoindex == true)
		{
			response.set_body(build_autoindex_string(request.final_request_path));
			if (response.get_body().size() == 0)
			{
				build_error_response(500);
				return ;
			}
			response.build_response(200);
		}
		else
		{
			// No index found and autoindex is off
			build_error_response(404);
		}
	}
	// If not a directory, then it is a Regular File requests
	else
	{
		if (open_file(&request.final_request_path.at(0), html_file) == true)
		{
			response.set_body(file_to_string(html_file));
			response.set_head_method(request.method == "HEAD");
			response.build_response(200);
		}
		else
		{
			// File exists (stat was OK) but cannot be opened (permissions, etc.)
			build_error_response(403);
		}
	}
}

void HttpTransaction::build_error_response(int error_code)
{
	response.set_status(error_code);
	response.set_body(generate_error_page(error_code));
	response.set_head_method(request.method == "HEAD");
	response.serialize_response();
	state = SENDING;
}

void HttpTransaction::build_bodyless_response(int status)
{
	response.set_status(status);
	response.set_head_method(request.method == "HEAD");
	response.serialize_response();
	state = SENDING;
}

std::string HttpTransaction::generate_error_page(int error_code)
{
	if (location && location->error_page.count(error_code))
	{
		std::string file_path =	location->root + location->error_page.at(error_code);
		std::ifstream file_stream(file_path.c_str());
		if (file_stream.is_open())
			return std::string((std::istreambuf_iterator<char>(file_stream)),
								std::istreambuf_iterator<char>());
	}
	else if (vir_server->error_page.count(error_code))
	{
		std::string file_path = vir_server->root + vir_server->error_page.at(error_code);
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
		return ("");
	}
	for (struct dirent *file = readdir(dir); file; file = readdir(dir))
	{
		if (std::string(file->d_name) == "."
			|| std::string(file->d_name) == "..")
			continue ;
		page += "<li><a href=http://localhost:";
		page += ft_int_to_string(this->vir_server->listen);
		page += request.uri;
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

void HttpTransaction::assign_state(State s)
{	
	this->state = s;
}