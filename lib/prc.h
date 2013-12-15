#pragma once

#include "uthash.h"
#include "sll.h"

#define MSG_SIZE 512

typedef void (cmd_handler_t)(sll_t*, char*, char*);

typedef struct handler_ht handler_ht_t;

struct handler_ht {
  cmd_handler_t *func;
  UT_hash_handle hh;
};

void
prc_register(handler_ht_t **head, char *key, cmd_handler_t *func);

char*
prc_msg(char *cmd, ...);

int
prc_lookup2(handler_ht_t *head,
            sll_t *wq,
            char *key,
            char *target,
            char *tok);

int
prc_lookup(handler_ht_t *head,
           sll_t *wq,
           char *target,
           char *tok);
