#include "url.hpp"
#include <algorithm>
#include <vector>

std::pair<std::string, std::string> separateString2(const std::string &str, size_t pos)
{
	if (pos >= str.size()) {
		return std::make_pair(str, std::string());
	}
	return std::make_pair(std::string(str, 0, pos), std::string(str, pos + 1));
}

std::pair<std::string, std::string> separateString2(const std::string &str, char ch)
{
	auto pos = str.find(ch);
	if (pos == std::string::npos) {
		return std::make_pair(str, std::string());
	}
	return std::make_pair(std::string(str, 0, pos), std::string(str, pos + 1));
}

URL parseURL(const std::string &url)
{
	auto pos = url.find('?');
	if (pos == std::string::npos){
		return URL{url, ""};
	}

	if (url[pos + 1] != '?') {
		return URL{separateString2(url, pos)};
	}

	// tengine
	auto version_pos = url.find('?', pos + 2);
	auto and_pos = url.find('&', pos + 2);
	auto inter_pos = std::min(version_pos, and_pos);
	if (inter_pos == std::string::npos) {
		inter_pos = std::max(version_pos, and_pos);
	}
	if (inter_pos == std::string::npos) {
		return URL{std::string(url, 0, pos), std::string(url, pos + 2)};
	}
	return URL{std::string(url, 0, pos), std::string(url, pos + 2, inter_pos - pos - 2)};
}

void parseURL(const std::string &url, std::string &path, std::string &query)
{
	URL u = parseURL(url);
	path = u.path;
	query = u.query;
}

std::vector<std::string> separateString(const std::string &str, char ch)
{
	std::vector<std::string> _list;
	if (str.empty()) {
		return _list;
	}
	size_t start = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == ch) {
			if (i == start) {
				start = i + 1;
			}
			else{
				_list.push_back(std::string(str, start, i - start));
				start = i + 1;
			}
		}
	}

	auto lastStr = std::string(str, start);
	if (!lastStr.empty() && lastStr.find(ch) == std::string::npos) {
		_list.push_back(lastStr);
	}
	_list.shrink_to_fit();
	return _list;
}

std::map<std::string, std::string> parseQuery(const std::string &query)
{
	std::vector<std::string> queryList = separateString(query, '&');
	std::map<std::string, std::string> queryMap;
	for (auto &singleQuery : queryList)
	{
		auto pairQuery = separateString2(singleQuery, '=');
		queryMap.insert(pairQuery);
	}
	return queryMap;
}

bool compare_map(const std::map<std::string, std::string> &m1, const std::map<std::string, std::string> &m2, const std::set<std::string> &ignore)
{
	for (auto &it : m1){
		if (ignore.count(it.first)){
			continue;
		}
		auto &m2It = m2.find(it.first);
		if (m2It == m2.end()){
			return false;
		}
		if (m2It->second != it.second){
			return false;
		}
	}
	return true;
}

bool compare_map2(const std::map<std::string, std::string> &m1, const std::map<std::string, std::string> &m2, const std::set<std::string> &accept)
{
	for (auto &it : m1){
		if (accept.count(it.first)){
			auto &m2It = m2.find(it.first);
			if (m2It == m2.end() || m2It->second != it.second)
			{
				return false;
			}
		}
	}
	return true;
}

std::set<std::string> separateStringSet(const std::string &str, char ch)
{
	std::set<std::string> _list;
	if (str.empty()) {
		return _list;
	}
	size_t start = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == ch) {
			if (i == start) {
				start = i + 1;
			}
			else{
				_list.emplace(std::string(str, start, i - start));
				start = i + 1;
			}
		}
	}

	auto lastStr = std::string(str, start);
	if (!lastStr.empty() && lastStr.find(ch) == std::string::npos) {
		_list.emplace(lastStr);
	}
	return _list;
}

std::string buildURL(const std::string &path, const std::map<std::string, std::string> &headers)
{
	std::string header_str = "?";
	for (auto it : headers)
	{
		header_str += it.first + "=" + it.second + "&";
	}
	while (header_str.size() > 0 && (header_str.at(header_str.size() - 1) == '&' || header_str.at(header_str.size() - 1) == '=' || header_str.at(header_str.size() - 1) == '?')) {
		header_str.pop_back();
	}
	return path + (header_str.empty() ? "" : header_str);
}

URL::URL()
{
}

URL::URL(std::pair<std::string, std::string> w) :path(w.first), query(w.second)
{
}

URL::URL(std::initializer_list<std::string> il)
{
	int i = 0;
	for (auto &item : il)
	{
		if (i == 0) {
			path = item;
			i++;
		}
		else if (i == 1){
			query = item;
			break;
		}
	}
}

bool URL::operator==(const URL & rV) const
{
	return path == rV.path && query == rV.query;
}

bool URL::operator==(const URL & rV)
{
	return path == rV.path && query == rV.query;
}

std::string filter(const std::string &url, std::vector<std::string> params, Filter filter)
{
	std::string path, query;
	parseURL(url, path, query);
	auto header = parseQuery(query);
	std::map<std::string, std::string> out;
	if (filter == optin) {
		for (auto &param : params)
		{
			auto it = header.find(param);
			if (it == header.end()) {
				throw std::exception();
			}
			else{
				out[it->first] = it->second;
			}
		}
	}
	else{
		for (auto &param : params)
		{
			auto it = header.find(param);
			if (it == header.end()) {
				continue;
			}
			else{
				header.erase(it);
			}
		}
		out = header;
	}
	return buildURL(path, out);
}

