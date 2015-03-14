#include <gmock/gmock.h>
#include <vector>
#include <memory>
using namespace std;
/*
Requirement:
recv the data
determin current state.
set data
change state
*/

#include <http_parser.h>
#include "../src/parser.hpp"



TEST(PARSERBASE, STATE)
{
	ParserBase parser;
	string req1("GET http://www.baidu.com HTTP/1.1\r\n\r\n");
	parser.parse(req1.data(), req1.size());
	ASSERT_TRUE(parser.complete());
	ASSERT_TRUE(parser.begin());
}

TEST(PARSERBASE, HEAD)
{
	ParserBase parser;
	string req1("GET http://www.baidu.com HTTP/1.1\r\nHost: www.baidu.com\r\nAccept-ENcoding: gzip, deflate\r\n\r\n");
	parser.parse(req1.data(), req1.size());
	map<string, string> head{ { "Host", "www.baidu.com" }, { "Accept-ENcoding", "gzip, deflate" } };
	ASSERT_EQ(*(parser.headers()), head);
}

TEST(PARSERBASE, BODY)
{
	ParserBase parser;
	string req1("POST http://www.baidu.com HTTP/1.1\r\nHost: www.baidu.com\r\nAccept-ENcoding: gzip, deflate\r\nContent-Length: 4\r\n\r\nmake");
	parser.parse(req1.data(), req1.size());
	vector<uint8_t> body{ 'm', 'a', 'k', 'e' };
	ASSERT_EQ(*parser.body(), body);
}

TEST(PARSERBASE, RESET)
{
	ParserBase parser;
	string req1("POST http://www.baidu.com HTTP/1.1\r\nHost: www.baidu.com\r\nAccept-ENcoding: gzip, deflate\r\nContent-Length: 4\r\n\r\nmake");
	parser.parse(req1.data(), req1.size());
	parser.reset();
	ASSERT_EQ(parser.body()->size(), 0);
	ASSERT_FALSE(parser.complete());
	ASSERT_FALSE(parser.begin());
}


TEST(RequestParser, URL)
{
	RequestParser parser;
	string req1("GET http://www.baidu.com/ HTTP/1.1\r\n\r\n");
	parser.parse(req1.data(), req1.size());
	ASSERT_EQ(parser.m_url, "http://www.baidu.com/");
}

TEST(RequestParser, URLWithHost)
{
	RequestParser parser;
	string req1("GET / HTTP/1.1\r\nHost: www.baidu.com\r\nAccept-ENcoding: gzip, deflate\r\n\r\n");
	parser.parse(req1.data(), req1.size());
	ASSERT_EQ(parser.m_url, "http://www.baidu.com/");
}

TEST(RequestParser, METHOOD)
{
	RequestParser parser;
	string req1("POST http://www.baidu.com/ HTTP/1.1\r\n\r\n");
	parser.parse(req1.data(), req1.size());
	ASSERT_EQ(parser.m_method, 3);
}

TEST(RequestParser, RESET)
{
	RequestParser parser;
	string req1("GET http://www.baidu.com/ HTTP/1.1\r\n\r\n");
	parser.parse(req1.data(), req1.size());
	parser.reset();
	ASSERT_EQ(parser.m_url.size(), 0);
	ASSERT_EQ(parser.m_method, -1);
	parser.parse(req1.data(), req1.size());
	ASSERT_EQ(parser.m_url, "http://www.baidu.com/");
	ASSERT_EQ(parser.m_method, 1);
}


TEST(ResponseParser, HEAD_TRANSFER_ENCODING)
{
	ResponseParser parser;
	string req1("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
	parser.parse(req1.data(), req1.size());
	auto &it = parser.headers()->find("Transfer-Encoding");
	ASSERT_EQ(it, parser.headers()->end());
}

TEST(ResponseParser, STATUS)
{
	ResponseParser parser;
	string req1("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
	parser.parse(req1.data(), req1.size());
	ASSERT_EQ(parser.status, 200);
}

TEST(ResponseParser, RESET)
{
	ResponseParser parser;
	string req1("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
	parser.parse(req1.data(), req1.size());
	ASSERT_EQ(parser.status, 200);
	parser.reset();
	ASSERT_EQ(parser.status, 0);
	parser.parse(req1.data(), req1.size());
	ASSERT_EQ(parser.status, 200);
}
