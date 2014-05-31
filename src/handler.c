#include <stdio.h>
#include <assert.h>

#include "prc.h"
#include "sasl.h"
#include "term.h"
#include "plugin.h"
#include "handler.h"

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

static dll_t *plugin_wq;

void
handler_init()
{
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

  plugin_wq = calloc(1, sizeof(dll_t));
}

void
handler_free()
{
  free(plugin_wq);
}

void
handler_lookup(char *command,
               dll_t *wq,
               char *prefix,
               char *buf)
{
  handler_ht_t *item;

  HASH_FIND_STR(handler_head, command, item);

  if (item != NULL)
    (item->func)(wq, prefix, buf);
}

static void
handler_cap(dll_t *wq, char *prefix, char *buf)
{
  /* lazy */

  dll_enq(wq, prc_msg("AUTHENTICATE PLAIN", NULL));
}

static void
handler_authenticate(dll_t *wq, char *prefix, char *buf)
{
  char *cred;

  /* lazy */

  if (sasl_auth("buhmin", "WorldDomination", &cred) < 0)
    return;

  dll_enq(wq, prc_msg("AUTHENTICATE", cred, NULL));

  free(cred);
}

static void
handler_capend(dll_t *wq, char *prefix, char *buf)
{
  dll_enq(wq, prc_msg("CAP END", NULL));
}

static void
handler_ping(dll_t *wq, char *prefix, char *buf)
{
  dll_enq(wq, prc_msg("PONG buhmin", NULL));
}

static void
handler_welcome(dll_t *wq, char *prefix, char *buf)
{
  dll_enq(wq, prc_msg("JOIN ##archlinux-botabuse", NULL));
}

static void
plugin_switch(char *prefix, char *target, char *msg, char *args)
{
  char *tok;

  // plugin prefix
  switch (*msg) {
  case '\001':
    plugin_lookup(plugin_wq, prefix, target, "ctcp", args);
    break;
  case '`':
    plugin_lookup(plugin_wq, prefix, target, "fact_find", args);
    break;
  case '%':
    plugin_lookup(plugin_wq, prefix, target, "fact_add", args);
    break;
  case '$':
    {
      tok = strchr(msg + 1, ' ');
      if (tok) {
        *tok = '\0';
        plugin_lookup(plugin_wq, prefix, target, args, tok + 1);
      }
      else
        plugin_lookup(plugin_wq, prefix, target, args, NULL);
    }
    break;
  case '#':
    plugin_cmd(plugin_wq, prefix, target, args);
    break;
  }
}

static void
handler_privmsg(dll_t *wq, char *prefix, char *buf)
{
  char *tok, *target, *msg, *redirect;

  tok = strchr(buf, ' ');
  if (!tok) {
    term_printf("privmsg(): no tok1");
    return;
  }
  *tok = '\0';

  target = buf;

  if (*(tok + 1) == ':')
    msg = tok + 2;
  else {
    term_printf("privmsg(): no msg: [%s] [%s] [%s]", prefix, target, tok + 1);
    return;
  }

  redirect = strchr(msg, '>');
  if (redirect)
    *redirect = '\0';

  {
    char *d1, *d2, *tok1;
    d1 = strchr(msg + 1, '[');
    if (d1) {
      d2 = strchr(d1 + 1, ']');
      if (!d2) {
        dll_enq(wq, prc_msg("PRIVMSG", target, ":[syntax error]", NULL));
        return;
      }

      tok = d1 + 1;

      while (tok1 != d2) {

        tok1 = memchr(tok, ',', d2 - tok);
        if (!tok1)
          tok1 = d2;

        {
          char *dup;
          dup = strdup(msg);
          memcpy(dup + (d1 - msg), tok, tok1 - tok);
          strcpy(dup + (d1 - msg) + (tok1 - tok), d2 + 1);

          plugin_switch(prefix, target, dup, dup + 1);
          free(dup);
        } /* ... */

        tok = tok1 + 1;
      }
    }
    else
      plugin_switch(prefix, target, msg, msg + 1);

    tok = d2 + 1;
  } /* ... */

  {
    prc_plugin_msg_t *pmsg;

    while ((pmsg = dll_pop(plugin_wq)) != NULL) {

      if (redirect && *(redirect + 1) == '>')
        dll_enq(wq, prc_msg3("%s %s :%s\r\n", pmsg->cmd, redirect + 2,
                              pmsg->buf));
      else if (redirect)
        dll_enq(wq, prc_msg3("%s %s :%s: %s\r\n", pmsg->cmd, pmsg->target,
                             redirect + 1, pmsg->buf));
      else
        dll_enq(wq, prc_msg(pmsg->cmd, pmsg->target, ":", pmsg->buf, NULL));

      free(pmsg->buf);
      free(pmsg);
    }
  } /* ... */
}
