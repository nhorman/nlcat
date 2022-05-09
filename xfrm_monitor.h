#ifndef __XFRM_MONITORS_H__
#define __XFRM_MONITORS_H__
#include <monitors.h>

void xfrm_sa_change_cb(struct nl_cache *, struct nl_object *, int val, void *data);
void xfrm_sp_change_cb(struct nl_cache *, struct nl_object *, int val, void *data);

#endif
