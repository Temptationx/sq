#include "type.hpp"

bool Response::operator==(const Response &rV)
{
	if (status != rV.status) {
		return false;
	}
	if (status_str != rV.status_str) {
		return false;
	}
	if (!headers && rV.headers || headers && !rV.headers) {
		return false;
	}
	if (!body && rV.body || body && !rV.body) {
		return false;
	}
	if (headers && rV.headers) {
		if (*headers != *rV.headers) {
			return false;
		}
	}
	if (body && rV.body) {
		if (*body != *rV.body) {
			return false;
		}
	}
	return true;
}

bool Response::operator==(const Response &rV) const
{
	if (status != rV.status) {
		return false;
	}
	if (status_str != rV.status_str) {
		return false;
	}
	if (!headers && rV.headers || headers && !rV.headers) {
		return false;
	}
	if (!body && rV.body || body && !rV.body) {
		return false;
	}
	if (headers && rV.headers) {
		if (*headers != *rV.headers) {
			return false;
		}
	}
	if (body && rV.body) {
		if (*body != *rV.body) {
			return false;
		}
	}
	return true;
}

Response* ResponseBuilder::build(const std::string &body, std::map<std::string, std::string> h, int s, const std::string &ss)
{
	auto res = new Response;
	std::remove_pointer<decltype(res->body)>::type a;
	res->body = std::make_shared<std::vector<uint8_t>>(begin(body), end(body));
	res->status = s;
	res->status_str = ss;
	res->headers = std::make_shared<std::map<std::string, std::string>>(begin(h), end(h));
	return res;
}
