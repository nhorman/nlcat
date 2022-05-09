#ifndef __MONOITOR_H__
#define __MONOITOR_H__
#include <netlink/xfrm/sa.h>
#include <netlink/xfrm/selector.h>

#define __unused __attribute__((unused))
struct monitor_socket
{
	int protocol;
	struct nl_cache *cache;
	int (*alloc_cache)(struct nl_sock *, struct nl_cache **);
	change_func_t change_cb;
};


int teardown_monitors();
int setup_monitors();
int start_monitoring();


void print_json_event(const char *event);

#endif

