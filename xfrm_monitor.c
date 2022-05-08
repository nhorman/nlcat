#include <jansson.h>
#include <netlink/xfrm/sa.h>
#include <netlink/xfrm/selector.h>
#include <monitors.h>
#include <xfrm_monitor.h>

static char* ops[] = {
	"UNKNOWN",
	"XFRM_MSG_NEWSA",
	"XFRM_MSG_DELSA",
	"XFRM_MSG_GETSA",
};

void xfrm_sa_change_cb(struct nl_cache *cache, struct nl_object *obj, int val, void *data)
{
	struct monitor_socket *s = data;
	struct xfrmnl_sa *sa = (struct xfrmnl_sa *)obj;
	json_t *output = NULL;

	output = json_object();
	json_object_set(output, "protocol", json_string("xfrm"));
	json_object_set(output, "object", json_string("sa"));
	json_object_set(output, "op", json_string(ops[val]));
	print_json_event(json_dumps(output, JSON_COMPACT));
	json_decref(output);
	return;
}
