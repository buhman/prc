#include <stdio.h>
#include "prc.h"

#include "chess.h"

prc_plugin_sym_t prc_sym[] = {
  {"autoprint", autoprint_cmd},
  {"print", print_cmd},
  {"move", move_cmd},
  {"undo", undo_cmd},
  {"snap", snap_cmd}
};

static const char *board[8] = {
  B_RO " " B_KN " " B_BI " " B_QU " " B_KI " " B_BI " " B_KN " " B_RO, //8
  B_PA " " B_PA " " B_PA " " B_PA " " B_PA " " B_PA " " B_PA " " B_PA,
  ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS,
  ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS,
  ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS,
  ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS,
  W_PA " " W_PA " " W_PA " " W_PA " " W_PA " " W_PA " " W_PA " " W_PA,
  W_RO " " W_KN " " W_BI " " W_QU " " W_KI " " W_BI " " W_KN " " W_RO,
};

static dll_t *moves;

static int autoprint = 0;

static void
autoprint_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  autoprint = !autoprint;
  if (autoprint)
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[enabled]", NULL));
  else
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[disabled]", NULL));
}

static void
print_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  char **b, **bi, **bii;
  const char *move;
  dll_link_t *li;

  b = malloc(sizeof(board));
  for (bii = b, bi = (char**)board; bii < b + 8; bi++, bii++)
    *bii = strdup(*bi);

  li = moves->tail;

  while (li) {
    move = li->buf;

    {
      char *x, *y;

      x = (*(b + move[Y1]) + move[X1] * 4);
      y = (*(b + move[Y2]) + move[X2] * 4);

      memcpy(y, x, 3);
      memcpy(x, ELPS, 3);
    }

    li = li->prev;
  }

  for (bi = b; bi < b + 8; bi++) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "%d %s", (int)(8 - (bi - b)), *bi));
    free(*bi);
  }

  free(b);

  dll_enq(wq, prc_msg2("PRIVMSG", target, "  a b c d e f g h"));
}

static void
move_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  //a1h8
  //97 49 104 56;
  //0,7 -> 7,0

  char *pos, *tok;
  int c; // component

  if (!args)
    goto invalid;

  pos = malloc(sizeof(char[4]));

  tok = strchr(args, ' ');
  if (tok)
    *tok = '\0';

  if (strlen(args) == 4) {

    for (int i = 0; i < 4; i++) {
      c = *(args + i);
      if (i % 2 && c > 48 && c < 57) //if Yx
        pos[i] = 56 - c;
      else if (!(i % 2) && c > 96 && c < 105) //if Xx
        pos[i] = c - 97;
      else
        goto invalid;
    }

    {
      dll_push(moves, pos);

      if (autoprint)
        print_cmd(wq, prefix, target, args);
    }

    return;
  }

 invalid:
  dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
}

static void
undo_cmd(dll_t *wq, char *prefix, char* target, char *args)
{

  dll_pop(moves);

  if (autoprint)
    print_cmd(wq, prefix, target, args);
}

static void
snap_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  char *buf, *bufi, *c;
  dll_link_t *li = moves->tail;

  buf = malloc(MSG_SIZE);
  bufi = buf;

  while (li) {
    for (c = (char*)li->buf; c < (char*)li->buf + 4; c++) {

      if ((c - (char*)li->buf) % 2) //if Yx
        *bufi = 56 - *c;
      else //if Xx
        *bufi = *c + 97;

      bufi++;
    }

    *bufi = ',';
    bufi++;

    li = li->prev;
  }

  *(bufi - 1) = '\0';

  dll_enq(wq, prc_msg2("PRIVMSG", target, buf));

  free(buf);
}

int
prc_ctor()
{
  moves = calloc(1, sizeof(dll_t));

  return 0;
}

int
prc_dtor()
{
  free(moves);

  return 0;
}
