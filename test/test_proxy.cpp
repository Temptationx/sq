#include <gmock/gmock.h>
#include <functional>
#include <memory>
#include <map>
#include "../src/type.hpp"
#include "../src/url.hpp"
#include "../src/proxy.hpp"
#include "../src/server.hpp"
#include "mock_storage.hpp"
using namespace std;

class MockServer : public IServer
{
public:
	MOCK_METHOD0(start, void());
	MOCK_METHOD0(stop, void());
	MOCK_METHOD0(setListener, void());
};

class XYTest : public testing::Test
{
public:
	void SetUp()
	{
		res.reset(ResponseBuilder::build("Hello", {{"Content-Length", "5"}}, 200, "OK"));
		tieba_res.reset(ResponseBuilder::build("Tieba", { { "Content-Length", "5" } }, 200, "OK"));
		g_res.reset(ResponseBuilder::build("Google", { { "Content-Length", "6" } }, 200, "OK"));
	}
	shared_ptr<Response> res;
	shared_ptr<Response> tieba_res;
	shared_ptr<Response> g_res;
	MockStorage storage;
	Proxy proxy = Proxy(&storage);
};

TEST_F(XYTest, recvRequest)
{
	EXPECT_CALL(storage, get(testing::_)).WillRepeatedly(testing::Return(shared_ptr<CachePacket>()));
	ASSERT_FALSE(proxy.onRequest("http://www.baidu.com/"));
}

//TEST_F(XYTest, customHandle)
//{
//	proxy.addHandler("http://www.baidu.com/search", [](const string &url) ->shared_ptr<Response> {
//		return shared_ptr<Response>(ResponseBuilder::build("Hello", { { "Content-Length", "5" } }, 200, "OK"));
//	});
//	ASSERT_EQ(*res, *proxy.onRequest("http://www.baidu.com/search"));
//}

TEST_F(XYTest, defaultHandle)
{
	EXPECT_CALL(storage, get(testing::_)).Times(2).WillRepeatedly(testing::Invoke([this](const string &url) -> std::shared_ptr<CachePacket> {
		if (url == "http://tieba.baidu.com/") {
			return std::make_shared<CachePacket>(url, nullptr, tieba_res);
		} 
		return nullptr;
	}));
	ASSERT_EQ(nullptr, proxy.onRequest("http://www.google.com/"));
	ASSERT_EQ(*tieba_res, *proxy.onRequest("http://tieba.baidu.com/"));
}

//TEST_F(XYTest, defaultHandleWithCustomHandle1)
//{
//	EXPECT_CALL(storage, get(testing::_)).Times(2).WillRepeatedly(testing::Invoke([this](const string &url) -> std::shared_ptr<CachePacket> {
//		if (url == "http://tieba.baidu.com/") {
//			return std::make_shared<CachePacket>(url, nullptr, tieba_res);
//		}
//		return nullptr;
//	}));
//	proxy.addHandler("http://www.google.com/search", [](const string &url) -> shared_ptr<Response> {
//		return nullptr;
//	});
//	ASSERT_EQ(nullptr, proxy.onRequest("http://www.google.com/"));
//	ASSERT_EQ(*tieba_res, *proxy.onRequest("http://tieba.baidu.com/"));
//	ASSERT_EQ(nullptr, proxy.onRequest("http://www.google.com/search?q=ccc"));
//}

//TEST_F(XYTest, defaultHandleWithCustomHandle2)
//{
//	EXPECT_CALL(storage, get(testing::_)).Times(3).WillRepeatedly(testing::Invoke([this](const string &url) -> std::shared_ptr<CachePacket> {
//		if (url == "http://tieba.baidu.com/") {
//			return std::make_shared<CachePacket>(url, nullptr, tieba_res);
//		}
//		else if (url == "http://www.google.com/search?q=ccc"){
//			return std::make_shared<CachePacket>(url, nullptr, g_res);
//		}
//		return nullptr;
//	}));
//	proxy.addHandler("http://www.google.com/search", [this](const string &url) -> shared_ptr < Response > {
//		string path, query;
//		parse_url(url, path, query);
//		auto url_search = filter(url, { "q" }, optin);
//		return this->storage.get(url_search)->response;
//	});
//	ASSERT_EQ(nullptr, proxy.onRequest("http://www.google.com/"));
//	ASSERT_EQ(*tieba_res, *proxy.onRequest("http://tieba.baidu.com/"));
//	ASSERT_EQ(*g_res, *proxy.onRequest("http://www.google.com/search?q=ccc&q=uid"));
//}

//
//TEST(PROXY_LUA, UrlRule)
//{
//	MockStorage storage;
//	auto response = std::make_shared<Response>();
//	response->status = 200;
//	auto pkt = std::make_shared<CachePacket>( "http://www.baidu.com/search.html?q=123", nullptr, response);
//	EXPECT_CALL(storage, get("http://www.baidu.com/search.html?q=123")).Times(1).WillOnce(testing::Return(pkt));
//	Proxy proxy(&storage);
//	const char *url_script = R"ABC(
//function rules(request_url)
//	local q_mark = '?'
//	if is_tengine(request_url) then q_mark = q_mark .. '?' end
//	local path, query = parse_url(request_url)
//	if not query or #query == 0 then return request_url end
//	local q_m = parse_query(query)
//	q_m = accept(q_m, {'q'})
//	query = compress_query(q_m)
//	if query and #query > 0 then path = path .. q_mark end
//	path = path .. query
//	return path
//end)ABC";
//	proxy.addRule("http://www.baidu.com/search.html", url_script, "");
//	auto res = proxy.onRequest("http://www.baidu.com/search.html?q=123&t=1");
//	ASSERT_EQ(res->status, 200);
//}
//
//TEST(PROXY_LUA, BodyRule)
//{
//	MockStorage storage;
//	auto response = std::make_shared<Response>();
//	response->status = 200;
//	std::string body_str = "jsonp1";
//	response->body = std::make_shared<std::vector<uint8_t>>(body_str.data(), body_str.data() + body_str.length());
//	auto pkt = std::make_shared<CachePacket>("http://www.baidu.com/search.html?q=123&callback=jsonp1", nullptr, response);
//	EXPECT_CALL(storage, get(testing::_)).Times(1).WillOnce(testing::Return(pkt));
//	Proxy proxy(&storage);
//	const char *url_script = R"ABC(function rules(request_url)
//	local q_mark = '?'
//	if is_tengine(request_url) then q_mark = q_mark .. '?' end
//	local path, query = parse_url(request_url)
//	if not query or #query == 0 then return request_url end
//	local q_m = parse_query(query)
//	q_m = accept(q_m, {'q'})
//	query = compress_query(q_m)
//	if query and #query > 0 then path = path .. q_mark end
//	path = path .. query
//	return path
//end)ABC";
//	const char *body_script = R"ABC(function bodys(request_url, cache_url, body)
//	local request_path, request_query = parse_url(request_url)
//	request_query = parse_query(request_query)
//	local cache_path, cache_query = parse_url(cache_url)
//	cache_query = parse_query(cache_query)
//	local request_jsonp = find_value(request_query, 'callback')
//	local cache_jsonp = find_value(cache_query, 'callback')
//	body = body:gsub(cache_jsonp, request_jsonp)
//	return body
//end)ABC";
//	proxy.addRule("http://www.baidu.com/search.html", url_script, body_script);
//	auto res = proxy.onRequest("http://www.baidu.com/search.html?q=123&callback=jsonp2");
//	ASSERT_EQ(res->status, 200);
//	std::string modified_body_str = std::string(res->body->data(), res->body->data() + res->body->size());
//	ASSERT_EQ(modified_body_str, "jsonp2");
//}
