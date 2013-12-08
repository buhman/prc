#include <stdio.h>

#include "prc.h"

static void
ctcp_handler(sll_t *wq, char *target, char *tok)
{
  char *sp, *cmd;

  tok = strtok_r(tok, "\001", &sp);

  printf("ctcp tok [%s]\n", tok);

  cmd = strtok_r(tok, " ", &sp);

  if (strcmp("VERSION", cmd) == 0) {
    sll_push(wq, prc_msg("NOTICE", target, ":\001VERSION suck my cock\001", NULL));
  }
}

void
prc_reg(handler_ht_t **plugin_head)                                         {
  prc_register(plugin_head, "ctcp", ctcp_handler);
}
