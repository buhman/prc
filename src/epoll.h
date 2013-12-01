#pragma once

#include "dbuf.h"

int
epoll_connect(int efd,
	      const char *node,
	      const char *service);

int
epoll_input(int efd);

static int
epoll_add(int efd,
	  int ifd,
	  dbuf_readf_t readf);
