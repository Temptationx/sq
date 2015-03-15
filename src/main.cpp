#include "one.hpp"

auto s_tb_s_url_rule = R"ABCD(function rules(request_url)
	local q_mark = '?'
	if is_tengine(request_url) then q_mark = q_mark .. '?' end
	local path, query = parse_url(request_url)
	if not query or #query == 0 then return request_url end
	local q_m = parse_query(query)
	q_m = ignore(q_m, {'spm'})
	query = compress_query(q_m)
	if query and #query > 0 then path = path .. q_mark end
	path = path .. query
	return path
end)ABCD";
auto click_simba_tb_url_rule = R"ABCD(function rules(request_url)
	local q_mark = '?'
	if is_tengine(request_url) then q_mark = q_mark .. '?' end
	local path, query = parse_url(request_url)
	if not query or #query == 0 then return request_url end
	local q_m = parse_query(query)
	q_m = ignore(q_m, {'spm'})
	query = compress_query(q_m)
	if query and #query > 0 then path = path .. q_mark end
	path = path .. query
	return path
end)ABCD";
void main()
{
	Sq sq("1024", 3, 1024);
	sq.enable_log("cache_log");
	sq.start();
	sq.add_rule("http://s.taobao.com/search", s_tb_s_url_rule, "");
	sq.add_rule("http://click.simba.taobao.com/cc_im", click_simba_tb_url_rule, "");
	sq.sync();
	sq.stop();
}
