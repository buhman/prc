#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "prc.h"

void
prc_register(handler_ht_t **head, char *key, cmd_handler_t *func)
{
  handler_ht_t *item;
  item = malloc(sizeof(handler_ht_t));
  item->func = func;

  HASH_ADD_KEYPTR(hh, *head, key, strlen(key), item);
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
