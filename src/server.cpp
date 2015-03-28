#include <cassert>
#include "server.hpp"
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/SharedPtr.h>
#include <Poco/URI.h>


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
	auto url = request.getURI();
	auto uri = URI(url);
	if (uri.getScheme() == "https"){
		response.setStatusAndReason(HTTPResponse::HTTP_UNSUPPORTEDMEDIATYPE);
		response.send();
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
