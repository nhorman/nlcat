#include <jansson.h>
#include <netlink/xfrm/sa.h>
#include <netlink/xfrm/selector.h>
#include <monitors.h>
#include <xfrm_monitor.h>
#include <parser.h>

static char* ops[] = {
	"UNKNOWN",
	"XFRM_MSG_NEWSA",
	"XFRM_MSG_DELSA",
	"XFRM_MSG_GETSA",
};

struct json_map sa_lifetime_cfg[] = {
	JSON_MAP_ENTRY("refcnt", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("soft_byte_limit", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("hard_byte_limit", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("soft_packet_limit", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("hard_byte_limit", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("soft_add_expires_seconds", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("hard_add_expires_seconds", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("soft_use_expires_seconds", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("hard_use_expires_seconds", JSON_INTEGER, TBD, 0, NULL),
};

struct json_map sa_data[] = {
	JSON_MAP_ENTRY("saddr", JSON_STRING, TBD, 0, NULL),
	JSON_MAP_ENTRY("daddr", JSON_STRING, TBD, 0, NULL),
	JSON_MAP_ENTRY("spi", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("proto", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("lifetime_cfg", JSON_OBJECT, TBD, ARRAY_SIZE(sa_lifetime_cfg), sa_lifetime_cfg), 
};

struct json_map sa_tree = JSON_COMMON_TOPLEVEL("xfrm", "sa", TBD_STRING, sa_data) ;


void xfrm_sa_change_cb(struct nl_cache *cache, struct nl_object *obj, int val, void *data)
{
	struct monitor_socket *s = data;
	struct xfrmnl_sa *sa = (struct xfrmnl_sa *)obj;
	json_t *output = NULL;
	char buf[256];
	struct nl_addr *saddr = xfrmnl_sa_get_saddr(sa);
	struct nl_addr *daddr = xfrmnl_sa_get_daddr(sa);

	output = json_object();
	val = json_string("xfrm");
	json_object_set(output, "protocol", val);
	json_object_set(output, "object", json_string("sa"));
	json_object_set(output, "op", json_string(ops[val]));
	data = json_object();
	json_object_set(output, "data", data);
	nl_addr2str(saddr, buf, 256);
	json_object_set(data, "saddr", json_string(buf));
	nl_addr2str(daddr, buf, 256);
	json_object_set(data, "daddr", json_string(buf));	
	print_json_event(json_dumps(output, JSON_COMPACT));
	json_decref(output);
	return;
}
