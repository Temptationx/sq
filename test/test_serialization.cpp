#include <gmock/gmock.h>
#include <md5.h>
#include <chrono>

#include "../src/type.hpp"
#include "../src/serialization.hpp"
#include "../src/stream.hpp"
using namespace std;


class SerializationTest : public testing::Test
{
public:
	void SetUp()
	{
		res.body = make_shared<vector<uint8_t>>();
		string body_str("Hello");
		res.body->insert(res.body->end(), body_str.begin(), body_str.end());
		res.status = 200;
		res.status_str = "OK";
		res.headers = make_shared<map<string, string>>();
		res.headers->emplace("Content-Length", "5");
		
	}
	Stream stream;
	Response res;
	 
	string response_str = "GET http://www.baidu.com/ HTTP/1.1\r\n\r\nHTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
};

TEST_F(SerializationTest, serialize)
{
	Serialization p;
	char *buf;
	size_t size = p.serialize(&buf, make_pair("http://www.baidu.com/", &res));
	ASSERT_EQ(size, 81);
	ASSERT_EQ(string(buf, size), "GET http://www.baidu.com/ HTTP/1.1\r\n\r\nHTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello");
	delete [] buf;
}

TEST_F(SerializationTest, parse)
{
	Serialization p;
	auto res_pair = p.parse(response_str.data(), response_str.size());
	auto url = res_pair.first;
	auto res1 = res_pair.second;
	ASSERT_EQ("http://www.baidu.com/", url);
	ASSERT_TRUE(res1);
	ASSERT_EQ(res, *res1);
	delete res1;
}

TEST_F(SerializationTest, serializethenparse)
{
	Serialization p;
	char *buf;
	size_t size = p.serialize(&buf, make_pair("http://www.baidu.com/", &res));
	auto res_pair = p.parse(buf, size);
	auto url = res_pair.first;
	auto res1 = res_pair.second;
	ASSERT_EQ("http://www.baidu.com/", url);
	ASSERT_TRUE(res1);
	ASSERT_EQ(res, *res1);
	delete res1;
	delete[] buf;
}

TEST_F(SerializationTest, Error)
{
	Serialization p;
	auto size = p.serialize(nullptr, make_pair("http://www.baidu.com/", nullptr));
	ASSERT_EQ(0, size);
}



TEST(SS, find)
{
	StringStream ss("abcdefg");
	ASSERT_EQ(1, ss.find("bcd"));
	ASSERT_EQ(4, ss.pos);
}

TEST(SS, extract)
{
	StringStream ss("abcdefghi");
	string str;
	ASSERT_TRUE(ss.extract(str, "bc", "gh"));
	ASSERT_EQ("def", str);
	ASSERT_EQ(8, ss.pos);
}

TEST(SS, extract2)
{
	StringStream ss("abcdefghi");
	string str;
	ss.find("bc");
	ASSERT_TRUE(ss.extract(str, "gh"));
	ASSERT_EQ("def", str);
	ASSERT_EQ(8, ss.pos);
}

TEST(SS, URL)
{
	StringStream ss("GET http://www.baidu.com/ HTTP/1.1\r\n");
	string url;
	ASSERT_TRUE(ss.extract(url, " ", " "));
	ASSERT_EQ("http://www.baidu.com/", url);
	ASSERT_EQ(26, ss.pos);
}
