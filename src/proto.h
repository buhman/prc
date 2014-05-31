#pragma once

#include <sys/epoll.h>
#include "dll.h"

int
proto_register(int epfd,
               const char *node,
               const char *service,
               dll_t **owq);

int
proto_connect(const char *node,
              const char *service);

int
proto_read(struct epoll_event *ev);

int
proto_write(struct epoll_event *ev);

int
proto_parse_buf(struct epoll_event *ev,
                char *buf, size_t len);

int
proto_parse_line(struct epoll_event *ev,
                 char *buf, size_t len);
