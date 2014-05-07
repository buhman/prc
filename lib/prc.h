#pragma once

#include "uthash.h"
#include "sll.h"

#define MSG_SIZE 512

typedef void (prc_plugin_cmd_t)(sll_t *wq, char *prefix, char *target, char *args);

typedef struct prc_plugin_ht prc_plugin_ht_t;

struct prc_plugin_ht {
  prc_plugin_cmd_t *func;
  UT_hash_handle hh;
};

enum prefix_cp {
  NICK = 0,
  USER,
  HOST
};

void
prc_register(prc_plugin_ht_t **head, char *key, prc_plugin_cmd_t *func);

char*
prc_prefix_parse(char *prefix, enum prefix_cp comp);

char*
prc_msg(char *cmd, ...);
