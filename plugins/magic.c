#include <string.h>
#include <stdio.h>

#include "prc.h"

static prc_plugin_cmd_t magic_cmd;
static prc_plugin_cmd_t cigam_cmd;

prc_plugin_sym_t prc_sym[] = {
  {"magic", magic_cmd},
  {"cigam", cigam_cmd},
  {NULL},
};

static void
cigam_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  char *ai, *buf;

  if (!args)
    return;

  buf = malloc(MSG_SIZE + 1);

  for (ai = args; ai < (args + strlen(args)) && ai < args + MSG_SIZE / 2; ai++)
    snprintf(buf + ((ai - args) * 2), 3, "%02x", (unsigned char)(*ai));

  *ai = '\0';

  dll_enq(wq, prc_msg2("PRIVMSG", target, "%s", buf));

  free(buf);
}

static void
magic_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  long long num;
  char *tok;
  int i, len;

  if (!args)
    return;

  if (strlen(args) > 16) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[not implemented]"));
    return;
  }

  tok = strchr(args, ' ');
  if (tok) {
    *tok = '\0';
    len = (tok - args) / 2;
  }
  else
    len = strlen(args) / 2;

  if (len % 2) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
    return;
  }

  num = strtoll(args, &tok, 16);
  if (*tok != '\0')
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));

  for (i = 0; i < len; i++)
    *(args + i) = (num >> (len - (i + 1)) * 8);

  *(args + len) = '\0';

  dll_enq(wq, prc_msg2("PRIVMSG", target, "%s", args));
}
