#include "serialization.hpp"
#include <string>
#include <cassert>


bool StringStream::extract(std::string &str_, const char *e, int es /*= -1*/)
{
	if (es < 0) {
		es = strsize(e, es);
	}
	int tmp_pos = pos;
	auto ep = find(e, es);
	if (ep < 0) {
		return false;
	}
	str_ = std::string(str, tmp_pos, ep - tmp_pos);
	pos = ep + es;
	return true;
}

bool StringStream::extract(std::string &str_, const char *s, const char *e, int ss /*= -1*/, int es /*= -1*/)
{
	if (ss < 0) {
		ss = strsize(s, ss);
	}
	if (es < 0) {
		es = strsize(e, es);
	}
	auto sp = find(s, ss);
	auto ep = find(e, es);
	if (sp < 0 || ep < 0) {
		return false;
	}
	if (sp >= ep) {
		return false;
	}
	str_ = std::string(str, sp + ss, ep - sp - ss);
	// abcdefghi
	// 0123456789
	//  12   67
	pos = ep + es;
	return true;
}

int StringStream::strsize(const char *s, int size_)
{
	if (size_ < 0) {
		const char *tmp = s;
		while (*(++tmp)){}
		size_ = tmp - s;
	}
	return size_;
}

int StringStream::find(const char *s, int size_ /*= -1*/)
{
	assert(size > 0 && str);
	auto ssize = strsize(s, size_);
	if (ssize > size || ssize <= 0) {
		return -1;
	}
	for (int i = pos; i < size - ssize + 1; i++)
	{
		int j = 0;
		for (; j < ssize; j++)
		{
			if (str[i + j] != s[j]) {
				break;
			}
		}
		if (j == ssize) {
			pos = i + ssize;
			return i;
		}
	}
	return -1;
}

StringStream::StringStream(const char *str_, int size_ /*= -1*/) : str(str_), size(strsize(str_, size_))
{
}


Serialization::~Serialization()
{
}

size_t Serialization::serialize(char **buf, const std::pair<std::string, Response*> &value)
{
	if (!value.second) {
		return 0;
	}
	auto &url = value.first;
	auto &response = *value.second;
	size_t e_size = 0;
	e_size += url.size();
	std::string header;
	if (response.headers) {
		for (auto &it : *response.headers) {
			header.append(it.first + ": " + it.second + "\r\n");
		}
	}
	e_size += header.size();
	if (response.body) {
		e_size += response.body->size();
	}
	e_size += 512;
	char *b = new char[e_size];
	size_t p_size = sprintf(b, "GET %s HTTP/1.1\r\n\r\n", url.data());
	p_size += sprintf(b + p_size, "HTTP/1.1 %d %s\r\n%s\r\n", response.status, response.status_text.data(), header.data());
	if (response.body) {
		memcpy(b + p_size, response.body->data(), response.body->size());
		p_size += response.body->size();
	}
	*buf = b;
	return p_size;
}

size_t Serialization::serialize(char **buf, const CachePacket &value)
{
	assert(value.response);

	auto &url = value.url;
	auto &response = *value.response;

	// calculate buffer size
	size_t buf_size = 0;
	buf_size += url.size();
	std::string header_text;
	// serialize header
	if (response.headers) {
		for (auto &it : *response.headers) {
			header_text.append(it.first + ": " + it.second + "\r\n");
		}
	}
	buf_size += header_text.size();
	if (response.body) {
		buf_size += response.body->size();
	}
	buf_size += 512;	// additional buffer size
	// serialize
	char *data = new char[buf_size];
	size_t used_size = sprintf(data, "GET %s HTTP/1.1\r\n\r\n", url.data());
	used_size += sprintf(data + used_size, "HTTP/1.1 %d %s\r\n%s\r\n", response.status, response.status_text.data(), header_text.data());
	if (response.body) {
		memcpy(data + used_size, response.body->data(), response.body->size());
		used_size += response.body->size();
	}
	*buf = data;
	return used_size;
}

CachePacket* Serialization::parse2(size_t size, const char *buf)
{
	StringStream ss(buf, size);
	std::string url;
	try {
		ss.extract(url, " ", " ");
		ss.find("\r\n\r\n");  // Skip to response
		std::string status_text;
		ss.extract(status_text, " ", " ");
		int status_code = std::stoi(status_text);
		ss.pos -= 1;
		ss.extract(status_text, " ", "\r\n");
		ss.pos -= 2;
		auto headers = std::make_shared<std::map<std::string, std::string>>();
		std::string head;
		while (true)
		{
			ss.extract(head, "\r\n", "\r\n");
			if (head.empty()) {
				break;
			}
			ss.pos -= 2;
			auto pos = head.find(":");
			headers->emplace(std::string(head, 0, pos), std::string(head, pos + 2));

		}
		auto body = std::make_shared<std::vector<uint8_t>>(ss.str + ss.pos, ss.str + ss.size);
		auto response = std::make_shared<Response>(headers, body, status_code, status_text);
		return new CachePacket{url, nullptr, response};
	}
	catch (...) {
		return nullptr;
	}
}

std::pair<std::string, Response*> Serialization::parse(const char *buf, size_t size)
{
	StringStream ss(buf, size);
	std::string url;
	try {
		ss.extract(url, " ", " ");
		ss.find("\r\n\r\n");  // Skip to response
		std::string status;
		ss.extract(status, " ", " ");
		int status_code = std::stoi(status);
		ss.pos -= 1;
		ss.extract(status, " ", "\r\n");
		ss.pos -= 2;
		auto headers = std::make_shared<std::map<std::string, std::string>>();
		std::string head;
		while (true)
		{
			ss.extract(head, "\r\n", "\r\n");
			if (head.empty()) {
				break;
			}
			ss.pos -= 2;
			auto pos = head.find(":");
			headers->emplace(std::string(head, 0, pos), std::string(head, pos + 2));

		}
		auto body = std::make_shared<std::vector<uint8_t>>(ss.str + ss.pos, ss.str + ss.size);
		auto res = new Response;
		res->body = body;
		res->headers = headers;
		res->status = status_code;
		res->status_text = status;
		return std::make_pair(url, res);
	}
	catch (...) {
		return std::pair<std::string, Response*>();
	}
}