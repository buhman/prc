include ../common.am

bin_PROGRAMS = prc

prc_CFLAGS = \
	$(AM_CFLAGS) \
	$(WARNING_CFLAGS) \
	$(GSASL_CFLAGS) \
	-I$(top_srcdir)/lib

prc_SOURCES = \
	event.c event.h \
	proto.c proto.h \
	prc.c

#sasl.c sasl.h
#plugin.c plugin.h

prc_LDADD = \
	$(GSASL_LIBS) \
	$(top_builddir)/lib/libprc.la \
	-ldl
