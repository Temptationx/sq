#include "one.hpp"
#include "url.hpp"
#include "utility.hpp"
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/SharedPtr.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <map>

#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>
using namespace std;
using namespace Poco;
using namespace Poco::Net;

std::map<std::string, bool> blacklist;

int wait_count = 20;

class ControlRequest : public HTTPRequestHandler
{
public:
	virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override
	{
		try
		{
			auto uri = URI(request.getURI());
			auto params = uri.getQueryParameters();
			map<string, string> query;
			for (auto p : params) {
				query.emplace(p.first, p.second);
			}
			auto command = query["c"];
			if (command == "black") {
				auto host = query["host"];
				blacklist.emplace(host, true);
			}
			else if (command == "rule") {
				auto path = query["path"];
				auto script = query["script"];
				assert(sq);
				sq->storage()->addRule(path, script);
			}
			else if (command == "max_wait"){
				auto max_wait = stoi(query["max_wait"]);
				wait_count = max_wait;
			}
			else {
				response.setStatusAndReason(HTTPResponse::HTTP_NOT_FOUND);
				response.send();
				return;
			}
			response.setStatus(HTTPResponse::HTTP_OK);
			response.send();
		}
		catch (...) {
			response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
			response.send();
		}
	}
	Sq *sq = nullptr;
private:
};

class ControlRequestFactory : public HTTPRequestHandlerFactory
{
public:
	virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) override
	{
		auto handler = new ControlRequest;
		handler->sq	 = sq;
		return handler;
	}
	Sq *sq = nullptr;
};

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
		for (auto black : blacklist) {
			if (url.find(black.first) == 0) {
				return nullptr;
			}
		}
		bool is_tengine = url.find("??") != std::string::npos;
		auto res = proxy_->onRequest(url);
		
		int i = 0;
		for (; i< wait_count && !res; i++){
			this_thread::sleep_for(chrono::seconds(1));
			res = proxy_->onRequest(url);
		}
		if (is_tengine && !res) {
			throw TengineNotCached();
		}
		spdlog::get("proxy")->info() << (res && res->response ? "[Found]" : "[!]") << " [" << i << "] " << url;
		
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
	auto request_factory = new ControlRequestFactory;
	request_factory->sq = this;
	control_server = new Poco::Net::HTTPServer(request_factory, 1023);
	control_server->start();
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
	delete control_server;
}

PersistentStorage* Sq::storage()
{
	return storage_.get();
}
