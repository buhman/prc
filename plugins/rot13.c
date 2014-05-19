#include <ctype.h>

#include "prc.h"

static prc_plugin_cmd_t rot13_cmd;

prc_plugin_sym_t prc_sym[] = {
  {"rot13", rot13_cmd},
  {NULL},
};

static void
rot13_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  char *c;

  if (!args)
    return;

  for (c = args; c < args + strlen(args); c++)
    if (isalpha(*c))
      *c = (*c & 96) + 1 + (*c - (*c & 96) + 12) % 26;

  dll_enq(wq, prc_msg2("PRIVMSG", target, args));
}
