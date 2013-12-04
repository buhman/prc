#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([prc], [0], [zack@buhman.org])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

#

AC_ENABLE_SHARED
AC_DISABLE_STATIC
AM_INIT_AUTOMAKE([1.11 foreign])
AM_SILENT_RULES([yes])
AC_PROG_CC_C99

#

LT_INIT([disable-static])

# Check for header files.
AC_CHECK_HEADERS([stdlib.h stdio.h stdbool.h unistd.h assert.h])
AC_CHECK_HEADERS([sys/socket.h sys/epoll.h])

# Check for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_CHECK_FUNCS([epoll_create1])

#

PKG_CHECK_MODULES(GSASL, [libgsasl])

#

AC_CONFIG_FILES([Makefile src/Makefile lib/Makefile plugins/Makefile])

AC_OUTPUT
