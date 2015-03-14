#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include "one.hpp"

void Sq::link()
{
	storage->loadAll();
	stream->addStreamCallback([this](const StreamID &id, shared_ptr<Request> req, shared_ptr<Response> res) {
		if (req && res) {
			storage->add(req->url, res);
		}
	});
	server->setListener([this](const string &url) -> shared_ptr < Response > {
		return proxy->onRequest(url);
	});
}

void Sq::sync()
{
	std::thread mm([this]() {
		stream->start();
	});
	std::thread ss([this]() {
		server->start();
	});
	mm.join();
	ss.join();
	stoped = 1;
}

void Sq::start()
{
	mm = std::thread([this]() {
		stream->start();
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

void Sq::enable_log()
{
	boost::log::add_file_log("log.txt");
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::severity_level::trace);
}

void Sq::stop()
{
	stream->stop();
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

