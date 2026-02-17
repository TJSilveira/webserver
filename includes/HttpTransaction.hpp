/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTransaction.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:18:58 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/17 22:02:25 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPTRANSACTION_HPP
# define HTTPTRANSACTION_HPP

# include "CgiHandler.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "VirtualServer.hpp"
# include "socket.hpp"
# include "utils.hpp"
# include <dirent.h>
# include <sys/stat.h>

class HttpTransaction
{
private:
	std::string temp;
	std::string temp_key;
	std::string temp_value;
	int parse_chuncked_bytes_to_read;
	std::string parse_chuncked_size_str;
	bool parse_is_last_chunck;

public:
	HttpTransaction(const VirtualServer *vir_server);
	~HttpTransaction();

	enum	State
	{
		PARSING_REQ_METHOD,
		PARSING_REQ_URI,
		PARSING_REQ_VERSION,
		PARSING_REQ_LINE_CR,
		PARSING_HEADER_KEY,
		PARSING_HEADER_OWS,
		PARSING_HEADER_VALUE,
		PARSING_HEADER_CR,
		PARSING_HEADER_DONE,
		PARSING_HEADER_FINAL_CR,
		PARSING_BODY,
		PARSING_CHUNCKED_SIZE,
		PARSING_CHUNCKED_SIZE_CR,
		PARSING_CHUNCKED_BODY,
		PARSING_CHUNCKED_CR,
		PARSING_CHUNCKED_DONE,
		PARSING_CHUNCKED_FINAL_CR,
		PARSING_ERROR,
		ERROR_EXCEEDS_LIMIT,
		ERROR_METHOD_NOT_ALLOWED,
		PROCESSING,
		WAITING_CGI,
		SENDING,
		COMPLETE
	};


	void parse(const std::string &raw);
	void process_request(int epollfd, int curr_socket);
	void prepare_response(int epollfd, int curr_socket, std::string final_path);
	void prepare_response_get(std::string final_path, struct stat &s);
	void prepare_response_post(int curr_socket, std::string final_path, struct stat &s);
	void build_error_response(int error_code);
	std::string build_cgi_path();
	const Location *find_location();
	std::string generate_error_page(int error_code);

	void build_response_found_resource(const Location *matched_location,
										struct stat &s,
										std::string &final_path);

	std::string build_autoindex_string(std::string &dir_path);

	void mark_as_complete();
	void assign_state(State s, char c);

	HttpRequest request;
	HttpResponse response;
	const VirtualServer *vir_server;
	const Location *location;
	struct CgiInfo cgi_info;
	State	state;
};

bool	is_allowed_method(const Location *matched_location,
						const std::string &method);

#endif
