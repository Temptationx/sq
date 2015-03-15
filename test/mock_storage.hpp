#ifndef mock_storage_h__
#define mock_storage_h__
#include <gmock/gmock.h>
#include "../src/storage.hpp"
class MockStorage : public IStore
{
public:
	MOCK_METHOD2(add, void(const std::string &url, std::shared_ptr<CachePacket> data));
	MOCK_METHOD1(get, std::shared_ptr<CachePacket>(const std::string &filename));
};
#endif // mock_storage_h__
