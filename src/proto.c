#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "sll.h"
#include "proto.h"
#include "uthash.h"
#include "sasl.h"
#include "facts.h"

static handler_ht_t *hht_head;

static handler_sym_t hst[] = {
  {"CAP", hndlr_cap},
  {"PING", hndlr_ping},
  {"AUTHENTICATE", hndlr_auth},
  {"900", hndlr_authed},
  {"PRIVMSG", hndlr_privmsg},
};

/* output processing */

static char*
msg2(char *cmd, ...)
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

static char*
msg(char *m)
{
  char *buf = malloc(MSG_SIZE);

  snprintf(buf, MSG_SIZE, "%s\r\n", m);

  return buf;
}

void
proto_register(sll_t *wq)
{
  sll_push(wq, msg("CAP REQ :sasl"));
  sll_push(wq, msg("NICK buhmin"));
  sll_push(wq, msg("USER buhmin foo bar :buhman's minion"));
}

/* input processing */

static int
prefix_parse(char prefix[], ...)
{
  va_list ap;
  char *tok, *sp, **arg;
  int count;

  va_start(ap, prefix);
  tok = strtok_r(prefix, ":!@", &sp);
  while (tok != NULL) {

    arg = va_arg(ap, char**);
    if (arg == NULL)
      break;

    count++;
    *arg = strdup(tok);
    tok = strtok_r(NULL, ":!@", &sp);
  }

  va_end(ap);

  return count;
}

static void
sc_cap_ack(sll_t *wq, char *tok)
{
  char *sp;

  tok = strtok_r(tok, " ", &sp);

  while (tok != NULL) {
    printf("stok [%s]\n", tok);

    if (strcmp(tok, "sasl") == 0)
      sll_push(wq, msg("AUTHENTICATE PLAIN"));
    else
      fprintf(stderr, "UHTOK sc_cap_ack: [%s]\n", tok);

    tok = strtok_r(NULL, " ", &sp);
  }
}

static void
hndlr_cap(sll_t *wq, char *prefix, char **sp) {

  char *tok;

  printf("hndlr_cap()\n");

  {
    while ((tok = strtok_r(NULL, " ", sp)) != NULL) {
      if (strcmp(tok, "*") == 0) {
        continue;
      }
      if (strcmp(tok, "ACK") == 0) {
        tok = strtok_r(NULL, ":", sp);

        sc_cap_ack(wq, tok);
        break;
      }
      fprintf(stderr, "UHTOK hndlr_cap: [%s]\n", tok);
      break;
    }
  }
}

static void
hndlr_auth(sll_t *wq, char *prefix, char **sp) {

  char *tok;

  printf("hndlr_auth()\n");

  {
    while ((tok = strtok_r(NULL, " ", sp)) != NULL) {
      if (strcmp(tok, "+") == 0) {
        char *cred;

        if (sasl_auth("buhmin", "WorldDomination", &cred) < 0)
          return;

        printf("CRED: %s\n", cred);
        sll_push(wq, msg2("AUTHENTICATE", cred));
        free(cred);
        return;
      }
      fprintf(stderr, "UHTOK hndlr_auth: [%s]\n", tok);
      break;
    }
  }
}

static void
hndlr_authed(sll_t *wq, char *prefix, char **sp) {

  sll_push(wq, msg("CAP END"));
}

static void
hndlr_ping(sll_t *wq, char *prefix, char **sp) {

  char *tok;

  printf("hndlr_ping()\n");

  tok = strtok_r(NULL, "", sp);
  printf("tok [%s]\n", tok);
  sll_push(wq, msg2("PONG", tok));
}

static void
hndlr_privmsg(sll_t *wq, char *prefix, char **sp) {

  char *tok, *user, *channel;

  printf("hndlr_privmsg()\n");

  prefix_parse(prefix, &user, NULL);
  if (strcmp(user, "buhman") != 0)
    return;

  tok = strtok_r(NULL, " ", sp);
  if (strcmp(tok, "buhmin") == 0) {
    channel = user;
  }
  else {
    channel = tok;
  }

  tok = strtok_r(NULL, ":", sp);
  if (*tok == '`') {
    char *fact, *pmsg;

    tok++;

    {
      fact = facts_get(tok);

      if (fact) {
        pmsg = malloc(strlen(fact) + 2);
        strcpy(pmsg + 1, fact);
        *pmsg = ':';

        sll_push(wq, msg2("PRIVMSG", channel, pmsg, NULL));
        free(fact);
      }
      else
        printf("FACT [%s] not fount\n", tok);
    }
  }

  free(user);
}

void
proto_init_ht() {

  handler_sym_t *hsti;
  handler_ht_t *item;
  char *key;

  int hst_size = sizeof(hst) / sizeof(*hst);

  hht_head = NULL;

  for(hsti = hst; hsti < hst + hst_size; hsti++) {

    item = malloc(sizeof(handler_ht_t));
    item->func = hsti->func;

    printf("HHTH [%s] -> [%p]\n", hsti->name, hsti->func);
    HASH_ADD_KEYPTR(hh, hht_head, hsti->name, strlen(hsti->name), item);
  }
}

void
proto_process(sll_t *wq, char *buf)
{
  char *tok, *prefix = NULL, *command, *sp;

  printf("S: [%s]\n", buf);

  tok = strtok_r(buf, " ", &sp);
  if (tok == NULL) {
    fprintf(stderr, "ERROR: no prefix or command\n");
    return;
  }

  if (tok[0] == ':') {
    // is the prefix
    prefix = tok;

    tok = strtok_r(NULL, " ", &sp);
    if (tok == NULL) {
      fprintf(stderr, "ERROR: no command\n");
      return;
    }
  }

  command = tok;

  {
    handler_ht_t *item;

    HASH_FIND_STR(hht_head, command, item);

    if (item != NULL)
      (item->func)(wq, prefix, &sp);
  }
}
