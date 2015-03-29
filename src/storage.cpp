#include <fstream>
#include <boost/filesystem.hpp>
#include "serialization.hpp"
#include "storage.hpp"
#include "stream.hpp"
#include "utility.hpp"
#include "url.hpp"

using std::shared_ptr;

// Lua declare
void add_rule(lua_State *L, const std::string &path, const std::string &script);
void on_cache(lua_State *L, const std::string &url);
std::shared_ptr<CachePacket> on_request(lua_State *L, const std::string &request_url);
void push_pointer(lua_State *L, void *pointer, const std::string &name);
static int get_body(lua_State *L);


URLStorage::URLStorage()
{
	L = lua_reset();
	push_pointer(L, this, "storage");
	lua_register(L, "get_pkt", URLStorage::get_pkt);
	lua_register(L, "copy_body", URLStorage::copy_body);
	lua_register(L, "new_pkt", URLStorage::new_pkt);
}

URLStorage::~URLStorage()
{
	lua_close(L);
}

void URLStorage::add(const std::string &url, std::shared_ptr<CachePacket> data)
{
	std::lock_guard<std::mutex> lk(lua_mutex);
	cache_map[url] = data;
	on_cache(L,url);
}

std::shared_ptr<CachePacket> URLStorage::get(const std::string &url)
{
	std::lock_guard<std::mutex> lk(lua_mutex);
	return on_request(L,url);
}

namespace{
	const char *lib = R"ABCD(path_table = {}
url_table = {}
rule_table = {}

function merge_table(a, b)
	local c = {}
	for k, v in pairs(a) do 
		c[k] = v
	end
	for k, v in pairs(b) do
		c[k] = v
	end
	return c
end

function is_tengine(url)
	if url:find('%?%?') then return true else return false end
end

function parse_tengine_url(url)
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

function parse_url2(url)
	local path, query = parse_url(url)
	local query_table = parse_query(query)
	return {url=url, path=path,query=query, query_table=query_table}
end

function split(s, delimiter)
    result = {};
    for match in (s..delimiter):gmatch("(.-)"..delimiter) do
        table.insert(result, match);
    end
    return result;
end

function tprint (tbl, indent)
  if not indent then indent = 0 end
  for k, v in pairs(tbl) do
    formatting = string.rep("  ", indent) .. k .. ": "
    if type(v) == "table" then
      print(formatting)
      tprint(v, indent+1)
    elseif type(v) == 'boolean' then
      print(formatting .. tostring(v))      
    else
      print(formatting .. v)
    end
  end
end

function parse_query(q)
	if not q then return nil end
	local x = split(q, '&')

	local y = {}
	for k, v in ipairs(x) do
		local z = split(v, '=')
		y[z[1]] = z[2]
	end
	return y
end

function on_cache(url)
	-- save in url table
	url_table[url] = #url_table
	-- save path table
	local path, query = parse_url(url)
	local query_table = parse_query(query)
	local cache = path_table[path]
	local entry = {url=url, query=query, query_table=query_table}

	if cache then
		cache[#cache + 1] = entry
	else
		cache = {entry}
		path_table[path] = cache
	end
end

function relation_filter(relations, request_query_table, cache_query_table)
	request_query_table = request_query_table or {}
	cache_query_table = cache_query_table or {}
	for k, v in pairs(relations) do
		if v == '!' then
			if request_query_table[k] ~= nil then
				return
			end
			if cache_query_table[k] ~= nil then
				return
			end
		elseif v == '=' then
			if request_query_table[k] == nil then
				return
			end
			if cache_query_table[k] == nil then
				return
			end
			if request_query_table[k] ~= cache_query_table[k] then
				return 
			end
		elseif v == '?' then
			if request_query_table[k] == nil then
				return
			end
			if cache_query_table[k] == nil then
				return
			end
		elseif v == '*' then

		end
	end
	return true
end

function filter_query(request_query_table, relations, cache_query_table)
	function filter_query_1(k, v, rel, query_table, wildcard)
		if rel == '=' then
			if v ~= cache_query_table[k] then
				return
			end
		elseif rel == '!' then
			return 
		elseif rel == '?' then
			if cache_query_table[k] == nil then
				return
			end
		elseif rel == '*' then

		elseif rel == nil then
			return filter_query_1(k, v, wildcard, query_table, '=')
		end
		return true
	end
	local wildcard = relations['*'] or '='
	if request_query_table then
		for k, v in pairs(request_query_table) do 
			local rel = relations[k]
			if not filter_query_1(k, v, rel, cache_query_table, wildcard) then
				return
			end
		end
	end
	return relation_filter(relations, request_query_table, cache_query_table)
end

function do_rule(request_url, rule)
	local path, request_query = request_url.path, request_url.query
	local request_query_table = request_url.query_table
	local cache = path_table[path]

	if cache ~= nil then
		for k, entry in pairs(cache) do
			if filter_query(request_query_table, rule, entry['query_table']) then
				return entry['url']
			end
		end
	end
	return nil
end

function add_string_rule(path, script)
	obj, err = load('return '..script)
	if obj == nil then return end
	add_rule(path, obj())
end

-- @param rule: table or function,
-- object {query filed=<'!|=|*|?'>}
-- function (request_url, url={path=,query=,query_table})
function add_rule(path, rule)
	rule_table[path] = rule
end

function on_tengine_request(request_url)
	local path, query = parse_url(request_url)
	local x = path_table[path]
	if x then 
		for k, entry in pairs(x) do
			if entry['query'] == query then
				return entry['url']
			end
		end
	end
	return nil
end

function replace_field(request_url, rule, field_name)
	rule = rule or {}
	field_name = field_name or 'callback'
	if request_url.query_table == nil or request_url.query_table[field_name] == nil then
		print('not found', request_url.url)
		t = {}
		t[field_name] = '!'
		return do_rule(request_url, merge_table(rule, t))
	end
	local request_query_table = request_url.query_table
	t = {}
	t[field_name] = '?'
	local cache_url = do_rule(request_url, merge_table(rule, t))
	if cache_url == nil then return nil end
	cache_url = parse_url2(cache_url)
	local pkt = get_pkt(storage, cache_url.url)
	local body = copy_body(pkt)
	local request_jsonp = request_url.query_table[field_name]
	local cache_jsonp = cache_url.query_table[field_name]
	print('jsonp', cache_jsonp, request_jsonp, request_url.url)
	body = body:gsub(cache_jsonp, request_jsonp)
	-- body:gsub
	local new_pkt = new_pkt(pkt, body)
	return new_pkt
end

function all_rule(request_url)
	return replace_field(request_url, {spm='*', _ksTS='*'})
end

function on_request(request_url)
	-- if found url then return url, so we can find it in c++ map
	local url = url_table[request_url]
	if url ~= nil then return request_url end
	-- if is tengine we use special functions
	if is_tengine(request_url) then
		return on_tengine_request(request_url)
	end
	-- if not found url, we fetch rules, if no rule found, we return nil
	local request_url = parse_url2(request_url)
	local rule = rule_table[request_url.path]
	if type(rule) == 'table' then
		return do_rule(request_url, rule)
	elseif type(rule) == 'function' then
		return rule(request_url)
	end
	return all_rule(request_url)
end)ABCD";

	std::shared_ptr<Response> post_rule(lua_State *L, const std::string request_url, const std::string &cached_url, std::shared_ptr<Response> response)
	{
		if (!response) {
			return nullptr;
		}
		if (request_url.empty()){
			printf("");
		}
		lua_getglobal(L, "post_rule");
		lua_pushstring(L, request_url.data());
		lua_pushstring(L, cached_url.data());
		// push body
		if (response && response->body) {
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
}

void add_rule(lua_State *L, const std::string &path, const std::string &script)
{
	lua_getglobal(L, "add_string_rule");
	lua_pushstring(L, path.data());
	lua_pushstring(L, script.data());

	if (lua_pcall(L, 2, 0, 0)) {
		printf("error %s\n", lua_tostring(L, -1));
		throw 1;
	}
}

void on_cache(lua_State *L, const std::string &url)
{
	lua_getglobal(L, "on_cache");
	lua_pushstring(L, url.data());

	if (lua_pcall(L, 1, 0, 0)) {
		printf("error %s\n", lua_tostring(L, -1));
		throw 1;
	}
}

std::shared_ptr<CachePacket> URLStorage::on_request(lua_State *L, const std::string &request_url)
{
	lua_getglobal(L, "on_request");
	lua_pushstring(L, request_url.data());

	if (lua_pcall(L, 1, 1, 0)) {
		printf("error %s\n", lua_tostring(L, -1));
		throw 1;
	}

	std::string url;
	std::shared_ptr<CachePacket> pkt;
	if (lua_isnil(L, -1)){
		// return nullptr;
	}
	else if (lua_isstring(L, -1)) {
		url = lua_tostring(L, -1);
		auto it = cache_map.find(url);
		if (it != cache_map.end()) {
			return it->second;
		}
		return nullptr;
	}
	else if (lua_isuserdata(L, -1)){
		auto tmp_pkt = (CachePacket*)lua_touserdata(L, -1);
		pkt.reset(tmp_pkt);
	}
	else{
		printf("error rules must return a string");
		throw 2;
	}
	lua_pop(L, 1);
	return pkt;
}

void push_pointer(lua_State *L, void *pointer, const std::string &name)
{
	lua_pushlightuserdata(L, pointer);
	lua_setglobal(L, name.data());
}

int get_body(lua_State *L)
{
	std::string url = lua_tostring(L, -1);
	auto pointer = lua_touserdata(L, -2);

	return 1;
}

lua_State* URLStorage::lua_reset()
{
	std::unique_lock<std::mutex> lua_lk(lua_mutex);
	if (L) {
		lua_close(L);
	}
	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_loadstring(L, lib);
	auto e = lua_pcall(L, 0, 0, 0);
	assert(!e);
	return L;
}

void URLStorage::addRule(const std::string &path, const std::string &script)
{
	std::lock_guard<std::mutex> lk(lua_mutex);
	add_rule(L, path, script);
}

int URLStorage::get_pkt(lua_State *L)
{
	auto storage = (URLStorage*)lua_touserdata(L, -2);
	auto url = lua_tostring(L, -1);
	assert(storage && url);

	auto it = storage->cache_map.find(url);
	// check
	if (it == storage->cache_map.end()) {
		throw 1;
		lua_pushnil(L);
	}
	lua_pushlightuserdata(L, it->second.get());
	return 1;
}

int URLStorage::copy_body(lua_State *L)
{
	auto pkt = (CachePacket*)lua_touserdata(L, -1);
	assert(pkt);

	lua_pushlstring(L, (const char *)pkt->response->body->data(), pkt->response->body->size());
	return 1;
}

int URLStorage::new_pkt(lua_State *L)
{
	auto pkt = (CachePacket*)lua_touserdata(L, -2);
	assert(pkt);
	size_t body_len = 0;
	auto body = lua_tolstring(L, -1, &body_len);
	if (!body || !body_len){
		throw 1;
	}
	auto new_body = std::make_shared<std::vector<uint8_t>>(body, body + body_len);

	auto response = std::make_shared<Response>(pkt->response->headers, new_body, pkt->response->status, pkt->response->status_text);
	auto new_pkt = new CachePacket(pkt->url, pkt->request, response);
	lua_pushlightuserdata(L, new_pkt);
	return 1;
}

bool FIO::write(const std::string & dir, const std::string &filename, size_t size, const char *data)
{
	auto p = boost::filesystem::path(dir);
	if (!boost::filesystem::exists(p) && !boost::filesystem::create_directories(p)) {
		return false;
	}
	p /= filename;
	std::ofstream of(p.string(), std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
	if (of && of.write(data, size)) {
		return true;
	}
	return false;
}

std::pair<size_t, const char*> FIO::read(const std::string &filename)
{
	auto p = boost::filesystem::path(filename);
	if (!boost::filesystem::exists(p)) {
		return std::pair<size_t, const char*>();
	}
	char *buf = nullptr;
	auto size = (size_t)boost::filesystem::file_size(p);
	buf = new char[size];
	std::ifstream f(p.string(), std::ifstream::in | std::ifstream::binary);

	if (f && f.read(buf, size)) {
		return std::make_pair(size, buf);
	}
	delete[] buf;
	return std::make_pair(0, nullptr);
}

std::vector<std::string> FIO::getAll(const std::string &dir)
{
	try {
		std::vector<std::string> files;
		auto p = boost::filesystem::path(dir);
		if (!boost::filesystem::exists(p)) {
			return files;
		}

		for (boost::filesystem::directory_iterator i(dir); i != boost::filesystem::directory_iterator(); i++)
		{
			try {
				if (boost::filesystem::exists(i->path()) && boost::filesystem::is_regular_file(i->path())) {
					files.push_back(i->path().filename().string());
				}
			}
			catch (...) {
				continue;
			}
		}
		return files;
	} catch (...) {
		return std::vector<std::string>();
	}
}

FileStorage::FileStorage(const std::string &dir, std::unique_ptr<IO> io) : m_dir(dir), m_io(move(io))
{
}

std::shared_ptr<CachePacket> FileStorage::get(const std::string &filename)
{
	auto path = boost::filesystem::path(m_dir);
	path.append(filename);
	auto data = m_io->read(path.string());
	if (!data.first) {
		return nullptr;
	}
	auto cache_pkt = Serialization::parse2(data.first, data.second);
	return std::shared_ptr<CachePacket>(cache_pkt);
}

void FileStorage::add(const std::string &url, std::shared_ptr<CachePacket> data)
{
	char *buf = nullptr;
	auto size = Serialization::serialize(&buf, *data);
	auto url_md5 = md5(url.data(), url.size());
	m_io->write(m_dir, url_md5, size, buf);
	delete[] buf;
}

void FileStorage::loadAll(IStore &target_storage)
{
	auto files = m_io->getAll(m_dir);
	for (auto &filename : files) {
		auto response = get(filename);
		target_storage.add(response->url, response);
	}
}

PersistentStorage::~PersistentStorage()
{
}

void PersistentStorage::add(const std::string &url, std::shared_ptr<CachePacket> data)
{
	m_url_storage->add(url, data);
	m_file_storage->add(url, data);
}

std::shared_ptr<CachePacket> PersistentStorage::get(const std::string &url)
{
	return m_url_storage->get(url);
}

PersistentStorage::PersistentStorage(std::unique_ptr<URLStorage> url_storage, std::unique_ptr<FileStorage> file_storage):
m_url_storage(move(url_storage)), m_file_storage(move(file_storage))
{
}

void PersistentStorage::loadAll()
{
	m_file_storage->loadAll(*m_url_storage);
}

void PersistentStorage::addRule(const std::string &path, const std::string &script)
{
	m_url_storage->addRule(path, script);
}
