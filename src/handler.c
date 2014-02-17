#include <stdio.h>

#include "prc.h"
#include "sasl.h"
#include "handler.h"

static void
handler_cap(sll_t *wq, char *prefix, char *sp);

static void
handler_authenticate(sll_t *wq, char *prefix, char *sp);

static void
handler_capend(sll_t *wq, char *prefix, char *sp);

static handler_ht_t *handler_head;

static handler_sym_t _handler_sym[] = {
  {"CAP", handler_cap},
  {"AUTHENTICATE", handler_authenticate},
  {"900", handler_capend},
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

    fprintf(stderr, "HHTH [%s] -> [%p]\n", hsti->name, *(void**)&hsti->func);
    HASH_ADD_KEYPTR(hh, handler_head, hsti->name, strlen(hsti->name), item);
  }
}

void
handler_lookup(char *command,
               sll_t *wq,
               char *prefix,
               char *sp) {

  handler_ht_t *item;

  HASH_FIND_STR(handler_head, command, item);

  if (item != NULL)
    (item->func)(wq, prefix, sp);
}

static void
handler_cap(sll_t *wq, char *prefix, char *sp) {

  fprintf(stderr, "handler_cap(): %s\n", strtok_r(NULL, "", &sp));

  /* lazy */

  sll_push(wq, prc_msg("AUTHENTICATE PLAIN", NULL));
}

static void
handler_authenticate(sll_t *wq, char *prefix, char *sp) {

  char *cred;

  fprintf(stderr, "auth()\n");

  /* lazy */

  if (sasl_auth("buhmin", "WorldDomination", &cred) < 0)
    return;

  sll_push(wq, prc_msg("AUTHENTICATE", cred, NULL));
}

static void
handler_capend(sll_t *wq, char *prefix, char *sp) {

  sll_push(wq, prc_msg("CAP END", NULL));
}
