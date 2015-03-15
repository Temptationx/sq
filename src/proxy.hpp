#ifndef proxy_h__
#define proxy_h__

#include <functional>
#include <memory>
#include <mutex>

#include <lua.hpp>
#include "storage.hpp"



class Proxy
{
public:
	Proxy(IStore *storage);
	Proxy(const Proxy &proxy);
	~Proxy();
	using PreHandler = std::function<std::string(const std::string &url)>;
	using PostHandler = std::function<std::shared_ptr<Response>(const std::string &request_url, const std::string &cached_url, std::shared_ptr<Response>)>;
	std::shared_ptr<Response> onRequest(std::string url);
	void addPreRule(const std::string &path, PreHandler handler);
	void addPreRule(const std::string &path, const std::string &pre_script);
	enum class FilterType{ Ignore, Accept };
	void addPreQueryFilter(const std::string &path, FilterType filter_type, std::string list);
	void addPostRule(const std::string &path, PostHandler handler);
	void addPostRule(const std::string &path, const std::string &post_script);
private:
	lua_State * reset_lua(lua_State *L);
	std::map<std::string, PreHandler> m_pre_handlers;
	std::map<std::string, PostHandler> m_post_handlers;
	IStore *m_storage = nullptr;
	lua_State *m_L = nullptr;
	std::mutex lua_mutex;
};
#endif // proxy_h__
