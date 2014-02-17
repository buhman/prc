#pragma once

#include <sys/epoll.h>

#include "sll.h"

typedef struct event_handler event_handler_t;
typedef int (eh_fptr_t)(struct epoll_event*);

struct event_handler {
  int epfd;
  int fd;
  eh_fptr_t *rf;
  eh_fptr_t *wf;
  sll_t *wq;
};

int
event_add(int epfd,
          int fd,
          uint32_t events,
          eh_fptr_t *rf,
          eh_fptr_t *wf,
          sll_t *wq,
          struct epoll_event *ev);

int
event_del(int epfd,
          struct epoll_event *ev);
