include ../common.am

AM_CFLAGS += \
	-I$(top_srcdir)/lib

AM_LIBADD = \
	$(top_builddir)/lib/libprc.la

lib_LTLIBRARIES = \
	libtime.la \
	libctcp.la \
	libfacts.la \
	libchess.la \
	librot13.la \
	libecho.la \
	libmagic.la \
	libdice.la

libtime_la_SOURCES = \
	time.c

libfacts_la_SOURCES = \
	facts.c facts.h

libctcp_la_SOURCES = \
	ctcp.c

libchess_la_SOURCES = \
	chess.c chess.h

librot13_la_SOURCES = \
	rot13.c

libecho_la_SOURCES = \
	echo.c

libmagic_la_SOURCES = \
	magic.c

libdice_la_SOURCES = \
	dice.c
