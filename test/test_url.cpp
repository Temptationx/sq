#include <gmock/gmock.h>
#include <list>
#include <map>
#include "../src/url.hpp"
using namespace std;

class SeparateStringTest : public ::testing::Test
{
public:

	std::string normalQuery = "a=b&c=d";
	std::vector<std::string> normalQueryList = std::vector<std::string>{"a=b", "c=d"};
	std::map<std::string, std::string> normalQueryMap;

	std::string normalQuery2 = "a=1&c=2";
	std::vector<std::string> normalQueryList2 = std::vector <std::string> {"a=1", "c=2"};
	std::map<std::string, std::string> normalQueryMap2;

	std::string singleQuery1 = "a=1";
	std::vector<std::string> singleQueryList1 = std::vector<std::string> {"a=1"};

	std::string emptyQuery = "";
	std::vector<std::string> emptyQueryList = std::vector<std::string>{};

	std::string connectedQuery1 = "a=1&&b=2";
	std::vector<std::string> connectedQueryList1 = std::vector<std::string>{"a=1", "b=2"};

	std::string connectedQuery2 = "a=1&&&";
	std::vector<std::string> connectedQueryList2 = std::vector < std::string > {"a=1"};

	std::string emptyLastQuery = "a=1&";
	std::vector<std::string> emptyLastQueryList = std::vector < std::string > {"a=1"};
	void testListEq(const std::vector<std::string> &l1, const std::vector<std::string> &l2)
	{
		ASSERT_EQ(l1.size(), l2.size());
		auto it1 = l1.begin();
		auto it2 = l2.begin();
		while (it1 != l1.end() && it2 != l2.end())
		{
			ASSERT_EQ((*it1), (*it2));
			it1++;
			it2++;
		}
	}
};

TEST_F(SeparateStringTest, SeparateQueryNormal)
{
	auto list = separateString(normalQuery, '&');
	ASSERT_EQ(normalQueryList, list);

	auto list2 = separateString(normalQuery2, '&');
	ASSERT_EQ(normalQueryList2, list2);
}

TEST_F(SeparateStringTest, SeparateQuerySingle)
{
	auto list1 = separateString(singleQuery1, '&');
	ASSERT_EQ(singleQueryList1, list1);
}

TEST_F(SeparateStringTest, SeparateEmpty)
{
	auto list1 = separateString(emptyQuery, '&');
	ASSERT_EQ(emptyQueryList, list1);
}

TEST_F(SeparateStringTest, SeparateConnected)
{
	auto list1 = separateString(connectedQuery1, '&');
	ASSERT_EQ(connectedQueryList1, list1);

	auto list2 = separateString(connectedQuery2, '&');
	ASSERT_EQ(connectedQueryList2, list2);
}

TEST_F(SeparateStringTest, SeparateEmptyLast)
{
	auto list1 = separateString(emptyLastQuery, '&');
	ASSERT_EQ(emptyLastQueryList, list1);
}

class SplitStringTest : public ::testing::Test
{
public:
	std::string str1 = "a=b";
	size_t posMiddle = 1;
	std::pair<std::string, std::string> middleSplitted = { "a", "b" };

	size_t posFront = 0;
	std::pair<std::string, std::string> frontSplitted = { "", "=b" };

	size_t posEnd = 3;
	std::pair<std::string, std::string> endSplitted = { "a=b", "" };

	std::pair<std::string, std::string> normalPair = { "a", "b" };

	std::pair<std::string, std::string> notFound = { "a=b", "" };

	std::string str2 = "a=";
	std::pair<std::string, std::string> before = { "a", "" };
	std::string str3 = "=b";
	std::pair<std::string, std::string> after = { "", "b" };
};

TEST_F(SplitStringTest, SeparateString2Pos)
{
	auto ret = separateString2(str1, posMiddle);
	ASSERT_EQ(ret, middleSplitted);

	auto ret2 = separateString2(str1, posFront);
	ASSERT_EQ(ret2, frontSplitted);

	auto ret3 = separateString2(str1, posEnd);
	ASSERT_EQ(ret3, endSplitted);
}

TEST_F(SplitStringTest, SeparateString2Find)
{
	auto ret = separateString2(str1, '=');
	ASSERT_EQ(ret, normalPair);

	auto ret2 = separateString2(str1, '&');
	ASSERT_EQ(ret2, notFound);

	auto ret3 = separateString2(str2, '=');
	ASSERT_EQ(ret3, before);

	auto ret4 = separateString2(str3, '=');
	ASSERT_EQ(ret4, after);
}

class ParseQueryTest : public ::testing::Test
{
public:
	void SetUp()
	{
		normalQueryMap = std::map < std::string, std::string > {{"a", "b"}, { "c", "d" }};
		normalQueryMap2 = std::map < std::string, std::string > {{"a", "1"}, { "c", "2" }};
		normalQueryMap3 = std::map < std::string, std::string > {{"a", "1"}};
	}
	std::string normalQuery = "a=b&c=d";
	std::map<std::string, std::string> normalQueryMap;

	std::string normalQuery2 = "a=1&c=2";
	std::map<std::string, std::string> normalQueryMap2;

	std::string normalQuery3 = "a=1";
	std::map<std::string, std::string> normalQueryMap3;

	
};

TEST_F(ParseQueryTest, QueryNormal)
{
	auto map1 = parseQuery(normalQuery);
	ASSERT_EQ(normalQueryMap, map1);

	auto map2 = parseQuery(normalQuery2);
	ASSERT_EQ(normalQueryMap2, map2);

	auto map3 = parseQuery(normalQuery3);
	ASSERT_EQ(normalQueryMap3, map3);

	// Empty
	auto emptyMap = parseQuery("");
	ASSERT_TRUE(emptyMap.empty());
}

class ParseUrlTest : public ::testing::Test
{
public:
	std::string noQuery = "http://www.baidu.com/";
	URL noQueryPair = URL{ "http://www.baidu.com/", "" };

	std::string hasQuery = "http://www.baidu.com/search?q=abc";
	URL hasQueryPair = URL{ "http://www.baidu.com/search", "q=abc" };

	std::string concatQuery = "http://www.baidu.com/search??q=abc";
	URL concatQueryPair = URL{ "http://www.baidu.com/search", "q=abc" };

	std::string concatQueryWithV = "http://www.baidu.com/search??q=abc?v=1";
	URL concatQueryWithVPair = URL{ "http://www.baidu.com/search", "q=abc" };

	std::string concatQueryWithAmp = "http://www.baidu.com/search??q=abc&v=1";
	URL concatQueryWithAmpPair = URL{ "http://www.baidu.com/search", "q=abc" };
};

TEST_F(ParseUrlTest, NoQuery)
{
	auto ret = parse_url(noQuery);
	ASSERT_EQ(ret, noQueryPair);
}

TEST_F(ParseUrlTest, HasQuery)
{
	auto ret = parse_url(hasQuery);
	ASSERT_EQ(ret, hasQueryPair);
}

TEST_F(ParseUrlTest, ConcatQuery)
{
	auto ret = parse_url(concatQuery);
	ASSERT_EQ(ret, concatQueryPair);
}

TEST_F(ParseUrlTest, ConcatQueryWithV)
{
	auto ret = parse_url(concatQueryWithV);
	ASSERT_EQ(ret, concatQueryWithVPair);
}

TEST_F(ParseUrlTest, ConcatQueryWithAmp)
{
	auto ret = parse_url(concatQueryWithAmp);
	ASSERT_EQ(ret, concatQueryWithAmpPair);
}

TEST(HELPER, CMPMAP)
{
	auto s = compare_map({ { "_", "1" }, { "q", "abc" } }, { { "_", "2" }, { "q", "abc" } }, { "_" });
	ASSERT_TRUE(s);
}

TEST(HELPER, Compar)
{
	auto s = compare_map2({ { "_", "1" }, { "q", "abc" } }, { { "_", "2" }, { "q", "abc" } }, { "q" });
	ASSERT_TRUE(s);
}

TEST(CMPMAP, KeyNotEq)
{
	auto s = compare_map({{"a", "b"}}, {{"c", "d"}}, {});
	ASSERT_FALSE(s);
}

TEST(CMPMAP, ValueNotEq)
{
	auto s = compare_map({ { "a", "b" } }, { { "a", "d" } }, {});
	ASSERT_FALSE(s);
}

TEST(parseURLSig2, NormalTest)
{
	std::string path, query, url("http://www.baidu.com/?q=jik");
	parse_url(url, path, query);
	ASSERT_EQ(path, "http://www.baidu.com/");
	ASSERT_EQ(query, "q=jik");
}



TEST(W, W)
{
	string path, query;
	parse_url("http://taobao.com/??ie9.js,webkit.js?v=1.1", path, query);
	ASSERT_EQ("http://taobao.com/", path);
	ASSERT_EQ("ie9.js,webkit.js", query);
	auto header = parseQuery(query);
	map<string, string> m{ { "ie9.js,webkit.js", ""}};
	ASSERT_EQ(m, header);

	string url = buildURL(path, m);
	ASSERT_EQ("http://taobao.com/?ie9.js,webkit.js", url);
}

TEST(W, W2)
{
	string path, query;
	parse_url("http://taobao.com/", path, query);
	auto header = parseQuery(query);
	string url = buildURL(path, header);
	ASSERT_EQ("http://taobao.com/", url);
}


TEST(RULE, optout)
{
	auto url = filter("http://www.baidu.com/search?q=123&v=0.1", { "v" }, optout);
	ASSERT_EQ("http://www.baidu.com/search?q=123", url);
}

TEST(Rule, optoutEmptyQuery)
{
	auto url = filter("http://www.baidu.com/search", { "v" }, optout);
	ASSERT_EQ("http://www.baidu.com/search", url);
}

TEST(Rule, optoutEmpty)
{
	auto url = filter("http://www.baidu.com/search?q=123&u=333", { "v" }, optout);
	ASSERT_EQ("http://www.baidu.com/search?q=123&u=333", url);
}

TEST(Rule, optoutTengine)
{
	auto url = filter("http://www.baidu.com/search??base-min.js,box-min.js?v=123", { "q" }, optout);
	ASSERT_EQ("http://www.baidu.com/search?base-min.js,box-min.js", url);
}

TEST(RULE, optin)
{
	auto url = filter("http://www.baidu.com/search?q=123&v=0.1", { "q" }, optin);
	ASSERT_EQ("http://www.baidu.com/search?q=123", url);
}

TEST(RULE, optinEmpty)
{
	ASSERT_THROW(filter("http://www.baidu.com/search", { "q" }, optin), exception);
}

