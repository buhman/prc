#pragma once

#include "uthash.h"
#include "sll.h"

#define MSG_SIZE 512

/* prc_{c,d}tor: called on plugin_{,un}load() */
typedef int (prc_plugin_init_t)();

/* prc_sym: array of all $functions the plugin wants to register */
typedef struct prc_plugin_sym prc_plugin_sym_t;

typedef void (prc_plugin_cmd_t)(sll_t *wq, char *prefix, char *target, char *args);

struct prc_plugin_sym {
  char *key;
  prc_plugin_cmd_t *func;
};

enum prefix_cp {
  NICK = 0,
  USER,
  HOST
};

char*
prc_prefix_parse(char *prefix, enum prefix_cp comp);

char*
prc_msg(char *cmd, ...);
