#pragma once

#include "uthash.h"

typedef void (cmd_handler_t)(sll_t *wq, char *prefix, char *buf);

typedef struct handler_ht handler_ht_t;

struct handler_ht {
  cmd_handler_t *func;
  UT_hash_handle hh;
};

typedef struct handler_sym handler_sym_t;

struct handler_sym {
  char *name;
  cmd_handler_t *func;
};

void
handler_init();

void
handler_free();

void
handler_lookup(char *command,
               sll_t *wq,
               char *prefix,
               char *sp);
