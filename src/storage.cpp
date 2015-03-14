#include <fstream>
#include <boost/filesystem.hpp>
#include <md5.h>
#include "serialization.hpp"
#include "storage.hpp"
#include "stream.hpp"
#include "url.hpp"
namespace fs = boost::filesystem;

void charHex(unsigned char c, unsigned char &l, unsigned char &h)
{
	l = c & 0x0F;
	h = c >> 4;

	if (l > 9) {
		l = 'A' + l - 10;
	}
	else{
		l = '0' + l;
	}

	if (h > 9) {
		h = 'A' + h - 10;
	}
	else{
		h = '0' + h;
	}
}

std::string md5(const char *data, size_t size)
{
	MD5_CTX ctx = { 0 };
	MD5_Init(&ctx);
	MD5_Update(&ctx, data, size);
	unsigned char result[16];
	MD5_Final(result, &ctx);
	char hex[32];
	for (int i = 0; i < 16; i++)
	{
		unsigned char l, h;
		charHex(result[i], l, h);
		hex[i * 2] = h;
		hex[i * 2 + 1] = l;
	}
	std::string hex_str = std::string(hex, 32);
	return hex_str;
}

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

std::pair<std::shared_ptr<Response>, std::string> URLStorage::get(const std::string &url)
{
	auto &it = m.find(url);
	if (it != m.end()) {
		return std::make_pair(it->second, it->first);
	}
	std::string path1, query1;
	parseURL(url, path1, query1);
	auto querylist1 = separateStringSet(query1, '&');
	std::string path2, query2;
	std::set<std::string> querylist2;
	for (auto &it : m)
	{
		parseURL(it.first, path2, query2);
		if (path1 != path2) {
			continue;
		}
		querylist2 = separateStringSet(query2, '&');
		if (!compare(querylist1, querylist2)) {
			continue;
		}
		return std::make_pair(it.second, it.first);
	}
	return std::pair<std::shared_ptr<Response>, std::string>();
}

void URLStorage::add(const std::string &url, std::shared_ptr<Response> v)
{
	m[url] = v;
}

bool FIO::write(const std::string & dir, const std::string &filename, const char *data, size_t size)
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
	size_t size = boost::filesystem::file_size(p);
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

std::pair<std::shared_ptr<Response>, std::string> FileStorage::get(const std::string &filename)
{
	auto p = boost::filesystem::path(m_dir);
	p.append(filename);
	auto data = m_io->read(p.string());
	if (!data.first) {
		return std::pair<std::shared_ptr<Response>, std::string>();
	}
	auto pa = Serialization::parse(data.second, data.first);
	return std::make_pair(std::shared_ptr<Response>(pa.second), pa.first);
}

void FileStorage::add(const std::string &url, std::shared_ptr<Response> res)
{
	char *buf = nullptr;
	auto size = Serialization::serialize(&buf, std::make_pair(url, res.get()));
	auto m5 = md5(url.data(), url.size());
	m_io->write(m_dir, m5, buf, size);
	delete[] buf;
}

void FileStorage::loadAll(IStore &target_storage)
{
	auto files = m_io->getAll(m_dir);
	for (auto &file : files)
	{
		auto res = get(file);
		target_storage.add(res.second, res.first);
	}
}

PersistentStorage::~PersistentStorage()
{
}

void PersistentStorage::add(const std::string &url, std::shared_ptr<Response> res)
{
	m_url_storage->add(url, res);
	m_file_storage->add(url, res);
}

std::pair<std::shared_ptr<Response>, std::string> PersistentStorage::get(const std::string &filename)
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
