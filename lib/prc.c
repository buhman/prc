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

int
prc_lookup2(handler_ht_t *head,
            sll_t *wq,
            char *key,
            char *target,
            char *tok)
{
  handler_ht_t *item;

  HASH_FIND_STR(head, key, item);
  if (item)
    (item->func)(wq, target, tok);
  else {
    fprintf(stderr, "bad key: [%s]\n", key);
    return -1;
  }

  return 0;
}

int
prc_lookup(handler_ht_t *head,
           sll_t *wq,
           char *target,
           char *tok)
{
  char *sp, *key;

  printf("tok: [%s]\n", tok);

  key = strtok_r(tok, " ", &sp);

  if (key != NULL) {
    tok = strtok_r(NULL, "", &sp);
    return prc_lookup2(head, wq, key, target, tok);
  }
  else
    fprintf(stderr, "bad key\n");

  return -1;
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
