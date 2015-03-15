#include <fstream>
#include <boost/filesystem.hpp>
#include "serialization.hpp"
#include "storage.hpp"
#include "stream.hpp"
#include "utility.hpp"
#include "url.hpp"

bool URLStorage::compare(const std::set<std::string> &reqired, const std::set<std::string> &t)
{
	for (auto &key : reqired)
	{
		if (t.find(key) == t.end()) {
			return false;
		}
	}
	return true;
}

URLStorage::~URLStorage()
{
}

std::shared_ptr<CachePacket> URLStorage::get(const std::string &url)
{
	auto cache_it = cache_map.find(url);
	if (cache_it != cache_map.end()) {
		return cache_it->second;
	}
	// Special 
	std::string request_path, request_query;
	parse_url(url, request_path, request_query);
	auto request_query_token = split_string(request_query, '&');

	std::string cached_path, cached_query;
	std::set<std::string> cached_query_token;
	for (auto cache_it : cache_map)
	{
		parse_url(cache_it.first, cached_path, cached_query);
		if (request_path != cached_path) {
			continue;
		}
		cached_query_token = split_string(cached_query, '&');
		if (!compare(request_query_token, cached_query_token)) {
			continue;
		}
		return cache_it.second;
	}
	return nullptr;
}

void URLStorage::add(const std::string &url, std::shared_ptr<CachePacket> data)
{
	cache_map[url] = data;
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

FileStorage::~FileStorage()
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

std::shared_ptr<CachePacket> PersistentStorage::get(const std::string &filename)
{
	return m_url_storage->get(filename);
}

PersistentStorage::PersistentStorage(std::unique_ptr<URLStorage> url_storage, std::unique_ptr<FileStorage> file_storage):
m_url_storage(move(url_storage)), m_file_storage(move(file_storage))
{
}

void PersistentStorage::loadAll()
{
	m_file_storage->loadAll(*m_url_storage);
}
