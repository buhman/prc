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
	term.c term.h \
	handler.c handler.h \
	sasl.c sasl.h \
	prc.c


#plugin.c plugin.h

prc_LDADD = \
	$(GSASL_LIBS) \
	$(top_builddir)/lib/libprc.la \
	-ldl
