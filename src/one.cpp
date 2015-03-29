#include "one.hpp"
#include "url.hpp"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>
using namespace std;


std::map<std::string, bool> blacklist{
{"gm.mmstat.com", true},
{"ga.mmstat.com", true},
{ "log.mmstat.com", true },
{ "ac.mmstat.com", true },
{ "amos.alicdn.com", true },
{ "q5.cnzz.com", true }, 
{ "ac.atpanel.com", true }, 
{ "count.tbcdn.cn", true },
{"hotclick.app.linezing.com", true},
{"cnzz.mmstat.com", true},
{"amos.im.alisoft.com", true},
{"cookiemapping.wrating.com", true}};

void Sq::link()
{
	storage_->loadAll();
	stream->addStreamCallback([this](const StreamID &id, shared_ptr<CachePacket> pkt) {
		if (pkt->request && pkt->response) {
			spdlog::get("cache")->info() << pkt->url;
			storage_->add(pkt->url, pkt);
		}
	});
	server->setListener([this](const string &url) -> shared_ptr < Response > {
		std::string host = get_host(url);
		if (blacklist.find(host) != blacklist.end()) {
			return nullptr;
		}

		auto res = proxy_->onRequest(url);
		for (auto i=0; i< 30 && !res;i++){
			this_thread::sleep_for(chrono::seconds(1));
			res = proxy_->onRequest(url);
		}
		spdlog::get("proxy")->info() << (res && res->response ? "[Found]" : "[!]") << " " << url;
		
		return (res && res->response) ? res->response : nullptr;
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
	storage_ = move(PersistentStorageFactory::build(dir));
	stream = std::make_unique<SnifferStream>(inter);
	proxy_ = std::make_unique<Proxy>(storage_.get());
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

PersistentStorage* Sq::storage()
{
	return storage_.get();
}
