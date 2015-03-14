#ifndef parser_h__
#define parser_h__
#include <memory>
#include <vector>
#include <map>
#include <http_parser.h>
#include <cstdint>
struct http_parser;
struct http_parser_settings;
class Request;
class Response;
class ParserBase
{
	typedef ParserBase* THIS;
public:
	ParserBase();
	ParserBase(int mode);
	bool complete();
	bool begin();
	void parse(const char *data, size_t size);
	std::shared_ptr<std::map<std::string, std::string>> headers();
	std::shared_ptr<std::vector<uint8_t>> body();
	virtual void reset();
protected:
	std::shared_ptr<std::vector<uint8_t>> m_body;
	std::shared_ptr<std::map<std::string, std::string>> header;
	std::unique_ptr<http_parser> p;
	std::unique_ptr<http_parser_settings> settings;
	std::string field;
	bool is_complete = false;
	bool is_beg = false;
private:
	static int onMessageBegin(http_parser *parser);
	static int onMessageComplete(http_parser *parser);
	static int onHeaderField(http_parser *parser, const char* data, size_t length);
	static int onHeaderValue(http_parser *parser, const char* data, size_t length);
	static int onBody(http_parser *parser, const char* data, size_t length);
};

class RequestParser : public ParserBase
{
public:
	RequestParser();
	std::string m_url;
	int m_method = -1;
	virtual void reset();
	std::shared_ptr<Request> get();
private:
	std::shared_ptr<Request> req;
	static int onUrl(http_parser *parser, const char *data, size_t size);
	static int onHeaderComplete(http_parser *parser);
};

class ResponseParser : public ParserBase
{
public:
	ResponseParser();
	int status = 0;
	std::string status_str;
	virtual void reset();
	std::shared_ptr<Response> get();
private:
	std::shared_ptr<Response> res;
	static int onHeaderComplete(http_parser *parser);
	static int onStatus(http_parser *parser, const char* data, size_t length);
};
#endif // parser_h__
