#pragma once

#include "uthash.h"
#include "dll.h"

#define MSG_SIZE 512

/* prc_{c,d}tor: called on plugin_{,un}load() */
typedef int (prc_plugin_init_t)();

/* prc_sym: array of all $functions the plugin wants to register */
typedef struct prc_plugin_sym prc_plugin_sym_t;

typedef void (prc_plugin_cmd_t)(dll_t *wq, char *prefix, char *target, char *args);

struct prc_plugin_sym {
  char *key;
  prc_plugin_cmd_t *func;
};

typedef struct prc_plugin_msg prc_plugin_msg_t;

struct prc_plugin_msg {
  char *cmd;
  char *target;
  char *buf;
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

prc_plugin_msg_t*
prc_msg2(char *cmd, char *target, char *format, ...);

char*
prc_msg3(char *format, ...);
