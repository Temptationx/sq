#include "one.hpp"
#include <spdlog/spdlog.h>

void Sq::link()
{
	storage->loadAll();
	stream->addStreamCallback([this](const StreamID &id, shared_ptr<Request> req, shared_ptr<Response> res) {
		if (req && res) {
			spdlog::get("log")->info() << "[Cache] " <<req->url;
			storage->add(req->url, res);
		}
	});
	server->setListener([this](const string &url) -> shared_ptr < Response > {
		
		auto res = proxy->onRequest(url);
		spdlog::get("log")->info() << "[Server] " << (res ? "[Found]" : "[!]") << " " << url;
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
	proxy = std::make_unique<Proxy>(storage.get());
	server.reset(ServerFactory::build(proxy_server_port));
	link();
}

void Sq::enable_log(const std::string &log_filename)
{
	auto log = spdlog::rotating_logger_mt("log", log_filename, 1024 * 1024 * 10, 1, true);
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

void Sq::add_rule(const std::string &path, const std::string &rules_script, const std::string &body_script)
{
	proxy->addHandler(path, rules_script, body_script);
}
