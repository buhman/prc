#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netdb.h>

#include "proto.h"
#include "event.h"
#include "term.h"
#include "handler.h"

#define BUFSIZE 512

sll_t *proto_cwq;

static char *read_buf = NULL;

int
proto_register(int epfd,
               char *node,
               char *service,
               sll_t **owq)
{
  int sfd, err;
  sll_t *wq;

  wq = calloc(1, sizeof(sll_t));
  proto_cwq = wq;

  sfd = proto_connect(node, service);
  if (sfd < 0) {
    perror("proto_connect()");
    return sfd;
  }

  err = event_add(epfd, sfd, EPOLLIN, proto_read, proto_write, wq, NULL);
  if (err < 0) {
    perror("event_add()");
    return err;
  }

  *owq = wq;

  return 0;
}

int
proto_connect(const char *node,
              const char *service)
{
  int err, sfd = -1, mode;
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

  {
    mode = fcntl(sfd, F_GETFL);
    if (mode < 0) {
      perror("fcntl() GETFL");
      return mode;
    }

    err = fcntl(sfd, F_SETFL, mode | O_NONBLOCK);
    if (err < 0) {
      perror("fcntl() SETFL");
      return err;
    }
  }

  return sfd;
}

static int
recv_line(const int fd,
          const int buf_size,
          char *buf,
          char **buf_iter,
          size_t *recv_c)
{
  char *ptr;
  int len;

  while (true) {
    if (*recv_c > 0) {
      ptr = memchr(*buf_iter, '\r', *recv_c);
      if (ptr) {
        *ptr = '\0';
        len = ptr - *buf_iter;
        *recv_c -= len + 2;
        *buf_iter = ptr + 2;
        return len + 2;
      }

      *buf_iter = memmove(buf, *buf_iter, *recv_c);
    }

    fprintf(stderr, "%p %d %p\n", buf, buf_size, *buf_iter);
    assert(buf + buf_size > *buf_iter);
    len = recv(fd, *buf_iter, buf_size - *recv_c, 0);
    if (len < 0)
      return len;
    if (len == 0) {
      return *recv_c;
    }

    *recv_c += len;
  }
}

int
proto_read(struct epoll_event *ev)
{
  int ret;
  char *buf_iter;
  size_t recv_c;
  event_handler_t *eh = (event_handler_t*)ev->data.ptr;

  if (!read_buf) {
    read_buf = calloc(1, BUFSIZE + 1);
    recv_c = 0;
  }
  else
    recv_c = strlen(read_buf);

  buf_iter = read_buf + recv_c;

  while (true) {
    ret = recv_line(eh->fd, BUFSIZE, read_buf, &buf_iter, &recv_c);
    if (ret < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        break;
      perror("recv_line()");
      return ret;
    }
    if (ret == 0)
      return 0;

    if (recv_c == 512)
      fprintf(stderr, "full recv: [%s]\n", read_buf);

    sll_push(term_wq, strdup(buf_iter - ret));

    assert(proto_parse(ev, buf_iter - ret) >= 0);
  }

  if (recv_c > 0) {
    *(read_buf + recv_c + 1) = '\0';
    fprintf(stderr, "st %lu rc %lu ; rb [%s] bi [%s]\n", strlen(read_buf), recv_c, read_buf, buf_iter);
    assert(strlen(read_buf) == recv_c);
  }
  else {
    free(read_buf);
    read_buf = NULL;
  }

  return 1;
}

int
proto_write(struct epoll_event *ev)
{
  int ret;
  char *buf;
  event_handler_t *eh = (event_handler_t*)ev->data.ptr;

  while (eh->wq->head) {

    sll_pop(eh->wq, &buf);

    fprintf(stderr, "buf: [%s]\n", buf);
    ret = send(eh->fd, buf, strlen(buf), 0);
    assert(ret > 0);
  }

  free(buf);

  return 0;
}

int
proto_parse(struct epoll_event *ev,
            char *buf)
{
  char *tok, *sp, *prefix = NULL;
  event_handler_t *eh = (event_handler_t*)ev->data.ptr;

  tok = strtok_r(buf, " ", &sp);
  if (tok == NULL) {
    fprintf(stderr, "proto_parse(): no prefix or command: [%s]\n", buf);
    return -1;
  }

  if (*tok == ':') {
    prefix = tok;

    tok = strtok_r(NULL, " ", &sp);
    if (tok == NULL) {
      fprintf(stderr, "proto_parse(): no command: [%s]\n", buf);
      return -1;
    }
  }
  else
    fprintf(stderr, "buf: [%s]\n", buf);

  fprintf(stderr, "prefix: [%s]; cmd: [%s]\n", prefix, tok);

  handler_lookup(tok, eh->wq, prefix, sp);

  return 0;
}
