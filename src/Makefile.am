bin_PROGRAMS = prc

AM_CFLAGS = -pedantic -D_GNU_SOURCE $(WARNING_CFLAGS) $(GSASL_CFLAGS)

prc_SOURCES = \
	epoll.c epoll.h \
	proto.c proto.h \
	sasl.c sasl.h \
	dbuf.c dbuf.h \
	sll.c sll.h \
	prc.c

prc_LDADD = \
	$(GSASL_LIBS)
