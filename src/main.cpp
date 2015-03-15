#include "one.hpp"

auto s_tb_s_url_rule = R"ABCD(function rules(request_url)
	return url_rules_core(request_url, false, {'spm'})
end)ABCD";
auto click_simba_tb_url_rule = R"ABCD(function rules(request_url)
	return url_rules_core(request_url, false, {'spm'})
end)ABCD";
auto apollon_market = R"ABCD(
function rules(request_url)
	return url_rules_core(request_url, false, {'t'})
end
)ABCD";
auto weekend_url_rule = R"ABCD(
function rules(request_url)
	return url_rules_core(request_url, false, {'callback', 't'})
end
)ABCD";
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
auto textlink_url_rule = R"ABCD(
function rules(request_url)
	return url_rules_core(request_url, false, {'callback'})
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
auto ald_url_rule = R"ABCD(
function rules(request_url)
	return url_rules_core(request_url, false, {'callback', '_ksTS'})
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
auto item_url_rule = R"ABCD(function rules(request_url)
	return url_rules_core(request_url, false, {'spm'})
end)ABCD";
auto detailskip_sib_url_rule = R"ABCD(function rules(request_url)
	return url_rules_core(request_url, false, {'ref'})
end)ABCD";
auto detailskip_activity_url_rule = R"ABCD(
function rules(request_url)
	return url_rules_core(request_url, false, {'callback', '_ksTS'})
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
	sq.add_rule("http://s.taobao.com/search", s_tb_s_url_rule, "");
	sq.add_rule("http://click.simba.taobao.com/cc_im", click_simba_tb_url_rule, "");
	sq.add_rule("http://apollon.t.taobao.com/market/AllContentByPage.do", apollon_market, "");
	sq.add_rule("http://www.taobao.com/go/rgn/global/weekend.php", weekend_url_rule, weekend_body_rule);
	sq.add_rule("http://textlink.simba.taobao.com/lk", textlink_url_rule, textlink_body_rule);
	sq.add_rule("http://ald.taobao.com/recommend2.htm", ald_url_rule, ald_body_rule);
	sq.add_rule("http://item.taobao.com/item.htm", item_url_rule, "");
	sq.add_rule("http://detailskip.taobao.com/json/sib.htm", detailskip_sib_url_rule, "");
	sq.add_rule("http://detailskip.taobao.com/json/activity.htm", detailskip_activity_url_rule, detailskip_activity_body_rule);
	sq.sync();
	sq.stop();
}
