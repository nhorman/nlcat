#include <jansson.h>
#include <netlink/xfrm/sa.h>
#include <netlink/xfrm/sp.h>
#include <netlink/xfrm/selector.h>
#include <netlink/xfrm/lifetime.h>
#include <monitors.h>
#include <xfrm_monitor.h>
#include <parser.h>

static char* sa_ops[] = {
	"UNKNOWN",
	"XFRM_MSG_NEWSA",
	"XFRM_MSG_DELSA",
	"XFRM_MSG_GETSA",
};

static char* sp_ops[] = {
	"UNKOWN",
	"XFRM_MSG_NEWSP",
	"XFRM_MSG_DELSP",
	"XFRM_MSG_GETSP",
};

static char *dirs[] = {
	"in",
	"out",
	"fwd",
};

static void fill_sel(struct xfrmnl_sel *sel, json_t *obj, char selsaddr[256], char seldaddr[256])
{
	struct nl_addr *saddr;
	struct nl_addr *daddr;

	memset(selsaddr, 0, 256);
	memset(seldaddr, 0, 256);	
	if (sel) {
		saddr = xfrmnl_sel_get_saddr(sel);
		daddr = xfrmnl_sel_get_daddr(sel);
		JSON_ASSIGN_STRING(obj, "saddr", nl_addr2str(saddr, selsaddr, 256));
		JSON_ASSIGN_STRING(obj, "daddr", nl_addr2str(daddr, seldaddr, 256));
		JSON_ASSIGN_INT(obj, "dport", xfrmnl_sel_get_dport(sel));
		JSON_ASSIGN_INT(obj, "dport_mask", xfrmnl_sel_get_dportmask(sel));
		JSON_ASSIGN_INT(obj, "sport", xfrmnl_sel_get_sport(sel));
		JSON_ASSIGN_INT(obj, "sport_mask", xfrmnl_sel_get_sportmask(sel));
		JSON_ASSIGN_INT(obj, "family", xfrmnl_sel_get_family(sel));
		JSON_ASSIGN_INT(obj, "prefixlen_d", xfrmnl_sel_get_prefixlen_d(sel));
		JSON_ASSIGN_INT(obj, "prefixlen_s", xfrmnl_sel_get_prefixlen_s(sel));
		JSON_ASSIGN_INT(obj, "proto", xfrmnl_sel_get_proto(sel));
		JSON_ASSIGN_INT(obj, "ifindex", xfrmnl_sel_get_ifindex(sel));
		JSON_ASSIGN_INT(obj, "user", xfrmnl_sel_get_userid(sel));
	}
}

void fill_lft(struct xfrmnl_ltime_cfg *ltime, json_t *obj)
{
	if (ltime) {
		JSON_ASSIGN_INT(obj, "soft_byte_limit", xfrmnl_ltime_cfg_get_soft_bytelimit(ltime));
		JSON_ASSIGN_INT(obj, "hard_byte_limit", xfrmnl_ltime_cfg_get_hard_bytelimit(ltime));
		JSON_ASSIGN_INT(obj, "soft_packet_limit", xfrmnl_ltime_cfg_get_soft_packetlimit(ltime));
		JSON_ASSIGN_INT(obj, "hard_packet_limit", xfrmnl_ltime_cfg_get_hard_packetlimit(ltime));
		JSON_ASSIGN_INT(obj, "soft_add_expires_limit", xfrmnl_ltime_cfg_get_soft_addexpires(ltime));
		JSON_ASSIGN_INT(obj, "hard_add_expires_limit", xfrmnl_ltime_cfg_get_hard_addexpires(ltime));
		JSON_ASSIGN_INT(obj, "soft_use_expires_limit", xfrmnl_ltime_cfg_get_soft_useexpires(ltime));
		JSON_ASSIGN_INT(obj, "hard_use_expires_limit", xfrmnl_ltime_cfg_get_hard_useexpires(ltime));
	}
	
}

void xfrm_sa_change_cb(struct nl_cache *cache __unused, struct nl_object *obj, int val, void *arg __unused)
{
	json_t *report;
	json_t *data;
	char *result;

	struct xfrmnl_sa *sa = (struct xfrmnl_sa *)obj;
	char srcaddr[256];
	char dstaddr[256];
	char selsaddr[256];
	char seldaddr[256];
	struct nl_addr *saddr = xfrmnl_sa_get_saddr(sa);
	struct nl_addr *daddr = xfrmnl_sa_get_daddr(sa);

	memset(srcaddr, 0, 256);
	memset(dstaddr, 0, 256);
	/* set the op */
	report = json_object();
	data = create_json_report(report, "xfrm", "sa", sa_ops[val]);

	/* set the general sa data */
	JSON_ASSIGN_STRING(data, "saddr", nl_addr2str(saddr, srcaddr, 256));	
	JSON_ASSIGN_STRING(data, "daddr", nl_addr2str(daddr, dstaddr, 256));
	JSON_ASSIGN_INT(data, "spi", xfrmnl_sa_get_spi(sa));
	JSON_ASSIGN_INT(data, "proto", xfrmnl_sa_get_proto(sa));
	JSON_ASSIGN_INT(data, "family", xfrmnl_sa_get_family(sa));
	JSON_ASSIGN_INT(data, "mode", xfrmnl_sa_get_mode(sa));
	JSON_ASSIGN_INT(data, "flags", xfrmnl_sa_get_flags(sa));
	JSON_ASSIGN_INT(data, "seq", xfrmnl_sa_get_seq(sa));
	JSON_ASSIGN_INT(data, "reqid", xfrmnl_sa_get_reqid(sa));


	/* Fill out the selector fields */
	fill_sel(xfrmnl_sa_get_sel(sa), create_json_child_object(data, "sel"), selsaddr, seldaddr);

	/* Fill in the lifetime config */
	fill_lft(xfrmnl_sa_get_lifetime_cfg(sa), create_json_child_object(data, "lifetime_cfg"));

	result = json_dumps(report, JSON_COMPACT);
	print_json_event(result);
	json_decref(report);
	free(result);
	return;
}

struct usrtmpl_storage {
	struct json_t *array;
	size_t num_strings;
	char *strings;
}; 

static void fill_usrtmpl_entry(struct xfrmnl_user_tmpl *utmpl, void *arg)
{
	json_t *new;
	struct usrtmpl_storage *array_store = arg;
	struct nl_addr *addr;
	int index = array_store->num_strings;
	array_store->num_strings++;
	array_store->strings = realloc(array_store->strings, 256 * array_store->num_strings);

	memset(&array_store->strings[index * 256], 0, 256);

	addr = xfrmnl_user_tmpl_get_saddr(utmpl);
	nl_addr2str(addr, &array_store->strings[index * 256], 256);

	new = json_object();

	JSON_ASSIGN_INT(new, "spi", xfrmnl_user_tmpl_get_spi(utmpl));
	JSON_ASSIGN_INT(new, "family", xfrmnl_user_tmpl_get_family(utmpl));
	JSON_ASSIGN_STRING(new, "saddr", &array_store->strings[index * 256]); 
	JSON_ASSIGN_INT(new, "reqid", xfrmnl_user_tmpl_get_reqid(utmpl));
	JSON_ASSIGN_INT(new, "mode", xfrmnl_user_tmpl_get_mode(utmpl));
	JSON_ASSIGN_INT(new, "share", xfrmnl_user_tmpl_get_share(utmpl));
	JSON_ASSIGN_INT(new, "optional", xfrmnl_user_tmpl_get_optional(utmpl));
	JSON_ASSIGN_INT(new, "aalgos", xfrmnl_user_tmpl_get_aalgos(utmpl));
	JSON_ASSIGN_INT(new, "ealgos", xfrmnl_user_tmpl_get_ealgos(utmpl));
	JSON_ASSIGN_INT(new, "calgos", xfrmnl_user_tmpl_get_calgos(utmpl));

	json_array_append(array_store->array, new);
	json_decref(new); 
}

void xfrm_sp_change_cb(struct nl_cache *cache __unused, struct nl_object *obj, int val, void *arg __unused)
{
	struct xfrmnl_sp *sp = (struct xfrmnl_sp *)obj;
	unsigned int len;
	unsigned int exttype;
	unsigned int alg;
	unsigned int doi;
	unsigned int ctx_len;
	char *ctx_str = NULL;
	char selsaddr[256];
	char seldaddr[256];
	json_t  *usrtmpl;
	json_t *report;
	json_t *data;
	json_t *sec_ctx;
	char *result;
	struct usrtmpl_storage array_store;

	/* set the op*/
	report = json_object();
	data = create_json_report(report, "xfrm", "sp", sp_ops[val]);

	/* set top level data */
	JSON_ASSIGN_INT(data, "priority", xfrmnl_sp_get_priority(sp));
	JSON_ASSIGN_INT(data, "index", xfrmnl_sp_get_index(sp));
	JSON_ASSIGN_STRING(data, "dir", dirs[xfrmnl_sp_get_dir(sp)]);
	JSON_ASSIGN_INT(data, "action", xfrmnl_sp_get_action(sp));
	JSON_ASSIGN_INT(data, "flags", xfrmnl_sp_get_flags(sp));
	JSON_ASSIGN_INT(data, "share", xfrmnl_sp_get_share(sp));
	JSON_ASSIGN_INT(data, "userpolicy_type", xfrmnl_sp_get_userpolicy_type(sp));

	/* fill in the selector */
	fill_sel(xfrmnl_sp_get_sel(sp), create_json_child_object(data, "sel"), selsaddr, seldaddr);

	/* And the lifetime config */
	fill_lft(xfrmnl_sp_get_lifetime_cfg(sp), create_json_child_object(data, "lifetime_cfg"));	

	/* And the user security context */
	sec_ctx = create_json_child_object(data, "sec_ctx");
	if (!xfrmnl_sp_get_sec_ctx(sp, &len, &exttype, &alg, &doi, &ctx_len, NULL)) {
		ctx_str = alloca(ctx_len+1);
		memset(ctx_str, 0, ctx_len+1);
		xfrmnl_sp_get_sec_ctx(sp, &len, &exttype, &alg, &doi, &ctx_len, ctx_str);

		JSON_ASSIGN_INT(sec_ctx, "len", len);
		JSON_ASSIGN_INT(sec_ctx, "exttype", exttype);
		JSON_ASSIGN_INT(sec_ctx, "alg", alg);
		JSON_ASSIGN_INT(sec_ctx, "doi", doi);
		JSON_ASSIGN_STRING(sec_ctx, "ctx_str", ctx_str);
	}

	/* fill out the usertmp array */
	usrtmpl = create_json_child_array(data, "usrtmpl_list"); 
	array_store.strings = NULL;
	array_store.num_strings = 0;
	array_store.array = usrtmpl;
	
	xfrmnl_sp_foreach_usertemplate(sp, fill_usrtmpl_entry, &array_store);

	result = json_dumps(report, JSON_COMPACT);
	print_json_event(result);
	json_decref(report);
	free(result);
	free(array_store.strings);

	return;
}

