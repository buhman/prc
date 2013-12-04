#pragma once

typedef int (reg_fp_t)(handler_ht_t**);

int
plugin_handler(sll_t *wq, char *target, char **sp);

void
plugin_init(handler_ht_t **admin_head);
