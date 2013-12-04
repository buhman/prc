#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "prc.h"

#include "sll.h"
#include "proto.h"
#include "uthash.h"
#include "sasl.h"
#include "plugin.h"

static void
hndlr_authed(sll_t *wq, char *prefix, char *sp);

static void
hndlr_auth(sll_t *wq, char *prefix, char *sp);

static void
hndlr_cap(sll_t *wq, char *prefix, char *sp);

static void
hndlr_ping(sll_t *wq, char *prefix, char *sp);

static void
hndlr_privmsg(sll_t *wq, char *prefix, char *sp);

static void
hndlr_welcome(sll_t *wq, char *prefix, char *sp);

static handler_ht_t *proto_head;
static handler_ht_t *admin_head;

static handler_sym_t hst[] = {
  {"CAP", hndlr_cap},
  {"PING", hndlr_ping},
  {"AUTHENTICATE", hndlr_auth},
  {"900", hndlr_authed},
  {"PRIVMSG", hndlr_privmsg},
  {"001", hndlr_welcome},
};

/* output processing */

void
proto_register(sll_t *wq)
{
  sll_push(wq, prc_msg("CAP REQ :sasl", NULL));
  sll_push(wq, prc_msg("NICK buhmin", NULL));
  sll_push(wq, prc_msg("USER buhmin foo bar :buhman's minion", NULL));
}

/* input processing */

static int
prefix_parse(char prefix[], ...)
{
  va_list ap;
  char *tok, *sp, **arg;
  int count = 0;

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
      sll_push(wq, prc_msg("AUTHENTICATE PLAIN", NULL));
    else
      fprintf(stderr, "UHTOK sc_cap_ack: [%s]\n", tok);

    tok = strtok_r(NULL, " ", &sp);
  }
}

static void
hndlr_cap(sll_t *wq, char *prefix, char *sp) {

  char *tok;

  printf("hndlr_cap()\n");

  {
    while ((tok = strtok_r(NULL, " ", &sp)) != NULL) {
      if (strcmp(tok, "*") == 0) {
        continue;
      }
      if (strcmp(tok, "ACK") == 0) {
        tok = strtok_r(NULL, ":", &sp);

        sc_cap_ack(wq, tok);
        break;
      }
      fprintf(stderr, "UHTOK hndlr_cap: [%s]\n", tok);
      break;
    }
  }
}

static void
hndlr_auth(sll_t *wq, char *prefix, char *sp) {

  char *tok;

  printf("hndlr_auth()\n");

  {
    while ((tok = strtok_r(NULL, " ", &sp)) != NULL) {
      if (strcmp(tok, "+") == 0) {
        char *cred;

        if (sasl_auth("buhmin", "WorldDomination", &cred) < 0)
          return;

        printf("CRED: %s\n", cred);
        sll_push(wq, prc_msg("AUTHENTICATE", cred, NULL));
        free(cred);
        return;
      }
      fprintf(stderr, "UHTOK hndlr_auth: [%s]\n", tok);
      break;
    }
  }
}

static void
hndlr_welcome(sll_t *wq, char *prefix, char *sp) {

  sll_push(wq, prc_msg("JOIN #Boohbah", NULL));
}

static void
hndlr_authed(sll_t *wq, char *prefix, char *sp) {

  sll_push(wq, prc_msg("CAP END", NULL));
}

static void
hndlr_ping(sll_t *wq, char *prefix, char *sp) {

  char *tok;

  printf("hndlr_ping()\n");

  tok = strtok_r(NULL, "", &sp);
  printf("tok [%s]\n", tok);
  sll_push(wq, prc_msg("PONG", tok, NULL));
}

static void
hndlr_privmsg(sll_t *wq, char *prefix, char *sp) {

  char *tok, *user, *target;
  char cmd;

  printf("hndlr_privmsg()\n");

  prefix_parse(prefix, &user, NULL);
  if (strcmp(user, "buhman") != 0)
    return;

  tok = strtok_r(NULL, " ", &sp);
  if (strcmp(tok, "buhmin") == 0) {
    target = user;
  }
  else {
    target = tok;
  }

  tok = strtok_r(NULL, ":", &sp);
  cmd = *tok;
  tok++;

  switch(cmd) {
  case '`':
    plugin_handler_cmd(wq, "fact_find", target, tok);
    break;
  case '%':
    plugin_handler_cmd(wq, "fact_add", target, tok);
    break;
  case '$':
    plugin_handler(wq, target, tok);
    break;
  case '#':
    {
      char *key;
      handler_ht_t *item;

      key = strtok_r(tok, " ", &sp);

      if (key != NULL) {
        HASH_FIND_STR(admin_head, key, item);
        if (item)
          (item->func)(wq, target, sp);
        else
          fprintf(stderr, "[ADMIN] BAD KEY: [%s]\n", key);
      }
      else {
        fprintf(stderr, "[ADMIN] NO KEY\n");
      }
    }
    break;
  }

  free(user);
}

void
proto_init_ht() {

  handler_sym_t *hsti;
  handler_ht_t *item;

  int hst_size = sizeof(hst) / sizeof(*hst);

  proto_head = NULL;

  for(hsti = hst; hsti < hst + hst_size; hsti++) {

    item = malloc(sizeof(handler_ht_t));
    item->func = hsti->func;

    printf("HHTH [%s] -> [%p]\n", hsti->name, *(void**)&hsti->func);
    HASH_ADD_KEYPTR(hh, proto_head, hsti->name, strlen(hsti->name), item);
  }

  admin_head = NULL;
  plugin_init(&admin_head);
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

    HASH_FIND_STR(proto_head, command, item);

    if (item != NULL)
      (item->func)(wq, prefix, sp);
  }
}
