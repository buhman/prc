#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <sys/epoll.h>

#include "sll.h"
#include "event.h"

int
event_add(int epfd,
          int fd,
          uint32_t events,
          eh_fptr_t *rf,
          eh_fptr_t *wf,
          sll_t *wq,
          struct epoll_event *oev)
{
  int err;
  event_handler_t *eh;
  struct epoll_event ev;

  {
    eh = malloc(sizeof(event_handler_t));
    eh->epfd = epfd;
    eh->rf = rf;
    eh->wf = wf;
    eh->fd = fd;
    eh->wq = wq;

    ev.events = events;
    ev.data.ptr = eh;
  }

  {
    err = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    if (err < 0) {
      free(eh);
      return err;
    }
  }

  if (oev) {
    oev->events = events;
    oev->data.ptr = eh;
  }

  return 0;
}

int
event_del(int epfd,
          struct epoll_event *ev)
{
  int err;
  event_handler_t *eh;

  eh = (event_handler_t*)ev->data.ptr;

  err = epoll_ctl(epfd, EPOLL_CTL_DEL, eh->fd, NULL);
  if (err < 0)
    return err;

  free(eh);
  ev->data.ptr = NULL;

  return 0;
}
