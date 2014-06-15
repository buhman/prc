#pragma once

#include "uthash.h"
#include "event.h"

typedef void (cmd_handler_t)(event_handler_t *eh, char *prefix, char *buf);

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
               event_handler_t *eh,
               char *prefix,
               char *sp);

void
handler_pump_plugin_wq(event_handler_t *eh, char *redirect);

int
handler_join_networks(int epfd, dll_t *networks);
