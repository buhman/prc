#pragma once

#include "prc.h"

typedef int (reg_fp_t)(prc_plugin_ht_t**);

void
plugin_lookup(sll_t *wq, char *prefix, char *target,
              char *cmd, char *args);

void
plugin_cmd(sll_t *wq, char *prefix, char *target, char *buf);
