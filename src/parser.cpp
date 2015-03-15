#include <string>
#include <http_parser.h>
#include "utility.hpp"
#include "parser.hpp"
#include "type.hpp"


ParserBase::ParserBase() : ParserBase(0)
{
}

ParserBase::ParserBase(int mode)
{
	p = std::make_unique<http_parser>();
	settings = std::make_unique<http_parser_settings>();
	p->data = this;
	memset(settings.get(), 0, sizeof(http_parser_settings));
	settings->on_message_begin = onMessageBegin;
	settings->on_message_complete = onMessageComplete;
	settings->on_header_field = onHeaderField;
	settings->on_header_value = onHeaderValue;
	settings->on_body = onBody;
	http_parser_init(p.get(), http_parser_type(mode));
	header = std::make_shared<std::map<std::string, std::string>>();
	m_body = std::make_shared<std::vector<uint8_t>>();
}

bool ParserBase::complete()
{
	return is_complete;
}

void ParserBase::parse(const char *data, size_t size)
{
	http_parser_execute(p.get(), settings.get(), data, size);
}

std::shared_ptr<std::map<std::string, std::string>> ParserBase::headers()
{
	return header;
}

std::shared_ptr<std::vector<uint8_t>> ParserBase::body()
{
	return m_body;
}

void ParserBase::reset()
{
	http_parser_init(p.get(), http_parser_type(p->type));
	field.clear();
	header = std::make_shared<std::map<std::string, std::string>>();
	m_body = std::make_shared<std::vector<uint8_t>>();
	is_complete = false;
	is_beg = false;
}

int ParserBase::onMessageBegin(http_parser *parser)
{
	auto _this = (THIS_PTR)parser->data;
	_this->is_beg = true;
	return 0;
}

int ParserBase::onMessageComplete(http_parser *parser)
{
	auto _this = (THIS_PTR)parser->data;
	_this->is_complete = true;
	return 0;
}

int ParserBase::onHeaderField(http_parser *parser, const char* data, size_t length)
{
	auto _this = (THIS_PTR)parser->data;
	_this->field = std::string(data, length);
	return 0;
}

int ParserBase::onHeaderValue(http_parser *parser, const char* data, size_t length)
{
	auto _this = (THIS_PTR)parser->data;
	_this->header->emplace(_this->field, std::string(data, length));
	return 0;
}

int ParserBase::onBody(http_parser *parser, const char* data, size_t length)
{
	auto _this = (THIS_PTR)parser->data;
	_this->m_body->insert(_this->m_body->end(), data, data + length);
	return 0;
}

bool ParserBase::begin()
{
	return is_beg;
}

RequestParser::RequestParser() : ParserBase(0)
{
	settings->on_url = onUrl;
	settings->on_headers_complete = onHeaderComplete;
}

void RequestParser::reset()
{
	ParserBase::reset();
	m_method = -1;
	m_url.clear();
	req = nullptr;
}

int RequestParser::onUrl(http_parser *parser, const char *data, size_t size)
{
	auto _this = (RequestParser*)parser->data;
	_this->m_url = std::string(data, size);
	return 0;
}

int RequestParser::onHeaderComplete(http_parser *parser)
{
	auto _this = (RequestParser*)parser->data;
	if (_this->m_url.at(0) == '/') {
		_this->m_url = std::string("http://") + (*(_this->header))["Host"] + _this->m_url;
	}
	_this->m_method = parser->method;
	return 0;
}

std::shared_ptr<Request> RequestParser::get()
{
	if (!is_complete) {
		return nullptr;
	}
	if (!req) {
		req = std::make_shared<Request>();
		req->headers = move(header);
		req->method = m_method;
		req->url = m_url;
	}
	return req;
}

ResponseParser::ResponseParser() : ParserBase(1)
{
	settings->on_headers_complete = onHeaderComplete;
	settings->on_status = onStatus;
}

void ResponseParser::reset()
{
	ParserBase::reset();
	status = 0;
	res = nullptr;
}

int ResponseParser::onHeaderComplete(http_parser *parser)
{
	auto _this = (ResponseParser*)parser->data;
	auto &it = _this->headers()->find("Transfer-Encoding");
	if (it != _this->headers()->end()) {
		_this->headers()->erase(it);
	}
	_this->status = parser->status_code;
	return 0;
}

std::shared_ptr<std::vector<uint8_t>> uncompress_body(std::shared_ptr<std::vector<uint8_t>> body)
{
	char *buf = nullptr;
	int size = 0;
	auto state = decode_gzip((char*)body->data(), body->size(), &buf, &size);
	if (state && size && buf) {
		auto new_body = std::make_shared<std::vector<uint8_t>>(buf, buf + size);
		delete[] buf;
		return new_body;
	}
	else{
		delete[] buf;
		return nullptr;
	}
}

void process_body(std::shared_ptr<Response> &res, const std::shared_ptr<std::vector<uint8_t>> &body)
{
	// First if we have body
	//  delete content-length
	//  Check if we have compresssed
	//	 decompress
	//   check if decompressed right
	//    if failed return body
	//    if ok delete content-encoding return new_body
	//  no compress return body
	// No body return null
	if (!body || body->size() == 0) {
		return;
	}
	res->headers->erase("Content-Length");
	auto it = res->headers->find("Content-Encoding");
	if (it != res->headers->end() && it->second != "none") {
		auto new_body = uncompress_body(body);
		// Reason: Failed decompression imply bad data, but we just return that data.
		if (new_body) {
			res->body = new_body;
			res->headers->erase(it);
		}
		else{
			res->body = body;
		}
	}
	else{
		res->body = body;
	}
}

std::shared_ptr<Response> ResponseParser::get()
{
	if (!is_complete) {
		return nullptr;
	}
	if (!res) {
		res = std::make_shared<Response>();
		res->headers = move(header);
		if (!res->headers) {
			res->headers = std::make_shared<std::map<std::string, std::string>>();
		}
		res->status = status;
		res->status_text = status_str;
		process_body(res, m_body);
		m_body = nullptr;
	}
	return res;
}

int ResponseParser::onStatus(http_parser *parser, const char* data, size_t length)
{
	auto _this = (ResponseParser*)parser->data;
	_this->status_str = std::string(data, length);
	return 0;
}
