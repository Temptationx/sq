#include <cassert>
#include "url.hpp"
#include "proxy.hpp"
#include <spdlog/spdlog.h>

Proxy::Proxy(IStore *storage) : m_storage(storage)
{
}

std::shared_ptr<CachePacket> Proxy::onRequest(std::string url)
{
	auto cache = m_storage->get(url);
	return cache;
}
