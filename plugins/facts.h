#pragma once

#include "uthash.h"

static prc_plugin_cmd_t facts_find_handler;
static prc_plugin_cmd_t facts_add_handler;

prc_plugin_ctor_t prc_ctor;
prc_plugin_dtor_t prc_dtor;

typedef struct fact_ht fact_ht_t;

struct fact_ht {
  char *fact;
  UT_hash_handle hh;
};

static int
facts_add(const char *key, const char *fact);

static char *
facts_get(const char *key);

static int
facts_init_ht(const char *path);
