#include <stdio.h>
#include "prc.h"

#include "chess.h"

static prc_plugin_cmd_t autoprint_cmd;
static prc_plugin_cmd_t print_cmd;
static prc_plugin_cmd_t move_cmd;
static prc_plugin_cmd_t undo_cmd;

prc_plugin_sym_t prc_sym[] = {
  {"autoprint", autoprint_cmd},
  {"print", print_cmd},
  {"move", move_cmd},
  {"undo", undo_cmd},
};

static char *board[8] = {
  B_RO " " B_KN " " B_BI " " B_QU " " B_KI " " B_BI " " B_KN " " B_RO, //8
  B_PA " " B_PA " " B_PA " " B_PA " " B_PA " " B_PA " " B_PA " " B_PA,
  ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS,
  ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS,
  ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS,
  ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS " " ELPS,
  W_PA " " W_PA " " W_PA " " W_PA " " W_PA " " W_PA " " W_PA " " W_PA,
  W_RO " " W_KN " " W_BI " " W_QU " " W_KI " " W_BI " " W_KN " " W_RO,
};

static sll_t *moves;

static int autoprint = 0;

static void
autoprint_cmd(sll_t *wq, char *prefix, char* target, char *args)
{
  autoprint = !autoprint;
  if (autoprint)
    sll_push(wq, prc_msg("PRIVMSG", target, ":[enabled]", NULL));
  else
    sll_push(wq, prc_msg("PRIVMSG", target, ":[disabled]", NULL));
}

static void
print_cmd(sll_t *wq, char *prefix, char* target, char *args)
{
  char **b, **bi, **bii, *move;
  char temp[2];

  b = malloc(sizeof(board));
  for (bii = b, bi = board; bii < b + 8; bi++, bii++)
    *bii = strdup(*bi);

  sll_link_t *li = moves->head;

  while (li) {
    move = li->buf;

    {
      char *x, *y;

      x = (*(b + move[Y1]) + move[X1] * 4);
      y = (*(b + move[Y2]) + move[X2] * 4);

      memcpy(y, x, 3);
      memcpy(x, ELPS, 3);
    }

    li = li->next;
  }

  for (bi = b; bi < b + 8; bi++) {
    snprintf(temp, 2, "%d", (int)(8 - (bi - b)));
    sll_push(wq, prc_msg("PRIVMSG", target, ":", temp, *bi,  NULL));
    free(*bi);
  }

  free(b);

  sll_push(wq, prc_msg("PRIVMSG", target, ":  a b c d e f g h", NULL));
}

static void
move_cmd(sll_t *wq, char *prefix, char* target, char *args)
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
        pos[i] = 7 - (c - 49);
      else if (!(i % 2) && c > 96 && c < 105) //if Xx
        pos[i] = c - 97;
      else
        goto invalid;
    }

    {
      sll_push(moves, pos);

      if (autoprint)
        print_cmd(wq, prefix, target, args);
    }

    return;
  }

 invalid:
  sll_push(wq, prc_msg("PRIVMSG", target, "[invalid]", NULL));
}

static void
undo_cmd(sll_t *wq, char *prefix, char* target, char *args)
{
  sll_link_t *li, *li_prev = NULL;

  if (!moves->head)
    return;

  li = moves->head;

  while (li->next) {
    li_prev = li;
    li = li->next;
  }

  if(!li_prev) {
    moves->head = NULL;
    return;
  }

  li_prev->next = NULL;
  moves->tail = li_prev;
  free(li->buf);
  free(li);

  if (autoprint)
    print_cmd(wq, prefix, target, args);
}

int
prc_ctor()
{
  moves = calloc(1, sizeof(sll_t));

  return 0;
}

int
prc_dtor()
{
  free(moves);

  return 0;
}
