#ifndef HTTPTRANSACTION_HPP
#define HTTPTRANSACTION_HPP

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "VirtualServer.hpp"
#include "socket.hpp"
#include "utils.hpp"

#include <iostream>
#include <fstream>

class HttpTransaction
{
private:
	std::string temp;
    std::string temp_key;
    std::string temp_value;
public:
	HttpTransaction(const VirtualServer *vir_server);
	~HttpTransaction();

	void	parse(const std::string &raw);
	void	process_request(int epollfd, int curr_socket);
	void	send_response(int curr_socket);
	void	build_error_reponse(int error_code);
	std::string	generate_error_page(int error_code);

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
		PARSING_ERROR,
		PROCESSING,
		WAITING_CGI,
		SENDING,
		COMPLETE
	};
	State			state;
};

#endif