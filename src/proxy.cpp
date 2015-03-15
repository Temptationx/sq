#include <cassert>
#include "url.hpp"
#include "proxy.hpp"


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
)ABCD";
}

Proxy::Proxy(IStore *storage) : m_storage(storage)
{
	defaultHandler = std::bind(&Proxy::searchCache, this, std::placeholders::_1);
}

std::shared_ptr<Response> Proxy::searchCache(const std::string &url)
{
	auto res = m_storage->get(url);
	return res->response;
}

Proxy::Handler& Proxy::getHandler(const std::string &path)
{
	auto h = m_handlers.find(path);
	if (h != m_handlers.end()) {
		return h->second;
	}
	return defaultHandler;
}

void Proxy::addRule(const std::string &path, Handler handler)
{
	m_handlers[path] = handler;
}

std::string do_rules(lua_State *L, const char *url)
{
	lua_getglobal(L, "rules");
	lua_pushstring(L, url);
	if (lua_pcall(L, 1, 1, 0)) {
		printf("error %s\n", lua_tostring(L, -1));
	}
	if (!lua_isstring(L, -1)) {
		printf("error rules must return a string");
	}
	std::string url_r = lua_tostring(L, -1);
	return url_r;
}

std::shared_ptr<Response> copy_response(Response *res)
{
	auto rres = std::make_shared<Response>();
	rres->body = res->body;
	rres->headers = res->headers;
	rres->status = res->status;
	rres->status_text = res->status_text;
	return rres;
}

std::shared_ptr<std::vector<uint8_t>> do_bodys(lua_State *L, const std::string &request_url, const std::string &cache_url, const char *data, int size)
{
	lua_getglobal(L, "bodys");
	lua_pushstring(L, request_url.data());
	lua_pushstring(L, cache_url.data());
	lua_pushlstring(L, data, size);
	if (lua_pcall(L, 3, 1, 0))
	{
		printf("error %s\n", lua_tostring(L, -1));
	}
	if (!lua_isstring(L, -1))
	{
		printf("error bodys must return a string");
	}
	size_t len = 0;
	auto d = lua_tolstring(L, -1, &len);
	auto body = std::make_shared<std::vector<uint8_t>>(d, d+len);
	return body;
}

void Proxy::addRule(const std::string &path, const std::string &rules_script, const std::string &body_script)
{
	auto handler = [this, rules_script, body_script](const std::string &request_url)->std::shared_ptr<Response> {
		lua_State *L = nullptr;
		L = luaL_newstate();
		luaL_openlibs(L);
		luaL_loadstring(L, LUAScript::lib);
		auto e = lua_pcall(L, 0, 0, 0);
		assert(!e);

		luaL_loadstring(L, rules_script.data());
		if (lua_pcall(L, 0, 0, 0)) {
			printf("error %s\n", lua_tostring(L, -1));
		}

		luaL_loadstring(L, body_script.data());
		if (lua_pcall(L, 0, 0, 0)) {
			printf("error %s\n", lua_tostring(L, -1));
		}
		auto modified_url = request_url;
		if (!rules_script.empty()) {
			modified_url = do_rules(L, request_url.data());
		}
		auto pkt = m_storage->get(modified_url);
		auto response = pkt->response;
		if (!body_script.empty()) {
			response = copy_response(response.get());
			response->body = do_bodys(L, request_url, pkt->url, (char *)response->body->data(), response->body->size());
		}
		lua_close(L);
		return response;
	};
	addRule(path, handler);
}

std::shared_ptr<Response> Proxy::onRequest(const std::string &url)
{
	std::string path, query;
	parse_url(url, path, query);
	auto &handler = getHandler(path);
	if (handler) {
		return handler(url);
	}
	return nullptr;
}
