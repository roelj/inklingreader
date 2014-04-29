AC_INIT([InklingReader], [0.6], [roel@roelserve.com])
AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([src/main.c])
AC_PROG_CC
AM_PROG_CC_C_O

AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h stdio.h])

PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([INKLINGREADER], [
	glib-2.0
	gtk+-3.0
	librsvg-2.0
	cairo
])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
