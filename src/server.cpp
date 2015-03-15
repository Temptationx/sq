#include <unordered_map>
#include <thread>
#include <chrono>
#include <functional>
#include <string>
#include <mongoose.h>
#include "server.hpp"


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

IServer* ServerFactory::build(uint16_t port)
{
	return new MongooseServer(port);
}
