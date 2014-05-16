#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <sys/epoll.h>

#include "term.h"
#include "proto.h"
#include "event.h"
#include "handler.h"

#include "prc.h"

#define MAXEVENT 5

extern struct epoll_event proto_cev;
extern event_handler_t *proto_ceh;

int
main(int argc,
     char **argv)
{
  int epfd, evfd, err, events, terminate = 1;
  struct epoll_event evs[MAXEVENT], *evi;
  event_handler_t *ehi;

  {
    epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd < 0) {
      perror("epoll_create1()");
      exit(EXIT_FAILURE);
    }
  } /* ... */

  {
    evfd = event_init(epfd);
    if (evfd < 0) {
      perror("event_init()");
      exit(EXIT_FAILURE);
    }

    handler_init();
  }

  {
    dll_t *wq;

    term_register(epfd);

    proto_register(epfd, "dickson.freenode.net", "6667", &wq);

    dll_enq(wq, prc_msg("CAP REQ :sasl", NULL));
    dll_enq(wq, prc_msg("NICK buhmin", NULL));
    dll_enq(wq, prc_msg("USER buhmin foo bar :buhman's minion", NULL));
  }

  while (terminate > 0) {
    events = epoll_wait(epfd, evs, MAXEVENT, -1);
    if (events < 0) {
      switch (errno) {
      case EINTR:
        break;
      default:
        perror("epoll_wait()");
        exit(EXIT_FAILURE);
      }
    }

    for (evi = evs; evi < evs + events; evi++) {

      ehi = (event_handler_t*)evi->data.ptr;

      if (ehi->fd == evfd) {
        // FIXME
        terminate--;
        continue;
      }

      if (evi->events & EPOLLIN) {
        //fprintf(stderr, "evir fd: %d\n", ehi->fd);

        err = (ehi->rf)(evi);
        if (err < 0)
          exit(EXIT_FAILURE);

        // remote disconnect
        if (err == 0) {
          assert(!ehi->wq->head);
          free(ehi->wq);
          fprintf(stderr, "event_del, %p\n", (void*)evi);
          err = event_del(epfd, evi);
          if (err < 0) {
            perror("event_del()");
            exit(EXIT_FAILURE);
          }
          continue;
        }
      }

      if (evi->events & EPOLLOUT) {
        //fprintf(stderr, "eviw fd: %d\n", ehi->fd);
        err = (ehi->wf)(evi);
        if (err < 0)
          exit(EXIT_FAILURE);
      }

      if (!ehi->wq) /* HACK? */
        continue;

      /* evi->events will only include the current events; HACK adds EPOLLIN */
      if (ehi->wq->head && !(evi->events & EPOLLOUT) && ehi->fd != STDIN_FILENO)
        //evi->events |= EPOLLOUT;
        evi->events = EPOLLOUT | EPOLLIN;
      else if (!ehi->wq->head && (evi->events & EPOLLOUT) && ehi->fd != STDIN_FILENO)
        //evi->events &= ~EPOLLOUT;
        evi->events = EPOLLIN;
      else if (ehi->fd == STDIN_FILENO && proto_ceh->wq->head) {
        proto_cev.events = EPOLLOUT | EPOLLIN;
        err = epoll_ctl(epfd, EPOLL_CTL_MOD, proto_ceh->fd, &proto_cev);
        if (err < 0) {
          perror("epoll_ctl()");
          exit(EXIT_FAILURE);
        }
      }
      else
        continue;

      assert((evi->events & EPOLLOUT && ehi->fd != STDIN_FILENO) ||
             (evi->events & EPOLLOUT) == 0);

      err = epoll_ctl(epfd, EPOLL_CTL_MOD, ehi->fd, evi);
      if (err < 0) {
        perror("epoll_ctl()");
        exit(EXIT_FAILURE);
      }
    }

    err = term_stdout(epfd);
    if (err < 0)
      exit(EXIT_FAILURE);
  }

  {
    handler_free();

    close(epfd);
    close(evfd);
  }

  return 0;
}
