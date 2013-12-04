include ../common.am

AM_CFLAGS += \
	-I$(top_srcdir)/lib

AM_LIBADD = \
	$(top_builddir)/lib/libprc.la

lib_LTLIBRARIES = libtime.la libfacts.la

libtime_la_SOURCES = \
	time.c

libfacts_la_SOURCES = \
	facts.c facts.h
