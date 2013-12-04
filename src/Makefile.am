include ../common.am

bin_PROGRAMS = prc

prc_CFLAGS = \
	$(AM_CFLAGS) \
	$(WARNING_CFLAGS) \
	$(GSASL_CFLAGS) \
	-I$(top_srcdir)/lib

prc_SOURCES = \
	epoll.c epoll.h \
	proto.c proto.h \
	sasl.c sasl.h \
	plugin.c plugin.h \
	prc.c

prc_LDADD = \
	$(GSASL_LIBS) \
	$(top_builddir)/lib/libprc.la \
	-ldl
