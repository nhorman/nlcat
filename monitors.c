#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include <netlink/xfrm/sa.h>
#include <netlink/xfrm/selector.h>
#include <monitors.h>
#include <xfrm_monitor.h>

static struct nl_cache_mngr* mngrs[MAX_LINKS] ={ NULL };
static pthread_t tids[MAX_LINKS];
static size_t num_mngrs = 0;
static bool teardown = false;

static pthread_mutex_t event_lock = PTHREAD_MUTEX_INITIALIZER;

static struct monitor_socket monitors[] = {
	{
		.protocol = NETLINK_XFRM,
		.alloc_cache = xfrmnl_sa_alloc_cache,
		.change_cb = xfrm_sa_change_cb,
	},
	{
		.protocol = 0,
		.change_cb = NULL,
	}
};


static int setup_cache(struct monitor_socket *s)
{
	int rc = -1;

	s->cache = NULL;
	rc = s->alloc_cache(NULL, &s->cache);
        if (rc)
                goto out;

	if (!mngrs[s->protocol]) {
		rc = nl_cache_mngr_alloc(NULL, s->protocol, 0, &mngrs[s->protocol]);
		if (!mngrs[s->protocol])
			goto out;
		num_mngrs++;
	}

	rc = nl_cache_mngr_add_cache(mngrs[s->protocol], s->cache, s->change_cb, s);
	if (rc) {
		goto out;
	}
out:
        return rc;

}

int setup_monitors()
{
	int i;
	int rc = -1;
	for(i=0; monitors[i].change_cb != NULL; i++) {
		rc = setup_cache(&monitors[i]);
		if (rc)
			goto out;
	} 
out:
	if (rc)
		teardown_monitors();
	return rc;
}

int teardown_monitors()
{
	int i;
	teardown = true;
	for(i=0; i<MAX_LINKS; i++) {
		if (tids[i])
			pthread_join(tids[i], NULL);
	}
	for (i=0; i<MAX_LINKS; i++) {
		if (mngrs[i]) 
			nl_cache_mngr_free(mngrs[i]);
	}
	return 0;
}

static void* monitor_cache(void *data)
{
	struct nl_cache_mngr *m = data;
	int rc;
	while (!teardown) {
		rc = nl_cache_mngr_poll(m, 1000);
		if (rc < 0) {
			fprintf(stderr, "Error polling cache %p\n", m);
			break;
		}
	}
	pthread_exit(NULL);
}

int start_monitoring()
{
	int i;
	int rc;
	for(i=0; i<MAX_LINKS; i++) {
		tids[i] = 0;
		if (mngrs[i]) {
			rc = pthread_create(&tids[i], NULL, monitor_cache, mngrs[i]);
			if (rc)
				goto out;
		}
	}
out:
	return rc;
}

void print_json_event(const char *event)
{
	pthread_mutex_lock(&event_lock);
	fprintf(stdout, "%s\n", event);
	fflush(stdout);
	pthread_mutex_unlock(&event_lock);
}
