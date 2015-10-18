EXTRA_DIST = \
	version.h.in

AM_CFLAGS = \
	$(WARNING_CFLAGS) \
	$(GSASL_CFLAGS) \
	$(GNUTLS_CFLAGS) \
	-I$(top_srcdir)/lib/buh/include \
	-I$(top_srcdir)/lib/bus/include \
	-I$(top_srcdir)/include

bin_PROGRAMS = prc prcc

prc_SOURCES = \
	prc.h prc.c \
	proto.h proto.c \
	method.h method.c \
	handler.h handler.c

prc_LDADD = \
	$(top_builddir)/lib/buh/lib/libbuh.la \
	$(top_builddir)/lib/bus/lib/bus/libbus.la \
	$(top_builddir)/lib/prc/libprc.la

prcc_SOURCES = \
	cli/cli.c

prcc_LDADD = \
	$(top_builddir)/lib/buh/lib/libbuh.la \
	$(top_builddir)/lib/bus/lib/bus/libbus.la \
	$(top_builddir)/lib/prc/libprc.la
