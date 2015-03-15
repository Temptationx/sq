#include <gmock/gmock.h>
#include "../src/utility.hpp"
#include "../src/storage.hpp"
#include "../src/url.hpp"
#include "mock_storage.hpp"
using namespace std;

TEST(TESTURLStorage, addget)
{
	URLStorage storage;
	auto cached_pkt = make_shared<CachePacket>();
	storage.add("http://www.baidu.com/search?q=1", cached_pkt);
	auto pkt = storage.get("http://www.baidu.com/search?q=2");
	ASSERT_FALSE(pkt->response);

	pkt = storage.get("http://www.baidu.com/search?q=1");

	ASSERT_TRUE(pkt->response);

	storage.add("http://www.baidu.com/search?q=1&u=a&t=9", cached_pkt);
	pkt = storage.get("http://www.baidu.com/search?q=1&u=a");
	ASSERT_TRUE(pkt->response);
}

class MockIO : public IO
{
public:
	MOCK_METHOD4(write, bool(const std::string & dir, const std::string &filename, size_t size, const char *data));
	MOCK_METHOD1(read, std::pair<size_t, const char*>(const std::string &filename));
	MOCK_METHOD1(getAll, std::vector<std::string>(const std::string &dir));
};

class TESTFileStorage : public testing::Test
{
public:
	void SetUp()
	{
		cached_pkt = make_shared<CachePacket>(url, 
										nullptr, 
										shared_ptr<Response>(ResponseBuilder::build("Hello", { { "Content-Length", "5" } }, 200, "OK")));
	}
	string response_rawdata = "GET http://www.baidu.com/ HTTP/1.1\r\n\r\nHTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
	string url = "http://www.baidu.com/";
	shared_ptr<CachePacket> cached_pkt;
	char *b = (char*)response_rawdata.data();
	size_t s = response_rawdata.size();
	string dir = "1024";
	string url_md5 = md5(url.data(), url.size());
#ifdef _WIN32
	string path_name = string("1024\\") + url_md5;
#else
	string path_name = string("1024/") + url_md5;
#endif
};

MATCHER_P2(IsEQ, d, n, ""){ return std::string(arg, n) == std::string(d, n); };

TEST_F(TESTFileStorage, add)
{

	MockIO *mio = new MockIO;
	EXPECT_CALL(*mio, write(dir, url_md5, s, IsEQ(b, s))).Times(1).WillOnce(testing::Return(true));
	auto file_storage = FileStorageFactory<MockIO>::build(dir, mio);

	file_storage->add(url, cached_pkt);
}

TEST_F(TESTFileStorage, get)
{
	MockIO *mio = new MockIO;
	EXPECT_CALL(*mio, read(path_name)).Times(1).WillOnce(testing::Return(make_pair(s, b)));
	auto file_storage = FileStorageFactory<MockIO>::build(dir, mio);

	auto pkt = file_storage->get(url_md5);
	ASSERT_EQ(url, pkt->url);
	ASSERT_EQ(200, pkt->response->status);
	ASSERT_EQ("OK", pkt->response->status_text);
}

TEST_F(TESTFileStorage, loadAll)
{
	// loadAll will call io.getAll then repeat call{ read and parse add}
	MockIO *mio = new MockIO;
	MockStorage cache_storage;
	EXPECT_CALL(*mio, getAll(dir)).Times(1).WillOnce(testing::Return(vector < string > {url_md5}));
	EXPECT_CALL(*mio, read(path_name)).Times(1).WillOnce(testing::Return(make_pair(s, b)));
	//EXPECT_CALL(cache_storage, add(url, testing::Pointee(*response))).Times(1);

	auto file_storage = FileStorageFactory<MockIO>::build(dir, mio);
	file_storage->loadAll(cache_storage);
}