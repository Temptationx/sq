#ifndef type_h__
#define type_h__

#include <memory>
#include <vector>
#include <map>
#include <functional>
#include "streamid.hpp"

typedef std::map<std::string, std::string> HeaderMap;

struct Request
{
	std::shared_ptr<HeaderMap> headers;
	std::string url;
	int method = 0;
};

struct Response
{
	Response(){}
	Response(std::shared_ptr<HeaderMap> headers_, 
			std::shared_ptr<std::vector<uint8_t>> body_,
			int status_,
			const std::string &status_text_)
			:headers(headers_),
			body(body_),
			status(status_),
			status_text(status_text_)
			{}
	std::shared_ptr<HeaderMap> headers;
	std::shared_ptr<std::vector<uint8_t>> body;
	int status = 0;
	std::string status_str;
	bool operator == (const Response &rV) const;
	bool operator == (const Response &rV);
};

class ResponseBuilder
{
public:
	static Response* build(const std::string &body, std::map<std::string, std::string> h, int s, const std::string &ss);
};

enum CallReason{ CallReasonRequest, CallReasonResponse, CallReasonClose };

typedef std::function<std::shared_ptr<Response>(const std::string &reqUrl)> ServerCallback;

#ifdef _WIN32
#	define MEMCPY(__DST__, __SIZE__, __SRC__) memcpy_s(__DST__, __SIZE__, __SRC__, __SIZE__);
#else
#	define MEMCPY(__DST__, __SIZE__, __SRC__) memcpy(__DST__, __SRC__, __SIZE__);
#endif // _WIN32
#endif // type_h__
