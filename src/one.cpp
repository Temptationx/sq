#include "one.hpp"
#include <spdlog/spdlog.h>

void Sq::link()
{
	storage->loadAll();
	stream->addStreamCallback([this](const StreamID &id, shared_ptr<CachePacket> pkt) {
		if (pkt->request && pkt->response) {
			spdlog::get("cache")->info() << pkt->url;
			storage->add(pkt->url, pkt);
		}
	});
	server->setListener([this](const string &url) -> shared_ptr < Response > {
		if (url.find("hotclick.app") != std::string::npos)
		{
			return nullptr;
		}
		auto res = proxy_->onRequest(url);
		spdlog::get("proxy")->info() << (res ? "[Found]" : "[!]") << " " << url;
		return res;
	});
}

void Sq::sync()
{
	mm.join();
	ss.join();
	stoped = 1;
}

void Sq::start()
{
	mm = std::thread([this]() {
		stream->startSniff();
	});
	ss = std::thread([this]() {
		server->start();
	});
}

Sq::Sq(const std::string dir, int inter, int proxy_server_port)
{
	storage = move(PersistentStorageFactory::build(dir));
	stream = std::make_unique<SnifferStream>(inter);
	proxy_ = std::make_unique<Proxy>(storage.get());
	server.reset(ServerFactory::build(proxy_server_port));
	link();
}

void Sq::enable_log(const std::string &log_filename)
{
	auto log = spdlog::rotating_logger_mt("cache", log_filename + "_cache", 1024 * 1024 * 10, 1, true);
	log = spdlog::rotating_logger_mt("proxy", log_filename + "_proxy", 1024 * 1024 * 10, 1, true);
}

void Sq::stop()
{
	stream->stopSniff();
	server->stop();
	mm.join();
	ss.join();
	stoped = 1;
}

Sq::~Sq()
{
	if (!stoped) {
		stop();
	}
}

Proxy* Sq::proxy()
{
	return proxy_.get();
}
