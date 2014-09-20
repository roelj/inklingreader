# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

AC_INIT([InklingReader], [0.8], [roel@roelserve.com])
AC_CONFIG_AUX_DIR([scripts])
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([-Wall -Werror])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([src/main.c])

AC_PROG_CC
AM_PROG_CC_C_O
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h stdio.h])
AC_CONFIG_FILES([Makefile])

case $host in
*-linux*)
AC_SUBST(OS_LINUX)
AM_CONDITIONAL([OS_LINUX], [true])
;;
*-mingw*)
AC_SUBST(OS_LINUX)
AM_CONDITIONAL([OS_LINUX], [false])
;;
*-cygwin*)
AC_SUBST(OS_LINUX)
AM_CONDITIONAL([OS_LINUX], [true])
;;
*-darwin*)
AC_SUBST(OS_LINUX)
AM_CONDITIONAL([OS_LINUX], [true])
;;
*)
AC_MSG_ERROR([InklingReader is not (yet) supported on $host.])
esac

AC_SUBST(ENABLE_GCOV_OPTION)
AC_ARG_ENABLE([gcov],
AC_HELP_STRING([--enable-gcov],
[enable GCOV suport @<:@default=no@:>@]),
AM_CONDITIONAL([ENABLE_GCOV_OPTION], [true]), AM_CONDITIONAL([ENABLE_GCOV_OPTION], [false]))

PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([gtk], [gtk+-3.0])
PKG_CHECK_MODULES([glib], [glib-2.0])
PKG_CHECK_MODULES([rsvg], [librsvg-2.0])
PKG_CHECK_MODULES([cairo], [cairo])

AC_OUTPUT
