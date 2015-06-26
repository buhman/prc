#pragma once

#include <unistd.h>

#include "prc/prc.h"

typedef struct prc_proto {
  const char *node;
  const char *service;
} prc_proto_t;

typedef struct prc_parse {
  char buf[512];
  size_t len;
} prc_proto_parse_t;

typedef struct prc_proto_ctx {
  prc_proto_t proto;
  prc_proto_parse_t parse;
} prc_proto_ctx_t;

int
prc_proto_connect(int efd, const char *node, const char *service);

int
prc_proto_read_cb(event_handler *eh, void *buf, size_t len);

int
prc_proto_close_cb(event_handler *eh);

int
prc_proto_parse_msg(event_handler *eh, char *buf, prc_msg_t *msg);
