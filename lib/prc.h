#pragma once

#include "uthash.h"
#include "sll.h"

#define MSG_SIZE 512

char*
prc_msg(char *cmd, ...);

typedef void (cmd_handler_t)(sll_t*, char*, char**);

typedef struct handler_ht handler_ht_t;

struct handler_ht {
  cmd_handler_t *func;
  UT_hash_handle hh;
};
