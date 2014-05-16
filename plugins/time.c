#include <time.h>

#include "prc.h"

static prc_plugin_cmd_t time_cmd;

prc_plugin_sym_t prc_sym[] = {
  {"time", time_cmd},
  {NULL},
};

static void
time_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  dll_enq(wq, prc_msg2("PRIVMSG", target, "%ld", (long)time(NULL)));
}
