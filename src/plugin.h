#pragma once

#include "prc.h"
#include "uthash.h"

typedef struct plugin_handle_ht plugin_handle_ht_t;

struct plugin_handle_ht {
  void *handle;
  prc_plugin_sym_t *sym;
  UT_hash_handle hh;
};

typedef struct plugin_ht_t plugin_ht_t;

struct plugin_ht_t {
  prc_plugin_cmd_t *func;
  UT_hash_handle hh;
};

void
plugin_lookup(dll_t *wq, char *prefix, char *target,
              const char *cmd, char *args);

void
plugin_cmd(dll_t *wq, char *prefix, char *target, char *buf);

int
plugin_cfg(dll_t *plugins);
