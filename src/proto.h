#pragma once

int
proto_connect(const char *node,
              const char *service);

int
proto_read(struct epoll_event *ev);
