#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>

#include <sys/socket.h>
#include <sys/epoll.h>

#include "epoll.h"

static int
epoll_add(int efd,
	  int ifd,
	  dbuf_readf_t readf);

int
epoll_connect(int efd,
	      const char *node,
	      const char *service)
{
  int err, sfd = -1;
  struct addrinfo hints, *result, *rp;

  {
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
  } /* ... */

  err = getaddrinfo(node, service, &hints, &result);
  if (err != 0) {
    fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(err));
    return err;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {

    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd < 0) {
      perror("socket()");
      continue;
    }

    err = connect(sfd, rp->ai_addr, rp->ai_addrlen);
    if (err < 0) {
      perror("connect()");
      close(sfd);
      continue;
    }

    break;
  }

  freeaddrinfo(result);

  return epoll_add(efd, sfd, DBUF_RECV);
}

int
epoll_input(int efd)
{
  int mode, err;

  mode = fcntl(STDIN_FILENO, F_GETFL);
  if (mode < 0) {
    perror("fcntl(F_GETFL)");
    return mode;
  }

  err = fcntl(STDIN_FILENO, F_SETFL, mode | O_NONBLOCK);
  if (err < 0) {
    perror("fcntl(F_SETFL)");
    return err;
  }

  return epoll_add(efd, STDIN_FILENO, DBUF_READ);
} /* ... */

static int
epoll_add(int efd,
	  int ifd,
	  dbuf_readf_t readf)
{
  int err;
  struct epoll_event ev;

  ev.data.u64 = dbuf_pack(ifd, readf);
  ev.events = EPOLLIN | EPOLLOUT;

  err = epoll_ctl(efd, EPOLL_CTL_ADD, ev.data.fd, &ev);
  if (err < 0) {
    perror("epoll_ctl()");
    return EXIT_FAILURE;
  }

  return 0;
} /* ... */
