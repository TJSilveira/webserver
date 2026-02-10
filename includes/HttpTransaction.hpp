#ifndef HTTPTRANSACTION_HPP
#define HTTPTRANSACTION_HPP

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "VirtualServer.hpp"
#include "socket.hpp"
#include "utils.hpp"

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>

class HttpTransaction
{
private:
	std::string temp;
    std::string temp_key;
    std::string temp_value;
	int			parse_chuncked_bytes_to_read;
	std::string	parse_chuncked_size_str;
	bool		parse_is_last_chunck;
public:
	HttpTransaction(const VirtualServer *vir_server);
	~HttpTransaction();

	void			parse(const std::string &raw);
	void			process_request(int epollfd, int curr_socket);
	void			send_response(int curr_socket);
	void			build_error_reponse(int error_code);
	const Location*	find_location();
	std::string		generate_error_page(int error_code);

	std::string	build_autoindex_string(const Location *matched_location);

	HttpRequest		request;
	HttpResponse	response;
	const VirtualServer	*vir_server;

	enum State {
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
		PROCESSING,
		WAITING_CGI,
		SENDING,
		COMPLETE
	};
	State			state;
};

bool is_allowed_method(const Location *matched_location, const std::string& method);

#endif