AM_CFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/bdb/src

LIBS = $(top_builddir)/lib/libprc.la

lib_LTLIBRARIES = \
	libtime.la \
	libctcp.la \
	libfacts.la \
	libchess.la \
	librot13.la \
	libecho.la \
	libmagic.la \
	libdice.la \
	libfullwidth.la \
	libquote.la \
	libsegfault.la \
	libcourier.la \
	libsysinfo.la \
	libpoll.la \
	librps.la \
	libweight.la

libtime_la_SOURCES = time.c

libfacts_la_SOURCES = facts.c facts.h

libctcp_la_SOURCES = ctcp.c

libchess_la_SOURCES = chess.c chess.h

librot13_la_SOURCES = rot13.c

libecho_la_SOURCES = echo.c

libmagic_la_SOURCES = magic.c

libdice_la_SOURCES = dice.c

libfullwidth_la_SOURCES = fullwidth.c

libquote_la_SOURCES = quote.c
libquote_la_LIBADD = $(LIBS) $(top_builddir)/bdb/src/libbdb.la

libsegfault_la_SOURCES = segfault.c

libcourier_la_SOURCES = courier.c
libcourier_la_LIBADD = $(LIBS) $(PTHREAD_LIBS)
libcourier_la_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS)

libsysinfo_la_SOURCES = sysinfo.c

libpoll_la_SOURCES = poll.c poll.h

librps_la_SOURCES = rps.c rps.h

libweight_la_SOURCES = weight.c weight.h weight_handler.c
