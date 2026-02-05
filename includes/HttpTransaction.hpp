#ifndef HTTPTRANSACTION_HPP
#define HTTPTRANSACTION_HPP

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "VirtualServer.hpp"

class HttpTransaction
{
private:
	/* data */
public:
	HttpTransaction(const VirtualServer *vir_server);
	~HttpTransaction();

	void	parse(const std::string &raw);

	HttpRequest		request;
	HttpResponse	response;
	const VirtualServer	*vir_server;

	std::string		write_buffer;

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
		PROCESSING,
		WAITING_CGI,
		SENDING
	};
	State			state;
};

#endif