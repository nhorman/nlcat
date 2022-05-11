#include <netlink/route/route.h>
#include <jansson.h>
#include <monitors.h>
#include <route_monitor.h>
#include <parser.h>

static char* ops[] = {
	"UNKNOWN",
	"RTMNSG_NEW_ROUTE",
	"RTMSG_DEL_ROUTE",
	"RTMSG_GET_ROUTE",
};

struct nexthop_storage {
        struct json_t *array;
        size_t num_strings;
        char *viastrings;
	char *gwstrings;
};

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
	data = create_json_report(report, "route", "route", ops[val]);

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


