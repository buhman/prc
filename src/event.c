#include <stdlib.h>
#include <stdint.h>

#include <sys/epoll.h>

#include "sll.h"
#include "event.h"

int
event_add(int epfd,
          int fd,
          uint32_t events,
          eh_fptr_t *func)
{
  int err;
  struct epoll_event *ev;
  struct event_handler *eh;
  sll_t *wq;

  {
    if (EPOLLIN & events)
      wq = malloc(sizeof(sll_t));
    else
      wq = NULL;
  }

  {
    eh = malloc(sizeof(struct event_handler));
    eh->epfd = epfd;
    eh->func = func;
    eh->fd = fd;
    eh->wq = wq;

    ev = malloc(sizeof(struct epoll_event));
    ev->events = events;
    ev->data.ptr = eh;
  }

  {
    err = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, ev);
    if (err < 0) {
      free(eh);
      free(ev);
      return err;
    }
  }

  return 0;
}

int
event_del(int epfd,
          struct epoll_event **ev)
{
  int err;
  event_handler_t *eh;

  eh = (event_handler_t*)(*ev)->data.ptr;

  err = epoll_ctl(epfd, EPOLL_CTL_DEL, eh->fd, NULL);
  if (err < 0)
    return err;

  if (eh->wq)
    free(eh->wq);

  free(eh);
  free(*ev);

  *ev = NULL;

  return 0;
}
