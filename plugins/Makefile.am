include ../common.am

AM_CFLAGS += \
	-I$(top_srcdir)/lib

AM_LIBADD = \
	$(top_builddir)/lib/libprc.la

lib_LTLIBRARIES = \
	libtime.la \
	libctcp.la \
	libfacts.la \
	libchess.la

libtime_la_SOURCES = \
	time.c

libfacts_la_SOURCES = \
	facts.c facts.h

libctcp_la_SOURCES = \
	ctcp.c

libchess_la_SOURCES = \
	chess.c chess.h
