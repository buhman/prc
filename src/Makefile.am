EXTRA_DIST = \
	version.h.in

AM_CFLAGS = \
	$(WARNING_CFLAGS) \
	$(GSASL_CFLAGS) \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/bdb/src

bin_PROGRAMS = prc

prc_SOURCES = \
	controller.c controller.h \
	prc.c

prc_LDADD = \
	-ldl

prc_LDFLAGS = \
	-Wl,-rpath -Wl,./src/.libs

lib_LTLIBRARIES = \
	libworker.la

libworker_la_SOURCES = \
	event.c event.h \
	proto.c proto.h \
	term.c term.h \
	handler.c handler.h \
	sasl.c sasl.h \
	buf.c buf.h \
	plugin.c plugin.h \
	cfg.c cfg.h \
	network.c network.h \
	worker.c worker.h

libworker_la_LIBADD = \
	$(GSASL_LIBS) \
	$(top_builddir)/lib/libprc.la \
	$(top_builddir)/bdb/src/libbdb.la \
	-ldl

libworker_la_LDFLAGS = \
	-Wl,-rpath -Wl,./plugins/.libs
