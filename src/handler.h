#pragma once

#include "uthash.h"

typedef void (cmd_handler_t)(dll_t *wq, char *prefix, char *buf);

typedef struct handler_ht handler_ht_t;

struct handler_ht {
  cmd_handler_t *func;
  UT_hash_handle hh;
};

typedef struct handler_sym handler_sym_t;

struct handler_sym {
  const char *name;
  cmd_handler_t *func;
};

void
handler_init(void);

void
handler_free(void);

void
handler_lookup(char *command,
               dll_t *wq,
               char *prefix,
               char *sp);
