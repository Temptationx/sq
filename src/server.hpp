#ifndef server_h__
#define server_h__
#include <cstdint>
#include <functional>
#include <memory>
#include "type.hpp"

class IServer
{
public:
	virtual ~IServer(){}
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void setListener(std::function<std::shared_ptr<Response>(const std::string &)>) = 0;
};

class ServerFactory
{
public:
	static IServer* build(uint16_t port);
	ServerFactory() = delete;
};

#endif // server_h__
