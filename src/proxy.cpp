#include <cassert>
#include "url.hpp"
#include "proxy.hpp"
#include <spdlog/spdlog.h>

namespace LUAScript{
	const char *lib = R"ABCD(function parse_tengine_url(url)
	local double_q_pos = url:find('%?%?')
	if not double_q_pos then
		return url, nil
	end	
	local last_qi = url:find('%?', double_q_pos + 2)
	last_qi = last_qi or 0
	local last_ai = url:find('&', double_q_pos + 2)
	last_ai = last_ai or 0
	function max(a, b)
		if a > b then
			return a
		else
			return b
		end
	end
	function min(a, b)
		if a < b then
			return a
		else
			return b
		end
	end
	local end_i = min(last_ai, last_qi)
	if end_i < 1 then
		end_i = max(last_ai, last_qi)
	end
	if end_i < 1 then
		end_i = url:len()
	end
	return url:sub(0, double_q_pos - 1), url:sub(double_q_pos + 2, end_i - 1)
end

function parse_url(url)
	local double_q_p = url:find('%?%?')
	if double_q_p then
		return parse_tengine_url(url)
	end
	
	local q_p = url:find('%?')
	if not q_p then
		return url, nil
	end
	return url:sub(0, q_p - 1), url:sub(q_p + 1)
end

function split(s, delimiter)
    result = {};
    for match in (s..delimiter):gmatch("(.-)"..delimiter) do
        table.insert(result, match);
    end
    return result;
end

function print_table(t)
	for k, v in pairs(t) do
		if type(v) == 'table' then
			print_table(v)
		else
			print(k, v)
		end
	end
end

function parse_query(q)
	local x = split(q, '&')

	local y = {}
	for k, v in ipairs(x) do 
		local z = split(v, '=')
		y[#y + 1] = {z[1], z[2]}
	end
	return y
end

function compress_query(q_m)
	q = ''
    for k, v in pairs(q_m) do
		if #v == 2 then
	    	q = q .. v[1] .. '=' .. v[2] .. '&'
		else
			q = q .. v[1] .. '&'
		end
	end
	return q:sub(0, #q - 1)
end

function find(l, f)
	for k, v in pairs(l) do
		if v == f then return true end
	end
	return false
end

function find_value(m, key)
	for k, v in pairs(m) do
		if v[1] == key then return v[2] end
	end
end

function accept(q_m, l)
	local o = {}
	for k, v in pairs(q_m) do
		if find(l, v[1]) then o[#o + 1] = v end
	end
	return o
end

function ignore(q_m, l)
	local o = {}
	for k, v in pairs(q_m) do
		if not find(l, v[1]) then o[#o + 1] = v end 
	end
	return o
end

function is_tengine(url)
	if url:find('%?%?') then return true else return false end
end

function url_rules_core(request_url, is_accept_list, l)
	if is_accept_list == nil then is_accept_list = true end
	local q_mark = '?'
	if is_tengine(request_url) then q_mark = q_mark .. '?' end
	local path, query = parse_url(request_url)
	if not query or #query == 0 then return request_url end
	local q_m = parse_query(query)
	if is_accept_list then
		q_m = accept(q_m, l)
	else
		q_m = ignore(q_m, l)
	end
	query = compress_query(q_m)
	if query and #query > 0 then path = path .. q_mark end
	path = path .. query
	return path
end

function replace_with_value(request_url, cache_url, body, l)
	local request_path, request_query = parse_url(request_url)
	request_query = parse_query(request_query)
	local cache_path, cache_query = parse_url(cache_url)
	cache_query = parse_query(cache_query)
	local request_jsonp = find_value(request_query, l)
	local cache_jsonp = find_value(cache_query, l)
	body = body:gsub(cache_jsonp, request_jsonp)
	return body
end
)ABCD";
}

Proxy::Proxy(IStore *storage) : m_storage(storage)
{
	m_L = reset_lua(nullptr);
}

Proxy::Proxy(const Proxy &proxy)
{

}

std::shared_ptr<Response> Proxy::onRequest(std::string url)
{
	std::unique_lock<std::mutex> lua_lk(lua_mutex);

	std::string path, query;
	parse_url(url, path, query);
	// Pre
	auto it = m_pre_handlers.find(path);
	std::string modified_url;
	if (it != m_pre_handlers.end()) {
		modified_url = it->second(url);
	}
	else {
		modified_url = url;
	}

	// lookup
	auto cache = m_storage->get(modified_url);

	// post
	auto it2 = m_post_handlers.find(path);
	if (it2 != m_post_handlers.end()) {
		return it2->second(url, cache->url, cache->response);
	}
	if (cache && cache->response) {
		return cache->response;
	}
	return nullptr;
}

void Proxy::addPreRule(const std::string &path, PreHandler handler)
{
	m_pre_handlers[path] = handler;
}

std::string pre_rule(lua_State *L, const std::string &url)
{
	lua_getglobal(L, "pre_rule");
	lua_pushstring(L, url.data());
	if (lua_pcall(L, 1, 1, 0)) {
		printf("error %s\n", lua_tostring(L, -1));
		throw 1;
	}
	if (!lua_isstring(L, -1)) {
		printf("error rules must return a string");
		throw 2;
	}
	std::string url_r = lua_tostring(L, -1);
	return url_r;
}

void Proxy::addPreRule(const std::string &path, const std::string &pre_script)
{
	if (pre_script.empty()) {
		return;
	}
	auto handle = [this, pre_script](const std::string &url) -> std::string {
		luaL_loadstring(m_L, pre_script.data());
		if (lua_pcall(m_L, 0, 0, 0)) {
			printf("error %s\n", lua_tostring(m_L, -1));
			throw 1;
		}
		return pre_rule(m_L, url);
	};
	addPreRule(path, handle);
}

void Proxy::addPostRule(const std::string &path, PostHandler handler)
{
	m_post_handlers[path] = handler;
}

std::shared_ptr<Response> post_rule(lua_State *L, const std::string &request_url, const std::string &cached_url, std::shared_ptr<Response> response)
{
	if (!response) {
		return nullptr;
	}
	lua_getglobal(L, "post_rule");
	lua_pushstring(L, request_url.data());
	lua_pushstring(L, cached_url.data());
	// push body
	if(response && response->body) {
		lua_pushlstring(L, (const char*)response->body->data(), response->body->size());
	}
	else{
		lua_pushstring(L, "");
	}
	// call function
	if (lua_pcall(L, 3, 1, 0)) {
		printf("error %s\n", lua_tostring(L, -1));
		throw 1;
	}
	if (!lua_isstring(L, -1)) {
		printf("error bodys must return a string");
		throw 2;
	}
	// copy body
	response = std::make_shared<Response>(*response);
	size_t len = 0;
	auto data = lua_tolstring(L, -1, &len);
	response->body = std::make_shared<std::vector<uint8_t>>(data, data + len);
	return response;
}

void Proxy::addPostRule(const std::string &path, const std::string &post_script)
{
	if (post_script.empty()) {
		return;
	}
	auto handle = [this, post_script](const std::string &request_url, const std::string &cached_url, std::shared_ptr<Response> response) -> std::shared_ptr<Response> {
		luaL_loadstring(m_L, post_script.data());
		if (lua_pcall(m_L, 0, 0, 0)) {
			printf("error %s\n", lua_tostring(m_L, -1));
			throw 1;
		}
		return post_rule(m_L, request_url, cached_url, response);
	};
	addPostRule(path, handle);
}

void Proxy::addPreQueryFilter(const std::string &path, FilterType filter_type, std::string list)
{
	auto url_script_ = R"ABC(function pre_rule(request_url)
	return url_rules_core(request_url, {}, {})
end)ABC";

	fmt::MemoryWriter w;
	w.write(url_script_, (filter_type == FilterType::Accept ? "true" : "false"), list);
	addPreRule(path, w.str());
}

lua_State * Proxy::reset_lua(lua_State *L)
{
	std::unique_lock<std::mutex> lua_lk(lua_mutex);
	if (L) {
		lua_close(L);
	}
	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_loadstring(L, LUAScript::lib);
	auto e = lua_pcall(L, 0, 0, 0);
	assert(!e);
	return L;
}

Proxy::~Proxy()
{
	std::unique_lock<std::mutex> lua_lk(lua_mutex);
	if (m_L) {
		lua_close(m_L);
	}
}
