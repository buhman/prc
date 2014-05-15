#include <stdio.h>
#include <time.h>

#include "prc.h"

static prc_plugin_cmd_t time_cmd;

prc_plugin_sym_t prc_sym[] = {
  {"time", time_cmd},
  {NULL},
};

static void
time_cmd(sll_t *wq, char *prefix, char* target, char *args)
{
  char buf[20];
  time_t t;

  t = time(NULL);
  sprintf(buf, "%ld", (long)t);

  sll_push(wq, prc_msg("PRIVMSG", target, buf, NULL));
}

/* {,de}initialization is not required and can be omitted completely */
int
prc_ctor()
{
  return 0;
}

int
prc_dtor()
{
  return 0;
}
