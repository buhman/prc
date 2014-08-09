#pragma once

#include <stdint.h>
#include <stdio.h>

#include "uthash.h"
#include "dll.h"
#include "bdb.h"

#define MSG_SIZE 512

#define herror(msg, ret)                        \
  do { perror(msg); return ret; } while (0)

#define gerror(msg, ret)                        \
  do {						\
    fprintf(stderr, "\n\n%s: %d, %s\n", msg, ret,	\
	       gnutls_strerror(ret));		\
    return ret;					\
  } while (0) 

typedef int (prc_main_t)(int cfd, int fds[], int *nfds);

/* prc_{c,d}tor: called on plugin_{,un}load() */
typedef int (prc_plugin_ctor_t)(bdb_t *bdb, int evfd);

typedef int (prc_plugin_dtor_t)(void);

/* prc_sym: array of all $functions the plugin wants to register */
typedef struct prc_plugin_sym prc_plugin_sym_t;

typedef void (prc_plugin_cmd_t)(dll_t *wq, char *prefix, char *target, char *args);

struct prc_plugin_sym {
  const char *key;
  prc_plugin_cmd_t *func;
};

typedef struct prc_plugin_msg prc_plugin_msg_t;

struct prc_plugin_msg {
  const char *cmd;
  const char *target;
  char *buf;
};

typedef struct prc_db_msg prc_db_msg_t;

struct prc_db_msg {
  uint64_t prefix;
  uint64_t command;
  uint64_t params;
};

enum prefix_cp {
  NICK = 0,
  USER,
  HOST
};

enum prc_signal {
  PSIG_EVENT = 0,
  PSIG_PLUGIN,
};

char*
prc_prefix_parse(char *prefix, enum prefix_cp comp);

char*
prc_msg(const char *cmd, ...);

prc_plugin_msg_t*
prc_msg2(const char *cmd, const char *target, const char *format, ...);

char*
prc_msg3(const char *format, ...);
