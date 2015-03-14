#ifndef cache_store_h__
#define cache_store_h__
#include "type.hpp"
#include <vector>
#include <set>


std::string md5(const char *data, size_t size);

class IO
{
public:
	virtual ~IO(){}
	virtual bool write(const std::string & dir, const std::string &filename, const char *data, size_t size) = 0;
	virtual std::pair<size_t, const char*> read(const std::string &filename) = 0;
	virtual std::vector<std::string> getAll(const std::string &dir) = 0;
};

class FIO : public IO
{
public:
	virtual ~FIO(){}
	virtual bool write(const std::string & dir, const std::string &filename, const char *data, size_t size) override;
	virtual std::pair<size_t, const char*> read(const std::string &filename) override;
	virtual std::vector<std::string> getAll(const std::string &dir) override;
};

class IStore
{
public:
	virtual ~IStore(){};
	virtual void add(const std::string &url, std::shared_ptr<Response> res) = 0;
	virtual std::pair<std::shared_ptr<Response>, std::string> get(const std::string &filename) = 0;
};

class FileStorage : public IStore
{
public:
	FileStorage(const std::string &dir, std::unique_ptr<IO> io);
	virtual ~FileStorage();
	virtual void add(const std::string &url, std::shared_ptr<Response> res);
	virtual std::pair<std::shared_ptr<Response>, std::string> get(const std::string &filename);
	void loadAll(IStore &target_storage);
public:
	std::string m_dir;
	std::unique_ptr<IO> m_io;
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

class URLStorage : public IStore
{
	typedef std::map<std::string, std::shared_ptr<Response>> Map;
public:
	virtual ~URLStorage();
	virtual void add(const std::string &url, std::shared_ptr<Response> v);
	virtual std::pair<std::shared_ptr<Response>, std::string> get(const std::string &url);
private:
	bool compare(const std::set<std::string> &reqired, const std::set<std::string> &t);
	Map m;
};

class PersistentStorage : public IStore
{
public:
	PersistentStorage(std::unique_ptr<URLStorage> url_storage, std::unique_ptr<FileStorage> file_storage);
	virtual ~PersistentStorage();
	virtual void add(const std::string &url, std::shared_ptr<Response> res) override;
	virtual std::pair<std::shared_ptr<Response>, std::string> get(const std::string &filename) override;
	virtual void loadAll();
private:
	std::unique_ptr<URLStorage> m_url_storage;
	std::unique_ptr<FileStorage> m_file_storage;
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
