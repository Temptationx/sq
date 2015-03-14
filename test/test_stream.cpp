#include <gmock/gmock.h>
#include <map>
#include <memory>
#include <vector>
#include "../src/type.hpp"
#include "../src/stream.hpp"

using namespace std;

/*
Requirement:
recv data
use correct parser.
check complete and build request and response 
call consumer.
close
*/

namespace
{
	class Test
	{
	public:
		virtual void cb_call(const StreamID &id) = 0;
		virtual void req(string ) = 0;
		virtual void res(int) = 0;
	};

	class MockTest : public Test
	{
	public:
		MOCK_METHOD1(cb_call, void(const StreamID &));
		MOCK_METHOD1(req, void(string));
		MOCK_METHOD1(res, void(int));
	};
}

class StreamTest : public testing::Test
{
public:
	void SetUp()
	{
		std::function<void(const StreamID&, std::shared_ptr<Request>, std::shared_ptr<Response>)> fun = [this](const StreamID& id, std::shared_ptr<Request> req, std::shared_ptr<Response> res) {
			test.cb_call(id);
			if (req && !res) {
				test.req(req->url);
			}
			if (res) {
				test.res(res->status);
			}
		};
		stream.addStreamCallback(fun);
	}
	Stream stream;
	MockTest test;
};

TEST_F(StreamTest, Request)
{
	EXPECT_CALL(test, cb_call(StreamID(1))).Times(1);
	EXPECT_CALL(test, req(string("http://www.baidu.com/"))).Times(1);
	string req_str1("GET http://www.baidu.com/ HTTP/1.1\r\nHost: www.baidu.com\r\n\r\n");
	stream.onCall(1, req_str1.data(), req_str1.size(), CallReasonRequest);
}

TEST_F(StreamTest, Response)
{
	EXPECT_CALL(test, cb_call(StreamID(2))).Times(1);
	EXPECT_CALL(test, res(200)).Times(1);
	string res_str1("HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello");
	stream.onCall(2, res_str1.data(), res_str1.size(), CallReasonResponse);
}

TEST_F(StreamTest, RequestThenResponse)
{
	EXPECT_CALL(test, cb_call(StreamID(1))).Times(2);
	EXPECT_CALL(test, req(string("http://www.baidu.com/"))).Times(1);
	EXPECT_CALL(test, res(200)).Times(1);
	string req_str1("GET http://www.baidu.com/ HTTP/1.1\r\nHost: www.baidu.com\r\n\r\n");
	stream.onCall(1, req_str1.data(), req_str1.size(), CallReasonRequest);

	string res_str1("HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello");
	stream.onCall(1, res_str1.data(), res_str1.size(), CallReasonResponse);
}

TEST_F(StreamTest, RequestTwice)
{
	testing::InSequence s;
	EXPECT_CALL(test, cb_call(StreamID(1)));
	EXPECT_CALL(test, req(string("http://www.baidu.com/")));
	EXPECT_CALL(test, cb_call(StreamID(1)));
	EXPECT_CALL(test, req(string("http://www.google.com/")));
	string req_str1("GET http://www.baidu.com/ HTTP/1.1\r\nHost: www.baidu.com\r\n\r\n");
	stream.onCall(1, req_str1.data(), req_str1.size(), CallReasonRequest);
	string req_str2("GET http://www.google.com/ HTTP/1.1\r\nHost: www.google.com\r\n\r\n");
	stream.onCall(1, req_str2.data(), req_str2.size(), CallReasonRequest);
}

TEST_F(StreamTest, ResponseTwice)
{
	testing::InSequence s;
	EXPECT_CALL(test, cb_call(StreamID(1)));
	EXPECT_CALL(test, res(200));
	EXPECT_CALL(test, cb_call(StreamID(1)));
	EXPECT_CALL(test, res(302));

	string res_str1("HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello");
	stream.onCall(1, res_str1.data(), res_str1.size(), CallReasonResponse);

	string res_str2("HTTP/1.1 302 Found\r\nLocation: http://www.baidu.com\r\nContent-Length: 5\r\n\r\nHello");
	stream.onCall(1, res_str2.data(), res_str2.size(), CallReasonResponse);
}
