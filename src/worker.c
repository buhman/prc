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
worker_main(int cfd, int fds[], int nfds)
{
  int epfd, err, events, terminate = 1;
  struct epoll_event evs[MAXEVENT], *evi;
  event_handler_t *ehi;
  cfg_t *cfg;
  char evbuf[8];

  epfd = epoll_create1(EPOLL_CLOEXEC);
  if (epfd < 0)
    herror("epoll_create1()", epfd);

  evfd = event_ev_init(epfd);
  if (evfd < 0)
    herror("event_ev_init()", evfd);

  handler_init();

  {
    cfg = cfg_create();

    err = cfg_open("../prc.cfg", cfg->file);
    if (err < 0) {
      perror("cfg_open()");
      return err;
    }

    err = cfg_parse(cfg);
    if (err < 0)
      return err;
  } /* ... */

  {
    err = plugin_cfg(cfg->plugins);
    if (err < 0)
      return err;

    term_register(epfd);

    err = proto_db_init("../prc.bdb");
    if (err < 0)
      return err;

    if (nfds > 0) {
      err = network_join_fds(epfd, fds, nfds);
      if (err < 0)
        return err;
    }
    else {
      err = network_join_cfg(epfd, cfd, cfg->networks);
      if (err < 0)
        return err;
    }
  }

  while (terminate > 0) {
    events = epoll_wait(epfd, evs, MAXEVENT, -1);
    if (events < 0) {
      switch (errno) {
      case EINTR:
        break;
      default:
        perror("epoll_wait()");
        return err;
      }
    }

    for (evi = evs; evi < evs + events; evi++) {

      ehi = (event_handler_t*)evi->data.ptr;

      if (ehi->fd == evfd) {

        err = read(evfd, evbuf, 8);
        if (err < 0) {
          perror("read(evfd)");
          return err;
        }

        if (evbuf[PSIG_EVENT]) {
          terminate--;
          continue;
        }

        if (evbuf[PSIG_PLUGIN]) {
          fprintf(stderr, "sigplugin\n");
          /* HACK: proto.cwq is an invalid assumption */
          handler_pump_plugin_wq(proto.ceh, NULL, NULL);
        }
      }
      else {

        if (evi->events & EPOLLIN) {
          //fprintf(stderr, "evir fd: %d\n", ehi->fd);

          err = (ehi->rf)(evi);
          if (err < 0)
            return err;

          // remote disconnect
          if (err == 0) {
            {
              char *buf;
              while ((buf = dll_pop(ehi->wq)) != NULL)
                free(buf);
            }
            free(ehi->wq);
            fprintf(stderr, "event_del, %p\n", (void*)evi);
            err = event_del(epfd, evi);
            if (err < 0) {
              perror("event_del()");
              return err;
            }
            continue;
          }
        }

        if (evi->events & EPOLLOUT) {
          //fprintf(stderr, "eviw fd: %d\n", ehi->fd);
          err = (ehi->wf)(evi);
          if (err < 0)
            return err;
        }
      }

      if ((!ehi->wq || !proto.ceh->wq) && ehi->fd != evfd) /* HACKS */
        continue;

      /* evi->events will only include the current events; HACK adds EPOLLIN */
      if ((ehi->fd == STDIN_FILENO || ehi->fd == evfd) && proto.ceh->wq->head) {
        proto.cev->events = EPOLLOUT | EPOLLIN;
        err = epoll_ctl(epfd, EPOLL_CTL_MOD, proto.ceh->fd, proto.cev);
        if (err < 0) {
          perror("epoll_ctl()");
          return err;
        }
      }
      else if (ehi->wq->head && !(evi->events & EPOLLOUT) && ehi->fd != STDIN_FILENO)
        //evi->events |= EPOLLOUT;
        evi->events = EPOLLOUT | EPOLLIN;
      else if (!ehi->wq->head && (evi->events & EPOLLOUT) && ehi->fd != STDIN_FILENO)
        //evi->events &= ~EPOLLOUT;
        evi->events = EPOLLIN;
      else
        continue;

      assert((evi->events & EPOLLOUT && ehi->fd != STDIN_FILENO) ||
             (evi->events & EPOLLOUT) == 0);

      if (ehi->fd != STDIN_FILENO || ehi->fd != evfd) {
        err = epoll_ctl(epfd, EPOLL_CTL_MOD, ehi->fd, evi);
        if (err < 0) {
          perror("epoll_ctl()");
          return err;
        }
      }
    }

    err = term_stdout(epfd);
    if (err < 0)
      return err;
  }

  {
    handler_free();

    close(epfd);
    close(evfd);

    err = cfg_close(cfg->file);
    if (err < 0) {
      perror("cfg_close()");
      return err;
    }

    cfg_free(cfg);
  } /* ... */

  return 0;
}
