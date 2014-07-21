#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "term.h"
#include "proto.h"
#include "event.h"
#include "handler.h"
#include "cfg.h"
#include "plugin.h"

#include "prc.h"
#include "worker.h"
#include "network.h"

#define MAXEVENT 10

int
worker_sendfd(int sfd, int fd)
{
  struct msghdr msg = {0};
  struct cmsghdr *cmsg;

  char buf[CMSG_SPACE(sizeof(int))];
  int *fdptr;

  struct iovec ib = {0};
  char ip;

  ssize_t retsize;

  {
    /* sendmsg's size calculations appear to be based on iov_len--if
      this is zero, it appears no msg is actually sent. */

    ib.iov_base = &ip;
    ib.iov_len = 1;

    msg.msg_iov = &ib;
    msg.msg_iovlen = 1;
  }

  msg.msg_control = buf;
  msg.msg_controllen = sizeof(buf);

  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof(int));

  fdptr = (int*)CMSG_DATA(cmsg);
  memcpy(fdptr, &fd, sizeof(int));

  msg.msg_controllen = cmsg->cmsg_len;

  retsize = sendmsg(sfd, &msg, 0);
  if (retsize < 0)
    herror("sendmsg()", retsize);
  fprintf(stderr, "SENDMSG: %zd\n", retsize);

  return 0;
}

int
worker_evfd()
{
  int ret;
  char evbuf[8];

  ret = read(evfd, evbuf, 8);
  if (ret < 0)
    herror("read(evfd)", ret);

  if (evbuf[PSIG_EVENT])
    return 1;

  if (evbuf[PSIG_PLUGIN]) {
    fprintf(stderr, "sigplugin\n");
    /* HACK: proto.cwq is an invalid assumption */
    handler_pump_plugin_wq(proto.ceh, NULL, NULL);
  }

  return 0;
}

int
worker_event(int epfd, struct epoll_event* evi, event_handler_t* ehi)
{
  int ret;
  char *buf;

  if (evi->events & EPOLLIN) {

    ret = (ehi->rf)(evi);
    if (ret < 0)
      herror("ehi->rf", ret);

    /* remote disconnect */
    if (ret == 0) {

      while ((buf = dll_pop(ehi->wq)) != NULL)
        free(buf);
      free(ehi->wq);

      fprintf(stderr, "event_del, %p\n", (void*)evi);

      ret = event_del(epfd, evi);
      if (ret < 0)
        herror("event_del", ret);

      /* TODO: need to inform controller of disconnect */
    }
  }

  if (evi->events & EPOLLOUT) {

    ret = (ehi->wf)(evi);
    if (ret < 0)
      herror("ehi->wf", ret);
  }

  return 0;
}

int
worker_main(int cfd, int fds[], int nfds)
{
  int epfd, ret, events;
  struct epoll_event evs[MAXEVENT], *evi;
  event_handler_t *ehi;
  cfg_t *cfg;

  {
    epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd < 0)
      herror("epoll_create1", epfd);

    evfd = event_ev_init(epfd);
    if (evfd < 0)
      herror("event_ev_init", evfd);

    handler_init();

    cfg = cfg_create();

    ret = cfg_open("../prc.cfg", cfg->file);
    if (ret < 0)
      herror("cfg_open", ret);

    ret = cfg_parse(cfg);
    if (ret < 0)
      herror("cfg_parse", ret);
  } /* ... */

  {
    ret = plugin_cfg(cfg->plugins);
    if (ret < 0)
      herror("plugin_cfg", ret);

    ret = term_register(epfd);
    if (ret < 0)
      herror("term_register", ret);

    ret = proto_db_init("../prc.bdb");
    if (ret < 0)
      herror("proto_db_init", ret);

    if (nfds > 0) {
      ret = network_join_fds(epfd, fds, nfds);
      if (ret < 0)
        herror("network_join_fds", ret);
    }
    else {
      ret = network_join_cfg(epfd, cfd, cfg->networks);
      if (ret < 0)
        herror("network_join_cfg", ret);
    }
  }

  while (true) {
    events = epoll_wait(epfd, evs, MAXEVENT, -1);
    if (events < 0) {
      switch (errno) {
      case EINTR:
        break;
      default:
        herror("epoll_wait", events);
      }
    }

    for (evi = evs; evi < evs + events; evi++) {

      ehi = evi->data.ptr;

      if (ehi->fd == evfd) {
        ret = worker_evfd();
        if (ret < 0)
          herror("worker_evfd", ret);
        else if (ret > 0)
          worker_quit("psig_event", 0);
        /* TODO: we need to help the controller understand normal
           termination */
      }
      else {
        ret = worker_event(epfd, evi, ehi);
        if (ret < 0)
          herror("worker_evfd", ret);
      }

      if (ehi->fd == STDIN_FILENO || ehi->fd == evfd)
        if (proto.ceh->wq && proto.ceh->wq->head)
          proto.cev->events = EPOLLOUT | EPOLLIN;
        else
          continue;
      else if (ehi->wq)
        if (ehi->wq->head && !(evi->events & EPOLLOUT))
          evi->events = EPOLLOUT | EPOLLIN;
        else if (!ehi->wq->head && (evi->events & EPOLLOUT))
          evi->events = EPOLLIN;
        else
          continue;
      else
        continue;

      assert((evi->events & EPOLLOUT && ehi->fd != STDIN_FILENO) ||
             (evi->events & EPOLLOUT) == 0);

      if (ehi->fd == STDIN_FILENO || ehi->fd == evfd) {
        ret = epoll_ctl(epfd, EPOLL_CTL_MOD, proto.ceh->fd, proto.cev);
        if (ret < 0)
          herror("epoll_ctl()", ret);
      }
      else {
        ret = epoll_ctl(epfd, EPOLL_CTL_MOD, ehi->fd, evi);
        if (ret < 0)
          herror("epoll_ctl()", ret);
      }
    }

    ret = term_stdout(epfd);
    if (ret < 0)
      herror("term_stdout", ret);
  }
}
