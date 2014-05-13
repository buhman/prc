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

void
prc_deregister(prc_plugin_ht_t **head, char *key)
{
  prc_plugin_ht_t *item;

  HASH_FIND_STR(*head, key, item);
  if (!item) {
    fprintf(stderr, "attempt to deregister unregistered [%s]\n", key);
    return;
  }

  HASH_DELETE(hh, *head, item);

  free(item);
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

    /* DRAGONS */
    if (*(arg + 1) == '\0')
      ibuf += sprintf(ibuf, arg);
    else
      ibuf += sprintf(ibuf, "%s ", arg);

    arg = va_arg(ap, char*);
  }

  va_end(ap);

  sprintf(ibuf - 1, "\r\n");

  return buf;
}
