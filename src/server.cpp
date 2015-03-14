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
	static int MgCall(mg_connection *conn, mg_event ev)
	{
		if (ev == MG_AUTH) {
			return MG_TRUE;
		}
		else if (ev == MG_REQUEST){
			if (std::string("CONNECT") == conn->request_method) {
				return MG_FALSE;
			}
			auto _this = (MongooseServer*)conn->server_param;
			auto url = linkUrl(conn);
			std::shared_ptr<Response> res;

			if (_this->m_listener) {
				res = _this->m_listener(url);
			}
			if (!res) {
				return MG_FALSE;
			}
			mg_send_status(conn, res->status);
			if (res->headers) {
				for (auto head : *res->headers)
				{
					mg_send_header(conn, head.first.data(), head.second.data());
				}
			}
			if (res->body) {
				mg_send_data(conn, res->body->data(), res->body->size());
			}
			return MG_TRUE;
		}
		else{
			return MG_FALSE;
		}
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
