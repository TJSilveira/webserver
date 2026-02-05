#include "../includes/HttpTransaction.hpp"

HttpTransaction::HttpTransaction(const VirtualServer *vir_server):vir_server(vir_server), state(PARSING_REQ_METHOD) {}

HttpTransaction::~HttpTransaction()
{
}

void HttpTransaction::parse(const std::string &raw)
{
	static std::string	temp;
	static std::string	temp_key;
	static std::string	temp_value;

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
			{
				// Add 400 error
			}
			break;
		case PARSING_HEADER_KEY:
			if (raw.at(i) == ':')
			{
				this->state = PARSING_HEADER_OWS;
			}
			else if(isspace(raw.at(i)))
			{
				// Add 400 error
			}
			else
				temp_key += raw.at(i);
			break;
		case PARSING_HEADER_OWS:
			if (raw.at(i) == ' ')
				continue;
			else if(raw.at(i) == '\r' || raw.at(i) == '\n')
			{
				// Add 400 error
			}
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
			else if (isspace(raw.at(i)))
			{
				// Add 400 error
			}
			else
				temp_value += raw.at(i);
			break;
		case PARSING_HEADER_CR:
			if (raw.at(i) == '\n')
				this->state = PARSING_HEADER_DONE;
			else
			{
				// Add 400 error
			}
			break;
		case PARSING_HEADER_DONE:
			if (raw.at(i) == '\r')
				this->state = PARSING_HEADER_FINAL_CR;
			else if(isspace(raw.at(i)))
			{
				// Add 400 error
			}
			else
			{
				this->state = PARSING_HEADER_KEY;
				i--;
			}
			break;
		case PARSING_HEADER_FINAL_CR:
			if (raw.at(i) == '\n')
				this->state = PARSING_BODY;
			else
			{
				// Add 400 error
			}
			break;
		case PARSING_BODY:
			this->request.body += raw.at(i);
		default:
			break;
		}
	}
}
