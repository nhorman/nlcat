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

static struct json_map xfrmnl_lft_cfg[] = {
	JSON_MAP_ENTRY("soft_byte_limit", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("hard_byte_limit", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("soft_packet_limit", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("hard_packet_limit", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("soft_add_expires_seconds", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("hard_add_expires_seconds", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("soft_use_expires_seconds", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("hard_use_expires_seconds", JSON_INTEGER, TBD, 0, NULL),
};

static struct json_map xfrmnl_sel[] ={
	JSON_MAP_ENTRY("saddr", JSON_STRING, TBD, 0, NULL),
	JSON_MAP_ENTRY("daddr", JSON_STRING, TBD, 0, NULL),
	JSON_MAP_ENTRY("dport", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("dport_mask", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("sport", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("sport_mask", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("family", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("prefixlen_d", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("prefixlen_s", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("proto", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("ifindex", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("user", JSON_INTEGER, TBD, 0, NULL),
};

static struct json_map sa_data[] = {
	JSON_MAP_ENTRY("saddr", JSON_STRING, TBD, 0, NULL),
	JSON_MAP_ENTRY("daddr", JSON_STRING, TBD, 0, NULL),
	JSON_MAP_ENTRY("spi", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("proto", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("family", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("mode", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("flags", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("seq", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("reqid", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("lifetime_cfg", JSON_OBJECT, TBD, ARRAY_SIZE(xfrmnl_lft_cfg), xfrmnl_lft_cfg), 
	JSON_MAP_ENTRY("sel", JSON_OBJECT, TBD, ARRAY_SIZE(xfrmnl_sel), xfrmnl_sel),
};

static struct json_map sa_tree[] = JSON_COMMON_TOPLEVEL("xfrm", "sa", TBD, sa_data);

static void fill_sel(struct xfrmnl_sel *sel)
{
	int dport, dport_mask;
	int sport, sport_mask;
	int selfam;
	int prefixlen_d, prefixlen_s;
	int selproto;
	int ifindex;
	int user;
	struct nl_addr *saddr;
	struct nl_addr *daddr;
	char selsaddr[256];
	char seldaddr[256];

	memset(selsaddr, 0, 256);
	memset(seldaddr, 0, 256);	
	if (!sel)
		parser_set_flags(sa_data, ARRAY_SIZE(sa_data), "sel", MAP_FLAG_EMPTY_OBJ);
	else {
		saddr = xfrmnl_sel_get_saddr(sel);
		daddr = xfrmnl_sel_get_daddr(sel);
		dport = xfrmnl_sel_get_dport(sel);
		dport_mask = xfrmnl_sel_get_dportmask(sel);
		sport = xfrmnl_sel_get_sport(sel);
		sport_mask = xfrmnl_sel_get_sportmask(sel);
		selfam = xfrmnl_sel_get_family(sel);
		prefixlen_d = xfrmnl_sel_get_prefixlen_d(sel);
		prefixlen_s = xfrmnl_sel_get_prefixlen_s(sel);
		selproto = xfrmnl_sel_get_proto(sel);
		ifindex = xfrmnl_sel_get_ifindex(sel);
		user = xfrmnl_sel_get_userid(sel);

		parser_set_val(xfrmnl_sel, ARRAY_SIZE(xfrmnl_sel), "saddr", nl_addr2str(saddr, selsaddr, 256));
		parser_set_val(xfrmnl_sel, ARRAY_SIZE(xfrmnl_sel), "daddr", nl_addr2str(daddr, seldaddr, 256));
		parser_set_val(xfrmnl_sel, ARRAY_SIZE(xfrmnl_sel), "dport", &dport);
		parser_set_val(xfrmnl_sel, ARRAY_SIZE(xfrmnl_sel), "dport_mask", &dport_mask);
		parser_set_val(xfrmnl_sel, ARRAY_SIZE(xfrmnl_sel), "sport", &sport);
		parser_set_val(xfrmnl_sel, ARRAY_SIZE(xfrmnl_sel), "sport_mask", &sport_mask);
		parser_set_val(xfrmnl_sel, ARRAY_SIZE(xfrmnl_sel), "family", &selfam);
		parser_set_val(xfrmnl_sel, ARRAY_SIZE(xfrmnl_sel), "prefixlen_d", &prefixlen_d);
		parser_set_val(xfrmnl_sel, ARRAY_SIZE(xfrmnl_sel), "prefixlen_s", &prefixlen_s);
		parser_set_val(xfrmnl_sel, ARRAY_SIZE(xfrmnl_sel), "proto", &selproto);
		parser_set_val(xfrmnl_sel, ARRAY_SIZE(xfrmnl_sel), "ifindex", &ifindex);
		parser_set_val(xfrmnl_sel, ARRAY_SIZE(xfrmnl_sel), "user", &user);
	}
}

void fill_lft(struct xfrmnl_ltime_cfg *ltime)
{
	int sblim, hblim;
	int splim, hplim;
	int saexp, haexp;
	int suexp, huexp;

	if (!ltime)
		parser_set_flags(sa_data, ARRAY_SIZE(sa_data), "lifetime_cfg", MAP_FLAG_EMPTY_OBJ);
	else {
		sblim = xfrmnl_ltime_cfg_get_soft_bytelimit(ltime);
		hblim = xfrmnl_ltime_cfg_get_hard_bytelimit(ltime);
		splim = xfrmnl_ltime_cfg_get_soft_packetlimit(ltime);
		hplim = xfrmnl_ltime_cfg_get_hard_packetlimit(ltime);
		saexp = xfrmnl_ltime_cfg_get_soft_addexpires(ltime);
		haexp = xfrmnl_ltime_cfg_get_hard_addexpires(ltime);
		suexp = xfrmnl_ltime_cfg_get_soft_useexpires(ltime);
		huexp = xfrmnl_ltime_cfg_get_hard_useexpires(ltime);

		parser_set_val(xfrmnl_lft_cfg, ARRAY_SIZE(xfrmnl_lft_cfg), "soft_byte_limit", &sblim);
		parser_set_val(xfrmnl_lft_cfg, ARRAY_SIZE(xfrmnl_lft_cfg), "hard_byte_limit", &hblim);
		parser_set_val(xfrmnl_lft_cfg, ARRAY_SIZE(xfrmnl_lft_cfg), "soft_packet_limit", &splim);
		parser_set_val(xfrmnl_lft_cfg, ARRAY_SIZE(xfrmnl_lft_cfg), "hard_packet_limit", &hplim);
		parser_set_val(xfrmnl_lft_cfg, ARRAY_SIZE(xfrmnl_lft_cfg), "soft_add_expires_limit", &saexp);
		parser_set_val(xfrmnl_lft_cfg, ARRAY_SIZE(xfrmnl_lft_cfg), "hard_add_expires_limit", &haexp);
		parser_set_val(xfrmnl_lft_cfg, ARRAY_SIZE(xfrmnl_lft_cfg), "soft_use_expires_limit", &suexp);
		parser_set_val(xfrmnl_lft_cfg, ARRAY_SIZE(xfrmnl_lft_cfg), "hard_use_expires_limit", &huexp);
	}
	
}

void xfrm_sa_change_cb(struct nl_cache *cache __unused, struct nl_object *obj, int val, void *data)
{
	struct monitor_socket *s __unused = data;
	char *result;
	struct xfrmnl_sa *sa = (struct xfrmnl_sa *)obj;
	char srcaddr[256];
	char dstaddr[256];
	int spi =  xfrmnl_sa_get_spi(sa);
	int proto = xfrmnl_sa_get_proto(sa);
	int family = xfrmnl_sa_get_family(sa);
	int mode = xfrmnl_sa_get_mode(sa);
	int flags = xfrmnl_sa_get_flags(sa);
	int seq = xfrmnl_sa_get_seq(sa);
	int reqid = xfrmnl_sa_get_reqid(sa);
	struct nl_addr *saddr = xfrmnl_sa_get_saddr(sa);
	struct nl_addr *daddr = xfrmnl_sa_get_daddr(sa);
	struct xfrmnl_sel *sel = xfrmnl_sa_get_sel(sa);
	struct xfrmnl_ltime_cfg *ltime = xfrmnl_sa_get_lifetime_cfg(sa);

	memset(srcaddr, 0, 256);
	memset(dstaddr, 0, 256);
	/* set the op */
	parser_set_val(sa_tree, ARRAY_SIZE(sa_tree), "op", sa_ops[val]);

	/* set the general sa data */
	parser_set_val(sa_data, ARRAY_SIZE(sa_data), "saddr", nl_addr2str(saddr, srcaddr, 256));
	parser_set_val(sa_data, ARRAY_SIZE(sa_data), "daddr", nl_addr2str(daddr, dstaddr, 256));
	parser_set_val(sa_data, ARRAY_SIZE(sa_data), "spi", &spi);
	parser_set_val(sa_data, ARRAY_SIZE(sa_data), "proto", &proto);
	parser_set_val(sa_data, ARRAY_SIZE(sa_data), "family", &family);
	parser_set_val(sa_data, ARRAY_SIZE(sa_data), "mode", &mode);
	parser_set_val(sa_data, ARRAY_SIZE(sa_data), "flags", &flags);
	parser_set_val(sa_data, ARRAY_SIZE(sa_data), "seq", &seq);
	parser_set_val(sa_data, ARRAY_SIZE(sa_data), "reqid", &reqid);


	/* Fill out the selector fields */
	fill_sel(sel);

	/* Fill in the lifetime config */
	fill_lft(ltime);

	result = compile_json_string(sa_tree, ARRAY_SIZE(sa_tree));
	print_json_event(result);
	parser_reset_tmpl(sa_tree, ARRAY_SIZE(sa_tree));
	free(result);
	return;
}

static struct json_map xfrmnl_user_sec[] = {
	JSON_MAP_ENTRY("len", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("exttype", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("alg", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("doi", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("ctx_str", JSON_STRING, TBD, 0, NULL),
};

static struct json_map sp_data[] = {
	JSON_MAP_ENTRY("sel", JSON_OBJECT, TBD, ARRAY_SIZE(xfrmnl_sel), xfrmnl_sel),
	JSON_MAP_ENTRY("lifetime_cfg", JSON_OBJECT, TBD, ARRAY_SIZE(xfrmnl_lft_cfg), xfrmnl_lft_cfg),
	JSON_MAP_ENTRY("priority", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("index", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("dir", JSON_STRING, TBD, 0, NULL),
	JSON_MAP_ENTRY("action", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("flags", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("share", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("user_sec_ctx", JSON_OBJECT, TBD, ARRAY_SIZE(xfrmnl_user_sec), xfrmnl_user_sec),
	JSON_MAP_ENTRY("userpolicy_type", JSON_INTEGER, TBD, 0, NULL),
	JSON_MAP_ENTRY("mark", JSON_INTEGER, TBD, 0, NULL),
};

static struct json_map sp_tree[] = JSON_COMMON_TOPLEVEL("xfrm", "sp", TBD, sp_data);

void xfrm_sp_change_cb(struct nl_cache *cache __unused, struct nl_object *obj, int val, void *data __unused)
{
	struct xfrmnl_sp *sp = (struct xfrmnl_sp *)obj;
	struct xfrmnl_sel *sel = xfrmnl_sp_get_sel(sp);
	struct xfrmnl_ltime_cfg *ltime = xfrmnl_sp_get_lifetime_cfg(sp);
	int prio = xfrmnl_sp_get_priority(sp);
	int index = xfrmnl_sp_get_index(sp);
	int dir = xfrmnl_sp_get_dir(sp);
	int action = xfrmnl_sp_get_action(sp);
	int flags = xfrmnl_sp_get_flags(sp);
	int share = xfrmnl_sp_get_share(sp);
	unsigned int len;
	unsigned int exttype;
	unsigned int alg;
	unsigned int doi;
	unsigned int ctx_len;
	char *ctx_str = NULL;
	int uptype = xfrmnl_sp_get_userpolicy_type(sp);
	char *result;

	/* set the op*/
        parser_set_val(sp_tree, ARRAY_SIZE(sp_tree), "op", sp_ops[val]);

	/* set top level data */
	parser_set_val(sp_tree, ARRAY_SIZE(sp_tree), "priority", &prio);
	parser_set_val(sp_tree, ARRAY_SIZE(sp_tree), "index", &index);
	parser_set_val(sp_tree, ARRAY_SIZE(sp_tree), "dir", dirs[dir]);
	parser_set_val(sp_tree, ARRAY_SIZE(sp_tree), "action", &action);
	parser_set_val(sp_tree, ARRAY_SIZE(sp_tree), "flags", &flags);
	parser_set_val(sp_tree, ARRAY_SIZE(sp_tree), "share", &share);
	parser_set_val(sp_tree, ARRAY_SIZE(sp_tree), "userpolicy_type", &uptype);
	
	/* fill in the selector */
	fill_sel(sel);

	/* And the lifetime config */
	fill_lft(ltime);	

	/* And the user security context */
	if (xfrmnl_sp_get_sec_ctx(sp, &len, &exttype, &alg, &doi, &ctx_len, NULL)) {
		parser_set_flags(sp_data, ARRAY_SIZE(sp_data), "user_sec_ctx", MAP_FLAG_EMPTY_OBJ);
	} else {
		ctx_str = alloca(ctx_len+1);
		memset(ctx_str, 0, ctx_len+1);
		xfrmnl_sp_get_sec_ctx(sp, &len, &exttype, &alg, &doi, &ctx_len, ctx_str);
		parser_set_val(xfrmnl_user_sec, ARRAY_SIZE(xfrmnl_user_sec), "len", &len);
		parser_set_val(xfrmnl_user_sec, ARRAY_SIZE(xfrmnl_user_sec), "exttype", &exttype);
		parser_set_val(xfrmnl_user_sec, ARRAY_SIZE(xfrmnl_user_sec), "alg", &alg);
		parser_set_val(xfrmnl_user_sec, ARRAY_SIZE(xfrmnl_user_sec), "doi", &doi);
		parser_set_val(xfrmnl_user_sec, ARRAY_SIZE(xfrmnl_user_sec), "ctx_str", ctx_str);
	}

	result = compile_json_string(sp_tree, ARRAY_SIZE(sp_tree));
	print_json_event(result);
	parser_reset_tmpl(sp_tree, ARRAY_SIZE(sp_tree));
	free(result);
	return;
}

