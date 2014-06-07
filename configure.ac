# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([prc],[m4_esyscmd(./git-version)],[zack@buhman.org],[https://buhman.org/cgit/prc.git])
AC_CONFIG_SRCDIR([lib/prc.h], [bdb/src/bdb.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability subdir-objects silent-rules color-tests dist-xz tar-ustar])

AS_IF([! test -n "$VERSION"], [
         AC_MSG_ERROR([git-version failed])
])

AC_SUBST([PRC_VERSION], $VERSION)

AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED([CANONICAL_HOST], "$host", [Canonical host string.])

AM_SILENT_RULES([yes])

### checks ###

# CC

AC_PROG_CC
AC_PROG_CC_C99
AC_GNU_SOURCE

# pkg-config

PKG_PROG_PKG_CONFIG

#### compiler flags ####

AX_APPEND_COMPILE_FLAGS(
        [-Wall -W -Wextra -pipe -Wno-long-long -Wno-overlength-strings -Wunsafe-loop-optimizations -Wundef -Wlogical-op -Wsign-compare -Wmissing-include-dirs -Wold-style-definition -Wpointer-arith -Winit-self -Wdeclaration-after-statement -Wfloat-equal -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls -Wmissing-declarations -Wmissing-noreturn -Wshadow -Wendif-labels -Wcast-align -Wstrict-aliasing -Wwrite-strings -Wno-unused-parameter -Wno-error=unused-function -ffast-math -fno-common -fdiagnostics-show-option],
        [], [-pedantic -pedantic-errors -Werror])

#### libtool ####

LT_PREREQ(2.4)
LT_INIT([dlopen disable-static])

#### environment ####

AC_HEADER_STDC
AC_CHECK_HEADERS([sys/socket.h sys/epoll.h])

PKG_CHECK_MODULES(GSASL, [libgsasl])

#

AC_CONFIG_FILES([
Makefile
lib/Makefile
src/Makefile
plugins/Makefile
lib/version.h
])

AC_CONFIG_SUBDIRS([bdb])

AC_OUTPUT
