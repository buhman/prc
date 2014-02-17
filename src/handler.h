#pragma once

#include "prc.h"

typedef struct handler_sym handler_sym_t;

struct handler_sym {
  char *name;
  cmd_handler_t *func;
};

void
handler_init();

void
handler_lookup(char *command,
               sll_t *wq,
               char *prefix,
               char *sp);
