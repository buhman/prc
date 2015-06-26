#pragma once

#include <unistd.h>

#include "bus/bus.h"

typedef struct {
  union {
    char *prefix;
    struct {
      char *nick;
      char *user;
      char *host;
    };
  };
  char *command;
  struct {
    char *middle;
    char *trailing;
  } params;
} prc_msg_t;

#define PRC_MSG_FIELDS (sizeof (prc_msg_t) / sizeof (char*))

typedef struct {
  short fields[PRC_MSG_FIELDS];
  size_t buflen;
  char buf[];
} prc_msg_pack_t;

#define prc_pack_alloca(buflen) alloca(sizeof (prc_msg_pack_t) + buflen)

int
prc_pack_msg(prc_msg_pack_t *pack, prc_msg_t *pmsg, char *buf, size_t len);

int
prc_unpack_msg(bb_message *bmsg, prc_msg_t *pmsg);
