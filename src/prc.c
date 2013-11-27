#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include <sys/epoll.h>

#include "dbuf.h"

#define MAX_EVENTS 2

int
main(int argc,
     char **argv)
{
  int efd, err;

  {
    efd = epoll_create1(EPOLL_CLOEXEC);
    if (efd < 0) {
      perror("epoll_create1()");
      return EXIT_FAILURE;
    }
  } /* ... */

  {
    int mode;

    mode = fcntl(STDIN_FILENO, F_GETFL);
    if (mode < 0) {
      perror("fcntl(F_GETFL)");
      return EXIT_FAILURE;
    }

    err = fcntl(STDIN_FILENO, F_SETFL, mode | O_NONBLOCK);
    if (err < 0) {
      perror("fcntl(F_SETFL)");
      return EXIT_FAILURE;
    }
  } /* ... */

  {
    struct epoll_event ev;

    ev.data.u32 = DBUF_READ;
    ev.data.fd = STDIN_FILENO;
    ev.events = EPOLLIN;

    err = epoll_ctl(efd, EPOLL_CTL_ADD, ev.data.fd, &ev);
    if (err < 0) {
      perror("epoll_ctl()");
      return EXIT_FAILURE;
    }
  } /* ... */

  {
    struct epoll_event *evs, *evi;
    int nfds;

    evs = calloc(MAX_EVENTS, sizeof(struct epoll_event));

    while (true) {


      nfds = epoll_wait(efd, evs, 2, -1);
      if (nfds < 0) {
	perror("epoll_wait()");
	return EXIT_FAILURE;
      }

      for (evi = evs; evi < evs + MAX_EVENTS; evi++) {

	if (evi->events & EPOLLIN) {

	  char *buf;
	  ssize_t size;
	  size = dbuf_read(evi->data.fd, &buf, evi->data.u32);
	  return EXIT_SUCCESS;
	}
      }
    }
  } /* ... */

  return EXIT_SUCCESS;
}
