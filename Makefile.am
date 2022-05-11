AUTOMAKE_OPTIONS = subdir-objects 
ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = COPYING autogen.sh


CFLAGS += -Wall -I. $(PTHREAD_CFLAGS) $(LIBJANSSON_CFLAGS)
AM_CPPFLAGS = -I${top_srcdir} -W -Wall $(PTHREAD_CFLAGS) $(LIBJANSSON_CFLAGS) $(LIBNL_CFLAGS) $(LIBNLROUTE_CFLAGS) $(LIBNLXFRM_CFLAGS)
COMMONLIB = $(LIBNLROUTE_LIBS) $(LIBNLXFRM_LIBS) $(LIBJANSSON_LIBS) $(LIBS) 
STATICLIBS= -lnl-3 -lnl-route-3 -lnl-xfrm-3  -ljansson

sbin_PROGRAMS= nlcat 
nlcat_SOURCES = main.c monitors.c xfrm_monitor.c route_monitor.c
# Note: We need to use --whole-archive here to pull in the constructors for
# runtime registration

if STATIC
nlcat_LDFLAGS = -Wl,-Bstatic -Wl,--whole-archive $(STATICLIBS) -Wl,--no-whole-archive -Wl,-Bdynamic $(PTHREAD_LIBS)
else
nlcat_LDFLAGS = $(COMMONLIB) $(PTHREAD_LIBS)
endif

nlcat_DEPENDENCIES = $(COMMON_LIBS)

