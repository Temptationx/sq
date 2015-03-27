#ifndef cache_store_h__
#define cache_store_h__
#include "type.hpp"
#include <vector>
#include <set>
#include <lua.hpp>
#include <mutex>

class IO
{
public:
	virtual ~IO(){}
	virtual bool write(const std::string & dir, const std::string &filename, size_t size, const char *data) = 0;
	virtual std::pair<size_t, const char*> read(const std::string &filename) = 0;
	virtual std::vector<std::string> getAll(const std::string &dir) = 0;
};

class FIO : public IO
{
public:
	virtual ~FIO(){}
	virtual bool write(const std::string & dir, const std::string &filename, size_t size, const char *data) override;
	virtual std::pair<size_t, const char*> read(const std::string &filename) override;
	virtual std::vector<std::string> getAll(const std::string &dir) override;
};

class IStore
{
public:
	virtual ~IStore(){};
	virtual void add(const std::string &url, std::shared_ptr<CachePacket> data) = 0;
	virtual std::shared_ptr<CachePacket> get(const std::string &filename) = 0;
};

class FileStorage : public IStore
{
public:
	FileStorage(const std::string &dir, std::unique_ptr<IO> io);
	virtual ~FileStorage(){};
	virtual void add(const std::string &url, std::shared_ptr<CachePacket> data) override;
	virtual std::shared_ptr<CachePacket> get(const std::string &filename) override;
	void loadAll(IStore &target_storage);
public:
	std::string m_dir;
	std::unique_ptr<IO> m_io;
};

class URLStorage : public IStore
{
	using Map = std::map<std::string, std::shared_ptr<CachePacket>>;
public:
	URLStorage();
	virtual ~URLStorage();
	virtual void add(const std::string &url, std::shared_ptr<CachePacket> data) override;
	virtual std::shared_ptr<CachePacket> get(const std::string &url) override;
	void addRule(const std::string &path, const std::string &script);
private:
	Map cache_map;

	// search engine
	lua_State *L = nullptr;
	std::mutex lua_mutex;
	lua_State* lua_reset();
	static int get_pkt(lua_State *L);
	static int copy_body(lua_State *L);
	static int new_pkt(lua_State *L);
	std::shared_ptr<CachePacket> on_request(lua_State *L, const std::string &request_url);
};

class PersistentStorage : public IStore
{
public:
	PersistentStorage(std::unique_ptr<URLStorage> url_storage, std::unique_ptr<FileStorage> file_storage);
	virtual ~PersistentStorage();
	virtual void add(const std::string &url, std::shared_ptr<CachePacket> data) override;
	virtual std::shared_ptr<CachePacket> get(const std::string &filename) override;

	virtual void loadAll();
	void addRule(const std::string &path, const std::string &script);
private:
	std::unique_ptr<URLStorage> m_url_storage;
	std::unique_ptr<FileStorage> m_file_storage;
};

template<typename IOT>
class FileStorageFactory
{
public:
	static std::unique_ptr<FileStorage> build(const std::string &dir)
	{
		return std::make_unique<FileStorage>(dir, std::make_unique<IOT>());
	}
	static std::unique_ptr<FileStorage> build(const std::string &dir, IOT *io)
	{
		return std::make_unique<FileStorage>(dir, std::unique_ptr<IO>(io));
	}
};

class PersistentStorageFactory
{
public:
	static std::unique_ptr<PersistentStorage> build(const std::string &dir)
	{
		auto file_storage = FileStorageFactory<FIO>::build(dir);
		auto url_storage = std::make_unique<URLStorage>();
		return std::make_unique<PersistentStorage>(move(url_storage), move(file_storage));
	}
};

#endif // cache_store_h__
