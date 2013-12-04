bin_PROGRAMS = prc

AM_CFLAGS = \
	-pedantic \
	-D_GNU_SOURCE \
	$(WARNING_CFLAGS) \
	$(GSASL_CFLAGS) \
	-I$(top_srcdir)/lib

prc_SOURCES = \
	epoll.c epoll.h \
	proto.c proto.h \
	sasl.c sasl.h \
	dbuf.c dbuf.h \
	facts.c facts.h \
	plugin.c plugin.h \
	prc.c

prc_LDADD = \
	$(GSASL_LIBS) \
	$(top_builddir)/lib/libprc.la \
	-ldl
