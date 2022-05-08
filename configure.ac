m4_include(m4/ax_pthread.m4)
AC_INIT([nlcat],[v0.0.1])
AC_PREREQ([2.69])dnl
AC_CONFIG_HEADERS(config.h,[],[])

AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB

AC_PROG_EGREP

#set this for PKG_CHECK_MODULES below

# Arguments to present via configure script
AX_PTHREAD([have_pthread=yes],[have_pthread=no])

AS_IF([test "x$have_pthread" = "xyes"],[
	AC_DEFINE(HAVE_PTHREAD,[],[We support pthreads])],[
	AC_MSG_ERROR([Missing pthread support])])

AC_CHECK_FUNC(getopt_long,[AC_DEFINE(HAVE_GETOPT_LONG,[],[Is the getopt_long function available])],[AC_MSG_NOTICE([Falling back to getopt])])

PKG_CHECK_MODULES([LIBNL], [libnl-3.0], [], [AC_MSG_ERROR([libnl-3 is required])])
PKG_CHECK_MODULES([LIBNLROUTE], [libnl-route-3.0], [], [AC_MSG_ERROR([libnl-route-3 is required])])
PKG_CHECK_MODULES([LIBNLXFRM], [libnl-xfrm-3.0], [], [AC_MSG_ERROR([libnl-xfrm-3 is required])])
PKG_CHECK_MODULES([LIBJANSSON], [jansson], [], [AC_MSG_ERROR([libjansson is required])])

AC_MSG_NOTICE()
AC_MSG_NOTICE([nlcat Version:     $VERSION])
AC_MSG_NOTICE([Target:                 $target])
AC_MSG_NOTICE([Installation prefix:    $prefix])
AC_MSG_NOTICE([Compiler:               $CC])
AC_MSG_NOTICE([Compiler flags: $CFLAGS])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
