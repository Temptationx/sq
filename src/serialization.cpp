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
		res->status_str = status;
		return std::make_pair(url, res);
	}
	catch (...) {
		return std::pair<std::string, Response*>();
	}
}

size_t Serialization::serialize(char **buf, const std::pair<std::string, Response*> &value)
{
	if (!value.second) {
		return 0;
	}
	auto &url = value.first;
	auto &res = *value.second;
	size_t e_size = 0;
	e_size += url.size();
	std::string header;
	if (res.headers) {
		for (auto &it : *res.headers)
		{
			header.append(it.first + ": " + it.second + "\r\n");
		}
	}
	e_size += header.size();
	if (res.body) {
		e_size += res.body->size();
	}
	e_size += 512;
	char *b = new char[e_size];
	size_t p_size = sprintf(b, "GET %s HTTP/1.1\r\n\r\n", url.data());
	p_size += sprintf(b + p_size, "HTTP/1.1 %d %s\r\n%s\r\n", res.status, res.status_str.data(), header.data());
	if (res.body) {
		memcpy(b + p_size, res.body->data(), res.body->size());
		p_size += res.body->size();
	}
	*buf = b;
	return p_size;
}
