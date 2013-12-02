#pragma once

#include "uthash.h"

#define MSG_SIZE 512

typedef void (cmd_handler_t)(sll_t*, char*, char**);

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

/* output processing */

static char*
msg2(char *cmd, ...);

static char *
msg(char *m);

void
proto_register(sll_t *wq);

/* input processing */

static void
hndlr_authed(sll_t *wq, char *prefix, char **sp);

static void
hndlr_auth(sll_t *wq, char *prefix, char **sp);

static void
hndlr_cap(sll_t *wq, char *prefix, char **sp);

static void
hndlr_ping(sll_t *wq, char *prefix, char **sp);

static void
hndlr_privmsg(sll_t *wq, char *prefix, char **sp);

void
proto_init_ht();

void
proto_process(sll_t *wq, char *buf);
