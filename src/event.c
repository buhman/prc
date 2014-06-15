#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "dll.h"
#include "event.h"
#include "prc.h"

int evfd;

static void
handle_signal(int signum)
{
  int err;
  char buf[8] = {0};

  buf[PSIG_EVENT] = 1;

  err = write(evfd, buf, 8);
  if (err < 0)
    perror("write(evfd)");
}

int
event_init(int epfd)
{
  int err;
  struct sigaction *act;

  evfd = eventfd(0, EFD_NONBLOCK);
  if (evfd < 0) {
    perror("eventfd");
    return evfd;
  }

  {
    act = calloc(1, sizeof(struct sigaction));
    act->sa_handler = handle_signal;

    err = sigaction(SIGINT, act, NULL);
    if (err < 0) {
      perror("sigaction");
      free(act);
      return err;
    }

    free(act);
  }

  {
    err = event_add(epfd, evfd, EPOLLIN, NULL, NULL, NULL, NULL, NULL);
    if (err < 0) {
      perror("event_add");
      return err;
    }
  }

  return evfd;
}

int
event_add(int epfd,
          int fd,
          uint32_t events,
          eh_fptr_t *rf,
          eh_fptr_t *wf,
          dll_t *wq,
          cfg_net_t *cfg,
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
    eh->cfg = cfg;

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
