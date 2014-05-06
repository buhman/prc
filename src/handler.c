#include <stdio.h>

#include "prc.h"
#include "sasl.h"
#include "handler.h"
#include "term.h"

static cmd_handler_t handler_cap;
static cmd_handler_t handler_authenticate;
static cmd_handler_t handler_ping;
static cmd_handler_t handler_capend;
static cmd_handler_t handler_welcome;
static cmd_handler_t handler_privmsg;

static handler_ht_t *handler_head;

static handler_sym_t _handler_sym[] = {
  {"CAP", handler_cap},
  {"AUTHENTICATE", handler_authenticate},
  {"PING", handler_ping},
  {"PRIVMSG", handler_privmsg},
  {"900", handler_capend},
  {"001", handler_welcome},
};

void
handler_init() {

  handler_sym_t *hsti;
  handler_ht_t *item;

  int hst_size = sizeof(_handler_sym) / sizeof(*_handler_sym);

  handler_head = NULL;

  for(hsti = _handler_sym; hsti < _handler_sym + hst_size; hsti++) {

    item = malloc(sizeof(handler_ht_t));
    item->func = hsti->func;

    //fprintf(stderr, "HHTH [%s] -> [%p]\n", hsti->name, *(void**)&hsti->func);
    HASH_ADD_KEYPTR(hh, handler_head, hsti->name, strlen(hsti->name), item);
  }
}

void
handler_lookup(char *command,
               sll_t *wq,
               char *prefix,
               char *buf) {

  handler_ht_t *item;

  HASH_FIND_STR(handler_head, command, item);

  if (item != NULL)
    (item->func)(wq, prefix, buf);
}

static void
handler_cap(sll_t *wq, char *prefix, char *buf) {

  //fprintf(stderr, "handler_cap(): %s\n", buf);

  /* lazy */

  sll_push(wq, prc_msg("AUTHENTICATE PLAIN", NULL));
}

static void
handler_authenticate(sll_t *wq, char *prefix, char *buf) {

  char *cred;

  //fprintf(stderr, "auth()\n");

  /* lazy */

  if (sasl_auth("buhmin", "WorldDomination", &cred) < 0)
    return;

  sll_push(wq, prc_msg("AUTHENTICATE", cred, NULL));
}

static void
handler_capend(sll_t *wq, char *prefix, char *buf) {

  sll_push(wq, prc_msg("CAP END", NULL));
}

static void
handler_ping(sll_t *wq, char *prefix, char *buf) {

  sll_push(wq, prc_msg("PONG buhmin", NULL));
}

static void
handler_welcome(sll_t *wq, char *prefix, char *buf) {

  sll_push(wq, prc_msg("JOIN ##archlinux-botabuse", NULL));
}

static void
handler_privmsg(sll_t *wq, char *prefix, char *buf) {

  term_printf("buf: [%s]", buf);
}
