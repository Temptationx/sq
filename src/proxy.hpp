#ifndef proxy_h__
#define proxy_h__

#include <functional>
#include <memory>
#include <mutex>
#include <list>
#include <lua.hpp>
#include "storage.hpp"


class Proxy
{
public:
	Proxy(IStore *storage);
	Proxy(const Proxy &) = delete;
	~Proxy() = default;
	std::shared_ptr<Response> onRequest(std::string url);
private:
	IStore *m_storage = nullptr;
};

#endif // proxy_h__
