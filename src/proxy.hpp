#ifndef proxy_h__
#define proxy_h__

#include <functional>
#include <memory>
#include "storage.hpp"
#include <lua.hpp>

class Proxy
{
public:
	Proxy(IStorage *storage);
	typedef std::function<std::shared_ptr<Response>(const std::string &url)> Handler;
	std::shared_ptr<Response> onRequest(const std::string &url);
	void addHandler(const std::string &path, Handler handler);
	void addHandler(const std::string &path, const std::string &rules_script, const std::string &body_script);
private:
	Handler& getHandler(const std::string &path);
	std::shared_ptr<Response> searchCache(const std::string &url);
	Handler defaultHandler;
	std::map<std::string, Handler> m_handlers;
	IStorage *m_storage = nullptr;
};
#endif // proxy_h__
