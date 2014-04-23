AC_INIT([InklingReader], [0.6], [roel@roelserve.com])
AC_CONFIG_SRCDIR([src])
AM_INIT_AUTOMAKE([-Wall])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([src/main.c])
AC_PROG_CC
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([INKLINGREADER], [
	glib-2.0
	gtk+-3.0
	librsvg-2.0
	cairo
])

AC_CONFIG_FILES([
 Makefile
])

AC_OUTPUT
