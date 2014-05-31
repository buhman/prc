bin_PROGRAMS = prc

prc_CFLAGS = \
	$(AM_CFLAGS) \
	$(WARNING_CFLAGS) \
	$(GSASL_CFLAGS) \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/bdb/src

prc_SOURCES = \
	event.c event.h \
	proto.c proto.h \
	term.c term.h \
	handler.c handler.h \
	sasl.c sasl.h \
	buf.c buf.h \
	plugin.c plugin.h \
	prc.c

prc_LDADD = \
	$(GSASL_LIBS) \
	$(top_builddir)/lib/libprc.la \
	$(top_builddir)/bdb/src/libbdb.la \
	-ldl

prc_LDFLAGS = \
	-Wl,-rpath -Wl,/home/zack/prc/plugins/.libs
