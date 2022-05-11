#ifndef __ROUTE_MONITOR_H__
#define __ROUTE_MONITOR_H__
void route_change_cb(struct nl_cache *, struct nl_object *, int val, void *data);
void addr_change_cb(struct nl_cache *, struct nl_object *, int val, void *data);
void link_change_cb(struct nl_cache *, struct nl_object *, int val, void *data);
#endif

