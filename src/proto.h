#pragma once

#include "sll.h"

extern sll_t *proto_cwq;

int
proto_register(int epfd,
               char *node,
               char *service);

int
proto_connect(const char *node,
              const char *service);

int
proto_read(struct epoll_event *ev);

int
proto_write(struct epoll_event *ev);
