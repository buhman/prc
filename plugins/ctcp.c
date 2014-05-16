#include <stdio.h>

#include "prc.h"

static prc_plugin_cmd_t ctcp_handler;

prc_plugin_sym_t prc_sym[] = {
  {"ctcp", ctcp_handler},
  {NULL},
};

static void
ctcp_handler(dll_t *wq, char *prefix, char *target, char *args)
{
  char *tok, *ts;

  fprintf(stderr, "%s\n", args);
  tok = strchr(args, '\001');

  if (!tok)
    return;

  *tok = '\0';

  ts = strchr(args, ' ');
  if (ts)
    *ts = '\0';

  if (strcmp("VERSION", args) == 0)
    dll_enq(wq, prc_msg2("NOTICE", prc_prefix_parse(prefix, NICK),
                         "\001VERSION Suck my cock\001", NULL));
  else if (strcmp("FINGER", args) == 0)
    dll_enq(wq, prc_msg2("NOTICE", prc_prefix_parse(prefix, NICK),
                         "\001FINGER No thanks\001", NULL));
  else if (strcmp("SOURCE", args) == 0)
    dll_enq(wq, prc_msg2("NOTICE", prc_prefix_parse(prefix, NICK),
                         "\001SOURCE https://buhman.org/cgit/prc.git\001", NULL));
  else if (strcmp("PING", args) == 0 && ts)
    dll_enq(wq, prc_msg2("NOTICE", prc_prefix_parse(prefix, NICK),
                         "\001PING", ts + 1, "\001", NULL));
}
