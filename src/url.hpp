#ifndef url_h__
#define url_h__

#include <utility>
#include <string>
#include <map>
#include <vector>
#include <set>

struct URL 
{
	URL();
	URL(std::initializer_list<std::string> il);
	URL(std::pair<std::string, std::string> w);
	bool operator == (const URL & rV);
	bool operator == (const URL & rV) const;
	std::string path;
	std::string query;
};

std::string buildURL(const std::string &path, const std::map<std::string, std::string> &headers);

URL parseURL(const std::string &url);
void parseURL(const std::string &url, std::string &path, std::string &query);
std::map<std::string, std::string> parseQuery(const std::string &query);

// Separate string use ch, it will ignore empty substring
std::vector<std::string> separateString(const std::string &str, char ch);
std::set<std::string> separateStringSet(const std::string &str, char ch);
// 
std::pair<std::string, std::string> separateString2(const std::string &str, size_t pos);
std::pair<std::string, std::string> separateString2(const std::string &str, char ch);


enum Filter{ optout, optin };
std::string filter(const std::string &url, std::vector<std::string> params, Filter filter);

bool compare_map(const std::map<std::string, std::string> &m1, const std::map<std::string, std::string> &m2, const std::set<std::string> &ignore);
bool compare_map2(const std::map<std::string, std::string> &m1, const std::map<std::string, std::string> &m2, const std::set<std::string> &accept);
#endif // url_h__
