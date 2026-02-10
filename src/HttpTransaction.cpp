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
			{
				this->state = PROCESSING;
				break;
			}
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

void	HttpTransaction::process_request(int epollfd, int curr_socket)
{
	std::cout << "Inside the process request\n";
	// If the status is not in Processing, there is nothing to process
	if (state == PROCESSING)
	{
		std::cout << "Inside Processing\n";
		response.build_response();
		change_socket_epollout(epollfd, curr_socket);
	}
	else if (state == PARSING_ERROR)
	{
		std::cout << "Inside Parsing Error\n";
		build_error_reponse(400);
		change_socket_epollout(epollfd, curr_socket);
	}
}

void	HttpTransaction::send_response(int curr_socket)
{
	while (response._bytes_sent < response._response_buffer.size())
	{
		size_t	missing_bytes = response._response_buffer.size() - response._bytes_sent;

		ssize_t current_sent_bytes = send(curr_socket,
			&response._response_buffer[response._bytes_sent], 
			missing_bytes, MSG_NOSIGNAL);

		if (current_sent_bytes > 0)
		{
			response._bytes_sent += current_sent_bytes;
		}
		else if (current_sent_bytes < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
		{
			return;
		}
		else
		{
			// Fatal error
			std::cout << "Send error\n";
			return;
		}
	}
	std::cout << "Finalized send\n";
	this->state = COMPLETE;
}

void HttpTransaction::build_error_reponse(int error_code)
{
	response.set_status(error_code);
	response.set_body(generate_error_page(error_code));
	response.serialize_response();
	state = SENDING;
}

std::string	HttpTransaction::generate_error_page(int error_code)
{
	if (vir_server->error_page.count(error_code))
	{
		std::string		file_path = vir_server->root + vir_server->error_page.at(error_code);
		std::ifstream	file_stream(file_path.c_str());

		if (file_stream.is_open())
			return std::string((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
	}
	return ("<html><body><h1>" + ft_int_to_string(error_code) + " Error</h1></body></html>");
}
