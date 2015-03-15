#ifndef serialization_h__
#define serialization_h__

#include "type.hpp"

class StringStream
{
public:
	StringStream(const char *str_, int size_ = -1);
	int find(const char *s, int size_ = -1);
	int strsize(const char *s, int size_);
	bool extract(std::string &str_, const char *s, const char *e, int ss = -1, int es = -1);
	bool extract(std::string &str_, const char *e, int es = -1);
	const char *str = nullptr;
	int size = 0;
	int pos = 0;
};

class Serialization
{
public:
	virtual ~Serialization();
	static size_t serialize(char **buf, const std::pair<std::string, Response*> &value);
	static size_t serialize(char **buf, const CachePacket &value);
	static std::pair<std::string, Response*> parse(const char *buf, size_t size);
	static CachePacket* parse2(size_t size, const char *buf);
};
#endif // serialization_h__
