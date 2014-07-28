#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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

#include "db.h"
#include "tag.h"
#include "row.h"
#include "malloc.h"
#include "util.h"

#include "proto.h"
#include "event.h"
#include "term.h"
#include "handler.h"
#include "buf.h"
#include "prc.h"

#define BUFSIZE 512

static char *parse_buf = NULL;
static char *parse_bufi = NULL;

static dll_t proto_nodes = {NULL};

proto_t proto = {NULL};

int
proto_set_node(char *sub)
{
  dll_link_t *li;
  proto_node_t *node;
  char *buf;

  li = proto_nodes.head;

  while (li) {

    node = li->buf;
    buf = strstr(node->name, sub);

    if (buf) {

      proto.cev = node->ev;
      proto.ceh = node->ev->data.ptr;
      proto.node = node->name;

      return 0;
    }

    li = li->next;
  }

  return -1;
}

void
proto_add_node(const char *node, struct epoll_event *ev)
{
  proto_node_t *n;

  n = malloc(sizeof(proto_node_t));

  n->name = node;
  n->ev = ev;

  dll_enq(&proto_nodes, n);
}

int
proto_db_init(const char *path)
{
  int err;

  proto.bdb = malloc(sizeof(bdb_t));
  err = bdb_db_open(path, proto.bdb);
  if (err < 0) {
    perror("bdb_db_open()");
    return err;
  }

  proto.tag = bdb_tag_find("default", proto.bdb);
  if (proto.tag < 0)
    proto.tag = bdb_tag_add("default", proto.bdb);

  return 0;
}

int
proto_register(int epfd,
               int sfd,
               const char *node,
               cfg_net_t *cfg,
               dll_t **owq)
{
  int err;
  dll_t *wq;
  struct epoll_event *ev;

  wq = calloc(1, sizeof(dll_t));
  ev = malloc(sizeof(struct epoll_event));

  err = event_add(epfd, sfd, EPOLLIN, proto_read, proto_write, wq, cfg, ev);
  if (err < 0) {
    free(wq);
    perror("event_add()");
    return err;
  }

  proto.cev = ev;
  proto.ceh = ev->data.ptr;
  proto.node = node;

  proto_add_node(node, proto.cev);

  if (owq)
    *owq = wq;

  return sfd;
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

int
proto_read(struct epoll_event *ev)
{
  event_handler_t *eh = (event_handler_t*)ev->data.ptr;
  ssize_t len;
  char *rbuf = malloc(BUFSIZE);

  while (true) {

    len = recv(eh->fd, rbuf, BUFSIZE, 0);
    if (len < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        break;
      perror("recv()");
      free(rbuf);
      return -1;
    } else if (len == 0) {
      free(rbuf);
      return 0;
    }

    proto_parse_buf(ev, rbuf, (size_t)len);
  }

  free(rbuf);

  return 1;
}

int
proto_write(struct epoll_event *ev)
{
  int ret;
  char *buf;
  event_handler_t *eh = (event_handler_t*)ev->data.ptr;

  while ((buf = dll_pop(eh->wq)) != NULL) {

    term_printf("%s", buf);

    ret = send(eh->fd, buf, strlen(buf), 0);
    assert(ret > 0);

    free(buf);
  }

  return 0;
}

int
proto_parse_buf(struct epoll_event *ev,
                char *buf, size_t len)
{
  char *ptr;

  if (parse_buf == NULL) {
    parse_buf = malloc(BUFSIZE * 2);
    parse_bufi = parse_buf;
  }

  {
    assert(parse_bufi - parse_buf + len < BUFSIZE * 2);
    memcpy(parse_bufi, buf, len);
    parse_bufi += len;
  }

  while (true) {
    ptr = memchr(parse_buf, '\r', parse_bufi - parse_buf);

    if (ptr && ptr + 1 < parse_bufi && *(ptr + 1) == '\n') {

      *ptr = '\0';

      proto_parse_line(ev, parse_buf, ptr - parse_buf);

      memmove(parse_buf, ptr + 2, parse_bufi - ptr - 2);
      parse_bufi = parse_buf + (parse_bufi - ptr - 2);
    }
    else
      break;
  }

  assert(parse_bufi - parse_buf < BUFSIZE);

  return 0;
}

int proto_parse_line(struct epoll_event *ev,
                     char *buf, size_t len)
{
  event_handler_t *eh = (event_handler_t*)ev->data.ptr;
  char *tok, *bufi = buf, *prefix = NULL;
  int err;

  *(buf + len) = '\0';

  term_printf("%s", buf);

  while (true) {

    tok = memchr(bufi, ' ', len - (bufi - buf));
    if (tok != NULL) {
      *tok = '\0';
    }
    else {
      fprintf(stderr, "NO TOK\n");
      break;
    }

    if (bufi == buf && *bufi == ':') {
      prefix = bufi;
      bufi = tok + 1;
      continue;
    }

    err = bdb_row_push_str(proto.tag, proto.bdb, 3, prefix, bufi, tok + 1);
    if (err < 0)
      perror("proto_push_msg");

    handler_lookup(bufi, eh, prefix, tok + 1);

    break;
  }

  return 0;
}
