# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

AC_INIT([InklingReader], [0.6], [roel@roelserve.com])
AM_SILENT_RULES([yes])
AC_CONFIG_AUX_DIR([scripts])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE([-Wall -Werror gnu])

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
