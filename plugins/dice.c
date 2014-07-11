#define _XOPEN_SOURCE 700

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "prc.h"

static prc_plugin_cmd_t dice_cmd;
prc_plugin_ctor_t prc_ctor;

static const char *d1[] = {
  "\u2609", /* ☉ */
};

static const char *d2[] = {
  "\u2686", /* ⚆ */
  "\u2687", /* ⚇ */
};

static const char *d4[] = {
  "\u2190", /* ← */
  "\u2191", /* ↑ */
  "\u2192", /* → */
  "\u2193", /* ↓ */
};

static const char *d6[] = {
  "\u2680", /* ⚀ */
  "\u2681", /* ⚁ */
  "\u2682", /* ⚂ */
  "\u2683", /* ⚃ */
  "\u2684", /* ⚄ */
  "\u2685", /* ⚅ */
};

static const char *dx[] = {
  "\u2460", /* (1) */
  "\u2461", /* (2) */
  "\u2462", /* (3) */
  "\u2463", /* (4) */
  "\u2464", /* (5) */
  "\u2465", /* (6) */
  "\u2466", /* (7) */
  "\u2467", /* (8) */
  "\u2468", /* (9) */
  "\u2469", /* (10) */
  "\u246A", /* (11) */
  "\u246B", /* (12) */
  "\u246C", /* (13) */
  "\u246D", /* (14) */
  "\u246E", /* (15) */
  "\u246F", /* (16) */
  "\u2470", /* (17) */
  "\u2471", /* (18) */
  "\u2472", /* (19) */
  "\u2473", /* (20) */

  "\u3251", /* (21) */
  "\u3252", /* (22) */
  "\u3253", /* (23) */
  "\u3254", /* (24) */
  "\u3255", /* (25) */
  "\u3256", /* (26) */
  "\u3257", /* (27) */
  "\u3258", /* (28) */
  "\u3259", /* (29) */
  "\u325A", /* (30) */
  "\u325B", /* (31) */
  "\u325C", /* (32) */
  "\u325D", /* (33) */
  "\u325E", /* (34) */
  "\u325F", /* (35) */

  "\u32B1", /* (36) */
  "\u32B2", /* (37) */
  "\u32B3", /* (38) */
  "\u32B4", /* (39) */
  "\u32B5", /* (40) */
  "\u32B6", /* (41) */
  "\u32B7", /* (42) */
  "\u32B8", /* (43) */
  "\u32B9", /* (44) */
  "\u32BA", /* (45) */
  "\u32BB", /* (46) */
  "\u32BC", /* (47) */
  "\u32BD", /* (48) */
  "\u32BE", /* (49) */
  "\u32BF", /* (50) */
};


prc_plugin_sym_t prc_sym[] = {
  {"dice", dice_cmd},
  {NULL},
};

static void
dice_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  long long num, i, flen, nface;
  char *buf, *bufi, *tok;
  const char **faces;

  nface = 6;
  if (!args)
    num = 1;
  else {
    tok = strchr(args, ' ');
    if (tok && *(tok + 1) != '\0') {
      *tok = '\0';
      nface = strtoll(tok + 1, &tok, 10);
      if (*tok != '\0' || nface > 50) {
        dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
        return;
      }
    }

    num = strtoll(args, &tok, 10);
    if (*tok != '\0') {
      dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
      return;
    }
  }

  if (nface == 1)
    faces = d1;
  else if (nface == 2)
    faces = d2;
  else if (nface == 4)
    faces = d4;
  else if (nface == 6)
    faces = d6;
  else
    faces = dx;

  flen = strlen(faces[0]);

  fprintf(stderr, "flen: %lld; faces: %lld\n", flen, nface);

  buf = malloc(MSG_SIZE);

  for (i = 0, bufi = buf; i < num; i++, bufi += flen + 1) {

    if ((bufi - buf) + flen + 1 > MSG_SIZE - 1) {
      *bufi = '\0';
      bufi = buf;
      dll_enq(wq, prc_msg2("PRIVMSG", target, "%s", buf));
      continue;
    }

    strcpy(bufi, faces[random() % nface]);
    *(bufi + flen) = ' ';
  }

  strcpy(bufi, faces[random() % nface]);
  *bufi = '\0';

  dll_enq(wq, prc_msg2("PRIVMSG", target, "%s", buf));

  free(buf);
}

int
prc_ctor(bdb_t *bdb, int evfd)
{
  srandom(time(NULL));

  return 0;
}
