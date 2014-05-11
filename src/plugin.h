#pragma once

#include "prc.h"
#include "uthash.h"

typedef int (reg_fp_t)(prc_plugin_ht_t**);

typedef struct plugin_handle_ht plugin_handle_ht_t;

struct plugin_handle_ht {
  void *handle;
  char *name;
  UT_hash_handle hh;
};

void
plugin_lookup(sll_t *wq, char *prefix, char *target,
              char *cmd, char *args);

void
plugin_cmd(sll_t *wq, char *prefix, char *target, char *buf);
