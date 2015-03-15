#include "one.hpp"


auto weekend_body_rule = R"ABCD(
function bodys(request_url, cache_url, body)
	local request_path, request_query = parse_url(request_url)
	request_query = parse_query(request_query)
	local cache_path, cache_query = parse_url(cache_url)
	cache_query = parse_query(cache_query)
	local request_jsonp = find_value(request_query, 'callback')
	local cache_jsonp = find_value(cache_query, 'callback')
	body = body:gsub(cache_jsonp, request_jsonp)
	return body
end
)ABCD";

auto textlink_body_rule = R"ABCD(
function bodys(request_url, cache_url, body)
	local request_path, request_query = parse_url(request_url)
	request_query = parse_query(request_query)
	local cache_path, cache_query = parse_url(cache_url)
	cache_query = parse_query(cache_query)
	local request_jsonp = find_value(request_query, 'callback')
	local cache_jsonp = find_value(cache_query, 'callback')
	body = body:gsub(cache_jsonp, request_jsonp)
	return body
end
)ABCD";

auto ald_body_rule = R"ABCD(
function bodys(request_url, cache_url, body)
	local request_path, request_query = parse_url(request_url)
	request_query = parse_query(request_query)
	local cache_path, cache_query = parse_url(cache_url)
	cache_query = parse_query(cache_query)
	local request_jsonp = find_value(request_query, 'callback')
	local cache_jsonp = find_value(cache_query, 'callback')
	body = body:gsub(cache_jsonp, request_jsonp)
	return body
end
)ABCD";

auto detailskip_activity_body_rule = R"ABCD(
function bodys(request_url, cache_url, body)
	local request_path, request_query = parse_url(request_url)
	request_query = parse_query(request_query)
	local cache_path, cache_query = parse_url(cache_url)
	cache_query = parse_query(cache_query)
	local request_jsonp = find_value(request_query, 'callback')
	local cache_jsonp = find_value(cache_query, 'callback')
	body = body:gsub(cache_jsonp, request_jsonp)
	return body
end
)ABCD";

void main()
{
	Sq sq("1024", 3, 1024);
	sq.enable_log("cache_log");
	sq.start();
	sq.proxy()->addPreQueryFilter("http://s.taobao.com/search", Proxy::FilterType::Ignore, "{'spm'}");

	sq.proxy()->addPreQueryFilter("http://click.simba.taobao.com/cc_im", Proxy::FilterType::Ignore, "{'spm'}");

	sq.proxy()->addPreQueryFilter("http://apollon.t.taobao.com/market/AllContentByPage.do", Proxy::FilterType::Ignore, "{'t'}");
	
	sq.proxy()->addPreQueryFilter("http://www.taobao.com/go/rgn/global/weekend.php", Proxy::FilterType::Ignore, "{'callback','t'}");
	// sq.add_rule("http://www.taobao.com/go/rgn/global/weekend.php", weekend_url_rule, weekend_body_rule);
	
	sq.proxy()->addPreQueryFilter("http://textlink.simba.taobao.com/lk", Proxy::FilterType::Ignore, "{'callback'}");
	// sq.add_rule("http://textlink.simba.taobao.com/lk", textlink_url_rule, textlink_body_rule);
	
	sq.proxy()->addPreQueryFilter("http://ald.taobao.com/recommend2.htm", Proxy::FilterType::Ignore, "{'callback','_ksTS'}");
	// sq.add_rule("http://ald.taobao.com/recommend2.htm", ald_url_rule, ald_body_rule);
	sq.proxy()->addPreQueryFilter("http://item.taobao.com/item.htm", Proxy::FilterType::Ignore, "{'spm'}");
	
	sq.proxy()->addPreQueryFilter("http://detailskip.taobao.com/json/sib.htm", Proxy::FilterType::Ignore, "{'ref'}");
	// sq.add_rule("http://detailskip.taobao.com/json/activity.htm", detailskip_activity_url_rule, detailskip_activity_body_rule);

	sq.proxy()->addPreQueryFilter("http://www.v2.com/", Proxy::FilterType::Ignore, "{'abc'}");
	sq.sync();
	sq.stop();
}
