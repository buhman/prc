#pragma once

#include <sys/epoll.h>
#include "dll.h"
#include "cfg.h"
#include "event.h"
#include "bdb.h"

typedef struct proto_node proto_node_t;

struct proto_node {
  const char *name;
  struct epoll_event *ev;
};

typedef struct proto proto_t;

struct proto {
  struct epoll_event *cev;
  event_handler_t *ceh;
  const char *node;
  bdb_t *bdb;
  int tag;
};

extern proto_t proto;

void
proto_add_node(const char *node, struct epoll_event *ev);

int
proto_set_node(char *sub);

int
proto_register(int epfd,
               int sfd,
               const char *node,
               cfg_net_t *cfg,
               struct epoll_event **oev);

int
proto_connect(const char *node,
              const char *service);

int
proto_tls(int sfd, gnutls_session_t *session_out, const char *node);

int
proto_read(struct epoll_event *ev);

int
proto_write(struct epoll_event *ev);

int
proto_parse_buf(struct epoll_event *ev,
                char *buf, size_t len);

int
proto_parse_line(struct epoll_event *ev,
                 char *buf, size_t len);

int
proto_push_msg(char *prefix, char *command, char *params);

int
proto_db_init(const char *path);
