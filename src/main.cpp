#include "one.hpp"
namespace
{
	std::string erase_all_query = R"ABC(function pre_rule(url)
	local path, query = parse_url(url)
	return path
end)ABC";
}

void main()
{
	Sq sq("1024", 3, 1024);
	sq.enable_log("cache_log");
	sq.start();

	sq.proxy()->addJSONPRule("http://s.taobao.com/search", {"_ksTS", "spm"});

	sq.proxy()->addPreQueryFilter("http://click.simba.taobao.com/cc_im", Proxy::FilterType::Ignore, "{'spm'}");

	sq.proxy()->addPreQueryFilter("http://apollon.t.taobao.com/market/AllContentByPage.do", Proxy::FilterType::Ignore, "{'t'}");
	
	sq.proxy()->addJSONPRule("http://www.taobao.com/go/rgn/global/weekend.php", { "t" });
	
	sq.proxy()->addJSONPRule("http://textlink.simba.taobao.com/lk");

	sq.proxy()->addJSONPRule("http://ald.taobao.com/recommend2.htm", { "_ksTS" });

	sq.proxy()->addPreQueryFilter("http://item.taobao.com/item.htm", Proxy::FilterType::Ignore, "{'spm'}");
	
	sq.proxy()->addPreQueryFilter("http://detailskip.taobao.com/json/sib.htm", Proxy::FilterType::Ignore, "{'ref'}");

	sq.proxy()->addPreQueryFilter("http://detail.tmall.com/item.htm", Proxy::FilterType::Ignore, "{'spm'}");

	sq.proxy()->addPreQueryFilter("http://mdskip.taobao.com/core/initItemDetail.htm", Proxy::FilterType::Ignore, "{'ref', 'timestamp'}");

	sq.proxy()->addJSONPRule("http://dsr.rate.tmall.com/list_dsr_info.htm", { "_ksTS" });

	sq.proxy()->addPreQueryFilter("http://www.tmall.com/go/market/main/2014/mallbar/config/mconf-pub-2_4_3.php", Proxy::FilterType::Ignore, "{'_ksTS'}");

	sq.proxy()->addPreRule("http://www.taobao.com/go/app/tmall/login-api.php", erase_all_query);

	sq.proxy()->addJSONPRule("http://ext.mdskip.taobao.com/extension/queryTmallCombo.do", { "_ksTS" });

	sq.proxy()->addPreRule("http://uaction.aliyuncdn.com/js/ua.js", erase_all_query);

	sq.proxy()->addJSONPRule("http://suggest.taobao.com/sug", { "_ksTS" });

	sq.proxy()->addPreQueryFilter("http://aldcdn.tmall.com/recommend.htm", Proxy::FilterType::Ignore, "{'refer'}");

	sq.proxy()->addPreQueryFilter("http://www.tmall.com/tms/read-tms3.php", Proxy::FilterType::Ignore, "{'t'}");

	sq.proxy()->addJSONPRule("http://www.tmall.com/go/rgn/tmall/searchbar/act.php", { "_ksTS" });

	sq.proxy()->addJSONPRule("http://ald.taobao.com/recommend.htm", { "_ksTS", "refer" });

	sq.proxy()->addPreQueryFilter("http://www.taobao.com/tms/read-tms2.php", Proxy::FilterType::Ignore, "{'t'}");

	sq.proxy()->addJSONPRule("http://rate.tmall.com/listTagClouds.htm", { "_ksTS", "t" });

	sq.proxy()->addJSONPRule("http://rate.tmall.com/list_detail_rate.htm", { "_ksTS", "ua" });

	sq.proxy()->addJSONPRule("http://ext.mdskip.taobao.com/extension/dealRecords.htm", { "_ksTS", "isg" });

	sq.proxy()->addPreQueryFilter("http://www.taobao.com/malldetail/read-tms.php", Proxy::FilterType::Ignore, "{'t'}");

	sq.proxy()->addPreRule("http://www.tmall.com/go/market/main/mui/storage/stp-1_1_2.php", erase_all_query);

	sq.proxy()->addJSONPRule("http://rate.tmall.com/listTryReport.htm", { "_ksTS" });

	sq.proxy()->addJSONPRule("http://ext.mdskip.taobao.com/extension/query_ecity_service.htm", { "_ksTS" });

	sq.proxy()->addPreQueryFilter("http://www.alimama.com/chongzhi/dian_ka.do", Proxy::FilterType::Ignore, "{'_ksTS', 'callback'}");

	sq.proxy()->addJSONPRule("http://u.alimama.com/chongzhi/dian_ka.do", {"_ksTS"});

	sq.proxy()->addJSONPRule("http://www.alimama.com/chongzhi/dian_ka_list.do", { "_ksTS" });

	sq.proxy()->addJSONPRule("http://www.alimama.com/chongzhi/phone_list.do", { "_ksTS" });

	sq.proxy()->addJSONPRule("http://tad.t.taobao.com/api/list", { "_ksTS", "buyerid", "cna" });

	sq.proxy()->addJSONPRule("http://detailskip.taobao.com/json/activity.htm", { "_ksTS" });

	// sq.proxy()->addJSONPRule("http://www.taobao.com/go/rgn/tb-fp/2014/tbar.php", {"t"}); // Error after apply rule, this url will have no query, so we must make sure get newest cache.
	sq.proxy()->addJSONPRule("http://tui.taobao.com/recommend", { "_ksTS", "buyerid", "cna" });

	sq.proxy()->addPreQueryFilter("http://xx3csc.taobao.com/", Proxy::FilterType::Ignore, "{'spm'}");

	sq.sync();
	sq.stop();
}


/*
ga like link
amos.alicdn.com
ac.mmstat.com
q5.cnzz.com
ac.atpanel.com
count.tbcdn.cn
*/