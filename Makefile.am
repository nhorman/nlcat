AUTOMAKE_OPTIONS = subdir-objects 
ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = COPYING autogen.sh


CFLAGS += -Wall -I. $(PTHREAD_CFLAGS) $(LIBJANSSON_CFLAGS)
AM_CPPFLAGS = -I${top_srcdir} -W -Wall $(PTHREAD_CFLAGS) $(LIBJANSSON_CFLAGS) $(LIBNL_CFLAGS) $(LIBNLROUTE_CFLAGS) $(LIBNLXFRM_CFLAGS)
COMMONLIB = $(LIBNL_LIBS) $(LIBNLROUTE_LIBS) $(LIBNLXFRM_LIBS) $(LIBJANSSON_LIBS) $(LIBS) $(PTHREAD_LIBS)

sbin_PROGRAMS= nlcat 
nlcat_SOURCES = main.c monitors.c xfrm_monitor.c parser.c
# Note: We need to use --whole-archive here to pull in the constructors for
# runtime registration

nlcat_LDFLAGS = $(COMMONLIB)

nlcat_DEPENDENCIES = $(COMMON_LIBS)

