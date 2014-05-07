#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "prc.h"

void
prc_register(prc_plugin_ht_t **head, char *key, prc_plugin_cmd_t *func)
{
  prc_plugin_ht_t *item;
  item = malloc(sizeof(prc_plugin_ht_t));
  item->func = func;

  HASH_ADD_KEYPTR(hh, *head, key, strlen(key), item);
}

char*
prc_prefix_parse(char *prefix, enum prefix_cp comp)
{
  char *sp, *tok;

  // first character will always be ':' and should be discarded
  prefix++;

  tok = strtok_r(prefix, "!@", &sp);
  if (!tok)
    return prefix;

  for (int i = NICK; i < (int)comp; i++) {
    tok = strtok_r(NULL, "!@", &sp);
    if (!tok)
      return prefix;
  }

  return tok;
}

char*
prc_msg(char *cmd, ...)
{
  va_list ap;
  char *arg = cmd;

  char *buf = malloc(MSG_SIZE);
  char *ibuf = buf;

  va_start(ap, cmd);

  while (arg != NULL) {

    ibuf += sprintf(ibuf, "%s ", arg);

    arg = va_arg(ap, char*);
  }

  va_end(ap);

  sprintf(ibuf - 1, "\r\n");

  return buf;
}
