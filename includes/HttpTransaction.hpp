/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTransaction.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:18:58 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/25 14:36:41 by tsilveir         ###   ########.fr       */
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

	bool is_directory;
	bool is_cgi;

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
		PARSING_CHUNKED_SIZE,
		PARSING_CHUNKED_SIZE_CR,
		PARSING_CHUNKED_BODY,
		PARSING_CHUNKED_CR,
		PARSING_CHUNKED_DONE,
		PARSING_CHUNKED_FINAL_CR,
		ERROR_PARSING,
		ERROR_DISK_WRITTING,
		ERROR_EXCEEDS_LIMIT,
		ERROR_METHOD_NOT_ALLOWED,
		PROCESSING,
		WAITING_CGI,
		SENDING,
		SENDING_ERROR,
		COMPLETE
	};

	void parse(const std::string &raw, int socketfd);
	void process_request(int epollfd, int curr_socket);
	void normalize_uri();
	void resolve_resource();
	void prepare_response(int epollfd, int curr_socket);
	void prepare_response_get();
	void prepare_response_post();
	void prepare_response_delete(struct stat &s);
	void prepare_response_cgi(int curr_socket);
	void build_error_response(int error_code);
	void build_bodyless_response(int status);
	std::string build_cgi_path();
	const Location *find_location();
	std::string generate_error_page(int error_code);

	void build_response_get_resource();

	std::string build_autoindex_string(std::string &dir_path);

	void mark_as_complete();
	void assign_state(State s);
	void assign_state(State s, const std::string &raw, int socketfd, int i);

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
