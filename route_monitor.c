#include <netlink/route/route.h>
#include <netlink/route/addr.h>
#include <jansson.h>
#include <monitors.h>
#include <route_monitor.h>
#include <parser.h>



#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

static char* route_ops[] = {
	"UNKNOWN",
	"RTMSG_NEW_ROUTE",
	"RTMSG_DEL_ROUTE",
	"RTMSG_GET_ROUTE",
	"RTMSG_SET_ROUTE",
	"RTMSG_CHANGE_ROUTE",
};

static char *addr_ops[] = {
	"UNKNOWN", 
	"RTMSG_NEW_ADDR",
	"RTMSG_DEL_ADDR",
	"RTMSG_GET_ADDR",
	"RTMSG_SET_ADDR",
	"RTMSG_CHANGE_ADDR",
};

static char *link_ops[] = {
	"UNKNOWN", 
	"RTMSG_NEW_LINK",
	"RTMSG_DEL_LINK",
	"RTMSG_GET_LINK",
	"RTMSG_SET_LINK",
	"RTMSG_CHANGE_LINK"
};

struct nexthop_storage {
        struct json_t *array;
        size_t num_strings;
        char *viastrings;
	char *gwstrings;
};

static char int_ary_val[32];

const char *get_op(char *ops_ary[], int maxsize, int val)
{
	memset(int_ary_val, 0, 32);
	if (val >= maxsize) {
		sprintf(int_ary_val, "%d", val);
		return int_ary_val;
	}
	return ops_ary[val];
}

void fill_nexthop(struct rtnl_nexthop *nh, void *arg)
{
	struct nexthop_storage *stor = arg;
	json_t *new;
	struct nl_addr *gw;
	struct nl_addr *via;
	int index = stor->num_strings;
	char *viastring;
	char *gwstring;

	stor->num_strings++;

	stor->viastrings = realloc(stor->viastrings, 256 * stor->num_strings);
	memset(&stor->viastrings[index * 256], 0, 256);
	viastring = &stor->viastrings[index * 256];

	stor->gwstrings = realloc(stor->gwstrings, 256 * stor->num_strings);
	memset(&stor->gwstrings[index * 256], 0, 256);
	gwstring = &stor->gwstrings[index * 256];

	gw = rtnl_route_nh_get_gateway(nh);
	via = rtnl_route_nh_get_via(nh);
	new = json_object();

	JSON_ASSIGN_INT(new, "flags", rtnl_route_nh_get_flags(nh));
	JSON_ASSIGN_INT(new, "weight", rtnl_route_nh_get_weight(nh));
	JSON_ASSIGN_INT(new, "ifindex", rtnl_route_nh_get_ifindex(nh));
	JSON_ASSIGN_STRING(new, "gateway", nl_addr2str(gw, gwstring, 256));
	JSON_ASSIGN_STRING(new, "via", nl_addr2str(via, viastring, 256));
	json_array_append(stor->array, new);
	json_decref(new);
}

void route_change_cb(struct nl_cache *cache __unused, struct nl_object *obj, int val, void *arg __unused)
{
	struct rtnl_route *route = (struct rtnl_route *)obj;
	struct nl_addr *src, *dst, *psrc;
	char sbuf[256], dbuf[256], psbuf[256];
	struct nexthop_storage nhs;
	char *result;
	json_t *report;
	json_t *data;

	/* set the op */
	report = json_object();
	data = create_json_report(report, "route", "route", get_op(route_ops, ARRAY_SIZE(route_ops), val));

	memset(sbuf, 0, 256);
	memset(dbuf, 0, 256);
	memset(psbuf, 0, 256);

	src = rtnl_route_get_src(route);
	dst = rtnl_route_get_dst(route);
	psrc = rtnl_route_get_pref_src(route);

	JSON_ASSIGN_INT(data, "family", rtnl_route_get_family(route));
	JSON_ASSIGN_INT(data, "tos", rtnl_route_get_tos(route));
	JSON_ASSIGN_INT(data, "protocol", rtnl_route_get_protocol(route));
	JSON_ASSIGN_INT(data, "scope", rtnl_route_get_scope(route));
	JSON_ASSIGN_INT(data, "type", rtnl_route_get_type(route));
	JSON_ASSIGN_INT(data, "ttl_propagate", rtnl_route_get_ttl_propagate(route));
	JSON_ASSIGN_INT(data, "flags", rtnl_route_get_flags(route));
	JSON_ASSIGN_INT(data, "table", rtnl_route_get_table(route));
	JSON_ASSIGN_INT(data, "iif", rtnl_route_get_iif(route));
	JSON_ASSIGN_INT(data, "prio", rtnl_route_get_priority(route));
	JSON_ASSIGN_STRING(data, "src", nl_addr2str(src, sbuf, 256));
	JSON_ASSIGN_STRING(data, "dst", nl_addr2str(dst, dbuf, 256));
	JSON_ASSIGN_STRING(data, "psrc", nl_addr2str(psrc, psbuf, 256));

	/* Fill out nexthop list */
	memset(&nhs, 0, sizeof(struct nexthop_storage));
	nhs.array = create_json_child_array(data, "nexthops");
	rtnl_route_foreach_nexthop(route, fill_nexthop, &nhs);
	
	result = json_dumps(report, JSON_COMPACT); 
	print_json_event(result);
	json_decref(report);
	free(nhs.viastrings);
	free(nhs.gwstrings);
        free(result);	
}

void fill_link_info(json_t *obj, struct rtnl_link *link)
{
	struct nl_addr *nladdr;
	char addr[256], bcast[256];
	int out_netnsid;

	memset(addr, 0, 256);
	memset(bcast, 0, 256);
	if (link) {
		JSON_ASSIGN_STRING(obj, "name", rtnl_link_get_name(link));
		JSON_ASSIGN_INT(obj, "family", rtnl_link_get_family(link));
		JSON_ASSIGN_INT(obj, "arptype", rtnl_link_get_arptype(link));
		JSON_ASSIGN_INT(obj, "ifindex", rtnl_link_get_ifindex(link));
		JSON_ASSIGN_INT(obj, "flags", rtnl_link_get_flags(link));
		JSON_ASSIGN_INT(obj, "mtu", rtnl_link_get_mtu(link));
		JSON_ASSIGN_INT(obj, "link", rtnl_link_get_link(link));
		if (!rtnl_link_get_link_netnsid(link, &out_netnsid))
			JSON_ASSIGN_INT(obj, "link_nsid", out_netnsid);
		else
			JSON_ASSIGN_INT(obj, "link_nsid", -1);
		JSON_ASSIGN_INT(obj, "txqueuelen", rtnl_link_get_txqlen(link));
		JSON_ASSIGN_INT(obj, "master", rtnl_link_get_master(link));
		nladdr = rtnl_link_get_addr(link);
		JSON_ASSIGN_STRING(obj, "addr", nl_addr2str(nladdr, addr, 256));
		nladdr = rtnl_link_get_broadcast(link);
		JSON_ASSIGN_STRING(obj, "bcast", nl_addr2str(nladdr, bcast, 256));
		JSON_ASSIGN_INT(obj, "operstate", rtnl_link_get_operstate(link));
		JSON_ASSIGN_INT(obj, "linkmode", rtnl_link_get_linkmode(link));
		JSON_ASSIGN_STRING(obj, "kind", rtnl_link_get_type(link));
		JSON_ASSIGN_INT(obj, "carrier", rtnl_link_get_carrier(link));
	}
}

void addr_change_cb(struct nl_cache *cache __unused, struct nl_object *obj, int val, void *arg __unused)
{
	struct rtnl_addr *addr = (struct rtnl_addr *)obj;
	json_t *report;
	json_t *data;
	struct nl_addr *nladdr;
	char a_peer[256], a_local[256], a_bcast[256];
	char a_anycast[256], a_multicast[256];
	char *result;

	memset(a_peer, 0, 256);
	memset(a_local, 0, 256);
	memset(a_bcast, 0, 256);
	memset(a_anycast, 0, 256);
	memset(a_multicast, 0, 256);

	/* set the op */
	report = json_object();
	data = create_json_report(report, "route", "addr", get_op(addr_ops, ARRAY_SIZE(addr_ops), val));

	JSON_ASSIGN_INT(data, "family", rtnl_addr_get_family(addr));
	JSON_ASSIGN_INT(data, "prefixlen", rtnl_addr_get_prefixlen(addr));
	JSON_ASSIGN_INT(data, "scope", rtnl_addr_get_scope(addr));
	JSON_ASSIGN_INT(data, "flags", rtnl_addr_get_flags(addr));
	JSON_ASSIGN_INT(data, "ifindex", rtnl_addr_get_ifindex(addr));

	nladdr = rtnl_addr_get_peer(addr);
	JSON_ASSIGN_STRING(data, "peer", nl_addr2str(nladdr, a_peer, 256));

	nladdr = rtnl_addr_get_local(addr);
	JSON_ASSIGN_STRING(data, "local", nl_addr2str(nladdr, a_local, 256));

	nladdr = rtnl_addr_get_broadcast(addr);
	JSON_ASSIGN_STRING(data, "bcast", nl_addr2str(nladdr, a_bcast, 256));

	nladdr = rtnl_addr_get_anycast(addr);
	JSON_ASSIGN_STRING(data, "acast", nl_addr2str(nladdr, a_anycast, 256));

	nladdr = rtnl_addr_get_multicast(addr);
	JSON_ASSIGN_STRING(data, "mcast", nl_addr2str(nladdr, a_multicast, 256));

	JSON_ASSIGN_STRING(data, "label", rtnl_addr_get_label(addr));

	fill_link_info(create_json_child_object(report, "link"), rtnl_addr_get_link(addr));

	result = json_dumps(report, JSON_COMPACT);
	print_json_event(result);
	json_decref(report);
        free(result);	
}

void link_change_cb(struct nl_cache *cache __unused, struct nl_object *obj, int val, void *arg __unused)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	json_t *report;
	json_t *data;
	char *result;

	report = json_object();
	data = create_json_report(report, "route", "link", get_op(link_ops, ARRAY_SIZE(link_ops), val));

	fill_link_info(data, link);

	result = json_dumps(report, JSON_COMPACT);
	print_json_event(result);
	json_decref(report);
	free(result);
}
