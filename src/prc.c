#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/epoll.h>

#include "term.h"
#include "proto.h"
#include "event.h"

/*
static void
term(int epfd)
{
  int err;

  err = term_setup();
  if (err < 0)
    exit(EXIT_FAILURE);

  err = event_add(epfd, STDIN_FILENO, EPOLLIN, term_read);
  if (err < 0) {
    perror("event_add()");
    exit(EXIT_FAILURE);
  }
}
*/

static void
proto(int epfd)
{
  int sfd, err;

  sfd = proto_connect("irc.freenode.net", "6667");
  if (sfd < 0)
    exit(EXIT_FAILURE);

  err = event_add(epfd, sfd, EPOLLIN, proto_read);
  if (err < 0) {
    perror("event_add()");
    exit(EXIT_FAILURE);
  }
}

int
main(int argc,
     char **argv)
{
  int epfd;

  {
    epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd < 0) {
      perror("epoll_create1()");
      exit(EXIT_FAILURE);
    }
  } /* ... */


  {
    proto(epfd);
  }

  int events;
  struct epoll_event evs[5], *evi;


  while (true) {
    events = epoll_wait(epfd, evs, 5, -1);
    if (events < 0) {
      perror("epoll_wait()");
      exit(EXIT_FAILURE);
    }

    for (evi = evs; evi < evs + events; evi++) {

      printf("evi\n");
      (((event_handler_t*)evi->data.ptr)->func)(evi);
    }
  }

  return 0;
}
