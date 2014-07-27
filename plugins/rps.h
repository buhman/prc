#pragma once

#include "prc.h"
#include "uthash.h"

static prc_plugin_cmd_t rps_cmd;
static prc_plugin_cmd_t test_cmd;
static prc_plugin_cmd_t stats_cmd;
static prc_plugin_cmd_t watch_cmd;

prc_plugin_dtor_t prc_dtor;

enum out {
  OUT_DRAW = 0,
  OUT_WIN,
  OUT_LOSE
};

typedef enum move move_e;

enum move {
  MOVE_BAD = -1,
  MOVE_ROCK = 0,
  MOVE_PAPER,
  MOVE_SCISSORS,
  MOVE_LAST
};

typedef enum args args_e;

enum args {
  ARG_FOE = 0,
  ARG_MOVE,
  ARG_LAST
};

typedef struct move_key move_key_t;

struct move_key {
  char self[16];
  char foe[16];
};

typedef struct move_ht move_ht_t;

struct move_ht {
  move_key_t *key;
  move_e move;
  UT_hash_handle hh;
};

typedef struct score_ht score_ht_t;

struct score_ht {
  char *name;
  long win;
  long loss;
  long draw;
  UT_hash_handle hh;
};
