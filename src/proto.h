#pragma once

#include "prc.h"

#define MSG_SIZE 512

typedef struct handler_sym handler_sym_t;

struct handler_sym {
  char *name;
  cmd_handler_t *func;
};

void
proto_register(sll_t *wq);

void
proto_init_ht();

void
proto_process(sll_t *wq, char *buf);
