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
#include <sstream>
#include <spdlog/spdlog.h>
#include <cassert>
#include "utility.hpp"
#include "server.hpp"


using namespace std;
using namespace Poco;
using namespace Poco::Net;

class PocoServer;

class RequestHandler : public HTTPRequestHandler
{
public:
	RequestHandler(PocoServer *server);
	virtual void handleRequest(HTTPServerRequest &request,
		HTTPServerResponse &response) override;
private:
	PocoServer *server_;
};

class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
	RequestHandlerFactory(PocoServer *server);
	virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) override;
private:
	PocoServer *server_;
};

class PocoServer : public IServer
{
public:
	PocoServer(uint16_t port);
	virtual ~PocoServer() = default;
	virtual void start() override;
	virtual void stop() override;
	virtual void setListener(std::function<std::shared_ptr<Response>(const std::string &) > listener) override;
private:
	unique_ptr<HTTPServer> m_server;
	std::function<std::shared_ptr<Response>(const std::string &)> m_listener;
	friend RequestHandler;
};

void PocoServer::setListener(std::function<std::shared_ptr<Response>(const std::string &) > listener)
{
	m_listener = listener;
}

void PocoServer::stop()
{
	m_server->stop();
}

void PocoServer::start()
{
	m_server->start();
}

PocoServer::PocoServer(uint16_t port)
{
	m_server = make_unique<HTTPServer>(new RequestHandlerFactory(this), port);
}

void RequestHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
	try{
		try{
			auto url = request.getURI();
			auto uri = URI(url);

			if (uri.getScheme() != "http"){
				response.setStatusAndReason(HTTPResponse::HTTP_UNSUPPORTEDMEDIATYPE);
				response.send();
				return;
			}
			// Work here
			assert(server_->m_listener);
			auto response_ = server_->m_listener(url);
			if (!response_) {
				response.setStatusAndReason(HTTPResponse::HTTP_NOT_FOUND);
				response.send();
				return;
			}
			response.setStatusAndReason(HTTPResponse::HTTPStatus(response_->status));
			if (response_->headers) {
				for (auto p : *response_->headers) {
					response.set(p.first, p.second);
				}
			}
			if (response_->body){
				response.sendBuffer(response_->body->data(), response_->body->size());
			}
			if (!response.sent()) {
				response.send();
			}
			return;
		}
		catch (TengineNotCached e){
			spdlog::get("proxy")->error() << "[TENGINE] " << request.getURI();
			URI uri(request.getURI());
			if (uri.getScheme() != "http") {
				return;
			}
			auto path = URI(request.getURI()).getPathAndQuery();
			if (path.empty()){
				path = "/";
			}
			HTTPClientSession session(uri.getHost(), uri.getPort());
			HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
			HTTPResponse res;
			session.sendRequest(req);
			auto &rs = session.receiveResponse(res);
			std::cout << res.getStatus() << " " << res.getReason() << std::endl;
			std::stringstream ss;
			StreamCopier::copyStream(rs, ss);
			response.setStatusAndReason(res.getStatus());
			for (auto it = res.begin(); it != res.end(); it++) {
				response.set(it->first, it->second);
				cout << it->first << ": " << it->second << endl;
			}
			auto s = ss.str();
			response.sendBuffer(s.data(), s.size());
		}
	}
	catch (Poco::Exception e){
		spdlog::get("proxy")->error() << "[POCO] [Exception] "<<e.displayText();
	}
}

RequestHandler::RequestHandler(PocoServer *server) : server_(server)
{
}

HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
	return new RequestHandler(server_);
}

RequestHandlerFactory::RequestHandlerFactory(PocoServer *server) : server_(server)
{
}

IServer* ServerFactory::build(uint16_t port)
{
	return new PocoServer(port);
}
