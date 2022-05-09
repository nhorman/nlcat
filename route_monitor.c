#include <netlink/route/route.h>
#include <jansson.h>
#include <monitors.h>
#include <route_monitor.h>
#include <parser.h>

/*
 * struct rtnl_route
{
        NLHDR_COMMON

        uint8_t                 rt_family;
        uint8_t                 rt_dst_len;
        uint8_t                 rt_src_len;
        uint8_t                 rt_tos;
        uint8_t                 rt_protocol;
        uint8_t                 rt_scope;
        uint8_t                 rt_type;
        uint8_t                 rt_nmetrics;
        uint8_t                 rt_ttl_propagate;
        uint32_t                rt_flags;
        struct nl_addr *        rt_dst;
        struct nl_addr *        rt_src;
        uint32_t                rt_table;
        uint32_t                rt_iif;
        uint32_t                rt_prio;
        uint32_t                rt_metrics[RTAX_MAX];
        uint32_t                rt_metrics_mask;
        uint32_t                rt_nr_nh;
        struct nl_addr *        rt_pref_src;
        struct nl_list_head     rt_nexthops;
        struct rtnl_rtcacheinfo rt_cacheinfo;
        uint32_t                rt_flag_mask;
};
*/

static char* ops[] = {
	"UNKNOWN",
	"RTMNSG_NEW_ROUTE",
	"RTMSG_DEL_ROUTE",
	"RTMSG_GET_ROUTE",
};

static struct json_map rt_data[] = {
        JSON_MAP_ENTRY("family", JSON_INTEGER, TBD, 0, NULL),
        JSON_MAP_ENTRY("tos", JSON_INTEGER, TBD, 0, NULL),
        JSON_MAP_ENTRY("protocol", JSON_INTEGER, TBD, 0, NULL),
        JSON_MAP_ENTRY("scope", JSON_INTEGER, TBD, 0, NULL),
        JSON_MAP_ENTRY("type", JSON_INTEGER, TBD, 0, NULL),
        JSON_MAP_ENTRY("ttl_propagate", JSON_INTEGER, TBD, 0, NULL),
        JSON_MAP_ENTRY("flags", JSON_INTEGER, TBD, 0 , NULL),
        JSON_MAP_ENTRY("dst", JSON_STRING, TBD, 0, NULL),
        JSON_MAP_ENTRY("src", JSON_STRING, TBD, 0, NULL),
        JSON_MAP_ENTRY("table", JSON_INTEGER, TBD, 0 , NULL),
        JSON_MAP_ENTRY("iif", JSON_INTEGER, TBD, 0 , NULL),
        JSON_MAP_ENTRY("prio", JSON_INTEGER, TBD, 0 , NULL),
        JSON_MAP_ENTRY("pref_src", JSON_STRING, TBD, 0, NULL),
};

static struct json_map rt_tree[] = JSON_COMMON_TOPLEVEL("route", "route", TBD, rt_data);

void route_change_cb(struct nl_cache *cache __unused, struct nl_object *obj, int val, void *data __unused)
{
	struct rtnl_route *route = (struct rtnl_route *)obj;
	int family;
	int tos, protocol, scope;
	int type, ttl_propagate;
	int flags, table, iif;
	int prio;
	struct nl_addr *src, *dst, *psrc;
	char sbuf[256], dbuf[256], psbuf[256];
	char *result;

	/* set the op */
	parser_set_val(rt_tree, ARRAY_SIZE(rt_tree), "op", ops[val]);

	family = rtnl_route_get_family(route);
	tos = rtnl_route_get_tos(route);
	protocol = rtnl_route_get_protocol(route);
	scope = rtnl_route_get_scope(route);
	type = rtnl_route_get_type(route);
	ttl_propagate = rtnl_route_get_ttl_propagate(route);
	flags = rtnl_route_get_flags(route);
	table = rtnl_route_get_table(route);
	iif = rtnl_route_get_iif(route);
	prio = rtnl_route_get_priority(route);
	memset(sbuf, 0, 256);
	memset(dbuf, 0, 256);
	memset(psbuf, 0, 256);

	src = rtnl_route_get_src(route);
	dst = rtnl_route_get_dst(route);
	psrc = rtnl_route_get_pref_src(route);

	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "family", &family);
	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "tos", &tos);
	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "protocol", &protocol);
	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "scope", &scope);
	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "type", &type);
	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "ttl_propagate", &ttl_propagate);
	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "flags", &flags);
	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "table", &table);
	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "iif", &iif);
	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "prio", &prio);
	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "src", nl_addr2str(src, sbuf, 256));
	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "dst", nl_addr2str(dst, dbuf, 256));
	parser_set_val(rt_data, ARRAY_SIZE(rt_data), "pref_src", nl_addr2str(psrc, psbuf, 256));

	result = compile_json_string(rt_tree, ARRAY_SIZE(rt_tree));
	print_json_event(result);
        parser_reset_tmpl(rt_tree, ARRAY_SIZE(rt_tree));
        free(result);	
}


