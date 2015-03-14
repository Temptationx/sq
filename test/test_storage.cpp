#include <gmock/gmock.h>
#include "../src/storage.hpp"
#include "../src/url.hpp"
#include "mock_storage.hpp"
using namespace std;

TEST(TESTURLStorage, addget)
{
	URLStorage storage;
	auto res = make_shared<Response>();
	storage.add("http://www.baidu.com/search?q=1", res);
	auto res2 = storage.get("http://www.baidu.com/search?q=2");
	ASSERT_FALSE(res2.first);

	res2 = storage.get("http://www.baidu.com/search?q=1");

	ASSERT_TRUE(res2.first);

	storage.add("http://www.baidu.com/search?q=1&u=a&t=9", res);
	res2 = storage.get("http://www.baidu.com/search?q=1&u=a");
	ASSERT_TRUE(res2.first);
}

class MockIO : public IO
{
public:
	MOCK_METHOD4(write, bool(const std::string & dir, const std::string &filename, const char *data, size_t size));
	MOCK_METHOD1(read, std::pair<size_t, const char*>(const std::string &filename));
	MOCK_METHOD1(getAll, std::vector<std::string>(const std::string &dir));
};

class TESTFileStorage : public testing::Test
{
public:
	void SetUp()
	{
		res.reset(ResponseBuilder::build("Hello", {{"Content-Length", "5"}}, 200, "OK"));
	}
	string response_rawdata = "GET http://www.baidu.com/ HTTP/1.1\r\n\r\nHTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
	string url = "http://www.baidu.com/";
	shared_ptr<Response> res;
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
	EXPECT_CALL(*mio, write(dir, url_md5, IsEQ(b, s), s)).Times(1).WillOnce(testing::Return(true));
	auto file_storage = FileStorageFactory<MockIO>::build(dir, mio);

	file_storage->add(url, res);
}

TEST_F(TESTFileStorage, get)
{
	MockIO *mio = new MockIO;
	EXPECT_CALL(*mio, read(path_name)).Times(1).WillOnce(testing::Return(make_pair(s, b)));
	auto file_storage = FileStorageFactory<MockIO>::build(dir, mio);

	auto pres = file_storage->get(url_md5);
	ASSERT_EQ(url, pres.second);
	ASSERT_EQ(200, pres.first->status);
	ASSERT_EQ("OK", pres.first->status_str);
}

TEST_F(TESTFileStorage, loadAll)
{
	// loadAll will call io.getAll then repeat call{ read and parse add}
	MockIO *mio = new MockIO;
	MockStorage cache_storage;
	EXPECT_CALL(*mio, getAll(dir)).Times(1).WillOnce(testing::Return(vector < string > {url_md5}));
	EXPECT_CALL(*mio, read(path_name)).Times(1).WillOnce(testing::Return(make_pair(s, b)));
	EXPECT_CALL(cache_storage, add(url, testing::Pointee(*res))).Times(1);

	auto file_storage = FileStorageFactory<MockIO>::build(dir, mio);
	file_storage->loadAll(cache_storage);
}