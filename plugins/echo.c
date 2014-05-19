#include <ctype.h>

#include "prc.h"

static prc_plugin_cmd_t echo_cmd;

prc_plugin_sym_t prc_sym[] = {
  {"echo", echo_cmd},
  {NULL},
};

static void
echo_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  dll_enq(wq, prc_msg2("PRIVMSG", target, args));
}
