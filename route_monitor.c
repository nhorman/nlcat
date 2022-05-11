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

void route_change_cb(struct nl_cache *cache __unused, struct nl_object *obj, int val, void *arg __unused)
{
	struct rtnl_route *route = (struct rtnl_route *)obj;
	struct nl_addr *src, *dst, *psrc;
	char sbuf[256], dbuf[256], psbuf[256];
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

	
	result = json_dumps(report, JSON_COMPACT); 
	print_json_event(result);
	json_decref(report);
        free(result);	
}


