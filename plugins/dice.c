#define _XOPEN_SOURCE 700

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "prc.h"

static prc_plugin_cmd_t dice_cmd;

static char *faces[] = {
  "\u2680", /* ⚀ */
  "\u2681", /* ⚁ */
  "\u2682", /* ⚂ */
  "\u2683", /* ⚃ */
  "\u2684", /* ⚄ */
  "\u2685", /* ⚅ */
};


prc_plugin_sym_t prc_sym[] = {
  {"dice", dice_cmd},
  {NULL},
};

static void
dice_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  long long num, i, flen;
  char *buf, *bufi, *tok;

  if (!args)
    return;

  tok = strchr(args, ' ');
  if (tok)
    *tok = '\0';

  num = strtoll(args, &tok, 10);
  if (*tok != '\0') {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
    return;
  }

  flen = strlen(faces[0]);

  buf = malloc(MSG_SIZE);

  for (i = 0, bufi = buf; i < num; i++, bufi += flen) {

    if ((bufi - buf) + flen > MSG_SIZE - 1) {
      *bufi = '\0';
      bufi = buf;
      dll_enq(wq, prc_msg2("PRIVMSG", target, "%s", buf));
    }

    strcpy(bufi, faces[random() % 6]);
  }

  *bufi = '\0';

  dll_enq(wq, prc_msg2("PRIVMSG", target, "%s", buf));

  free(buf);
}

int
prc_ctor()
{
  srandom(time(NULL));

  return 0;
}
