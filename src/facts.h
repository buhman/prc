#pragma once

#include "uthash.h"

typedef struct fact_ht fact_ht_t;

struct fact_ht {
  char *fact;
  UT_hash_handle hh;
};

char *
facts_get(char *key);

int
facts_init_ht(char *path);
