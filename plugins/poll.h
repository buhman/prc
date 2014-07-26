#pragma once

#include <time.h>
#include <string.h>
#include "uthash.h"

static prc_plugin_cmd_t open_cmd;
static prc_plugin_cmd_t vote_cmd;
static prc_plugin_cmd_t count_cmd;
static prc_plugin_cmd_t close_cmd;
static prc_plugin_cmd_t adhoc_cmd;

prc_plugin_dtor_t prc_dtor;

typedef struct ballot_ht ballot_ht_t;

struct ballot_ht {
  char *key;
  UT_hash_handle hh;
};

typedef struct vote_ht vote_ht_t;

struct vote_ht {
  char *nick;
  char *key;
  UT_hash_handle hh;
};

typedef struct poll_ht poll_ht_t;

struct poll_ht {
  char *name;
  time_t time;
  char adhoc;
  ballot_ht_t *ballot;
  vote_ht_t *vote;
  UT_hash_handle hh;
};
