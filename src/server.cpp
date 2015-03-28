#include <unordered_map>
#include <thread>
#include <chrono>
#include <functional>
#include <string>
#include <mongoose.h>
#include "server.hpp"
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/SharedPtr.h>
#include <map>
#include <cassert>
using namespace std;
using namespace Poco;
using namespace Poco::Net;

class MongooseServer : public IServer
{
public:
	MongooseServer(uint16_t port)
	{
		m_server = mg_create_server(this, MongooseServer::MgCall);
		mg_set_option(m_server, "listening_port", std::to_string(port).data());
	}
	virtual void start() override
	{
		while (ss)
		{
			mg_poll_server(m_server, 10);
		}
		y = 1;
	}
	virtual void stop() override
	{
		ss = 0;
		while (!y){
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}
	}
	virtual void setListener(std::function<std::shared_ptr<Response>(const std::string &)> listener) override
	{
		m_listener = listener;
	}
	virtual ~MongooseServer()
	{
		if (m_server) {
			mg_destroy_server(&m_server);
			m_server = nullptr;
		}
	}
private:
	int ss = 1;
	int y = 0;
	int mg_request(mg_connection *conn)
	{
		auto url = linkUrl(conn);
		std::shared_ptr<Response> response;

		if (m_listener) {
			response = m_listener(url);
		}
		if (!response) {
			return MG_FALSE;
		}
		mg_send_status(conn, response->status);
		if (response->headers) {
			for (auto head : *response->headers) {
				mg_send_header(conn, head.first.data(), head.second.data());
			}
		}
		if (response->body) {
			mg_send_data(conn, response->body->data(), response->body->size());
		}
		return MG_TRUE;
	}
	static int MgCall(mg_connection *conn, mg_event ev)
	{
		switch (ev)
		{
			case MG_POLL:
				break;
			case MG_CONNECT:
				break;
			case MG_AUTH:
				return MG_TRUE;
				break;
			case MG_REQUEST:
			{
				if (std::string("CONNECT") == conn->request_method) {
					return MG_FALSE;
				}
				auto _this = (MongooseServer*)conn->server_param;
				return _this->mg_request(conn);
			}
				break;
			case MG_REPLY:
				break;
			case MG_RECV:
				break;
			case MG_CLOSE:
				break;
			case MG_WS_HANDSHAKE:
				break;
			case MG_WS_CONNECT:
				break;
			case MG_HTTP_ERROR:
				break;
			default:
				break;
		}
		return MG_FALSE;
	}
	static std::string linkUrl(mg_connection * conn)
	{
		std::string url = conn->uri;
		url.c_str();
		if (conn->query_string) {
			url += std::string("?") + conn->query_string;
		}
		return url;
	}
	std::function<std::shared_ptr<Response>(const std::string &)> m_listener;
	mg_server* m_server = nullptr;
};

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
