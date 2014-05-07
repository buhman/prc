#include <stdio.h>

#include "prc.h"

static void
ctcp_handler(sll_t *wq, char *prefix, char *target, char *args)
{
  char *tok;

  tok = strchr(args, '\001');
  *tok = '\0';

  if (strcmp("VERSION", args) == 0)
    sll_push(wq, prc_msg("NOTICE", prc_prefix_parse(prefix, NICK),
                         ":\001VERSION Suck my cock\001", NULL));
}

void
prc_reg(prc_plugin_ht_t **plugin_head)
{
  prc_register(plugin_head, "ctcp", ctcp_handler);
}
