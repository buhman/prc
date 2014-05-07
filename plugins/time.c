#include <stdio.h>
#include <time.h>

#include "prc.h"

void
time_cmd(sll_t *wq, char *prefix, char* target, char *args)
{
  char buf[20];
  time_t t;

  t = time(NULL);
  sprintf(buf, "%ld", (long)t);

  sll_push(wq, prc_msg("PRIVMSG", target, buf, NULL));
}

void
prc_reg(prc_plugin_ht_t **plugin_head)
{
  prc_register(plugin_head, "time", time_cmd);
}
