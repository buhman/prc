#include <string.h>

#include "prc.h"
#include "rps.h"

prc_plugin_sym_t prc_sym[] = {
  {"rps", rps_cmd},
  {"rps.test", test_cmd},
  {"rps.stats", stats_cmd},
  {"rps.watch", watch_cmd},
  {NULL},
};

static const char *moves[] = {
  "rock",
  "paper",
  "scissors",
};

static move_ht_t *moves_head = NULL;
static score_ht_t *scores_head = NULL;
static dll_t audience = {NULL};

static move_e
parse_move(char *m)
{
  int i;

  for (i = 0; i < MOVE_LAST; i++)
    if (strcmp(moves[i], m) == 0)
      return i;

  return MOVE_BAD;
}

static void
add_move(char *self, char *foe, move_e m)
{
  move_ht_t *move;
  move_key_t *key;

  key = calloc(1, sizeof(move_key_t));

  strcpy(key->self, self);
  strcpy(key->foe, foe);

  move = calloc(1, sizeof(move_ht_t));

  move->key = key;
  move->move = m;

  HASH_ADD_KEYPTR(hh, moves_head, move->key, sizeof(move_key_t), move);
}

static move_ht_t *
find_move(char *self, char *foe)
{
  move_ht_t *move;
  move_key_t key;

  memset(&key, 0, sizeof(move_key_t));
  strcpy(key.self, self);
  strcpy(key.foe, foe);

  HASH_FIND(hh, moves_head, &key, sizeof(move_key_t), move);

  return move;
}

static score_ht_t *
get_score(char *name)
{
  score_ht_t *score;

  HASH_FIND(hh, scores_head, name, strlen(name), score);

  if (!score) {
    score = calloc(1, sizeof(score_ht_t));
    score->name = strdup(name);
    HASH_ADD_KEYPTR(hh, scores_head, score->name, strlen(name), score);
  }

  return score;
}

static char *
score_move(move_e m, move_ht_t *move)
{
  char *winner;
  score_ht_t *self, *foe;
  int o;

  HASH_DELETE(hh, moves_head, move);

  o = move->move - m;
  o = o >= 0 ? o : 3 + o;

  self = get_score(move->key->self);
  foe = get_score(move->key->foe);

  switch (o) {
  case OUT_WIN:
    winner = move->key->self;
    self->win++;
    foe->loss++;
    break;
  case OUT_LOSE:
    winner = move->key->foe;
    foe->win++;
    self->loss++;
    break;
  case OUT_DRAW:
    winner = NULL;
    self->draw++;
    foe->draw++;
    break;
  }

  return winner;
}

static void
announce(dll_t *wq, const char *target, const char *a, const char *b, const char *c, const char *d)
{
  dll_link_t *li;

  dll_enq(wq, prc_msg2("PRIVMSG", target, "winner: %s (%s); loser: %s (%s)",
                       a, b, c, d));

  for (li = audience.head; li; li = li->next)
    dll_enq(wq, prc_msg2("PRIVMSG", li->buf, "winner: %s (%s); loser: %s (%s)",
                         a, b, c, d));
}

static void
rps_cmd(dll_t *wq, char *prefix, char *target, char *tok)
{
  char *nick, **args, *winner;
  move_e m;
  move_ht_t *move_foe;

  args = prc_parse_args(tok, ARG_LAST);
  if (args == NULL) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
    return;
  }

  m = parse_move(*(args + ARG_MOVE));
  if (m < 0) {
    free(args);
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[bad move]"));
    return;
  }

  nick = prc_prefix_parse(prefix, NICK);

  move_foe = find_move(args[ARG_FOE], nick);

  if (move_foe == NULL) {
    if (find_move(nick, args[ARG_FOE]) == NULL) {
      add_move(nick, args[ARG_FOE], m);
      dll_enq(wq, prc_msg2("PRIVMSG", target, "[success]"));
    }
    else
      dll_enq(wq, prc_msg2("PRIVMSG", target, "[failure]"));
  }
  else {
    winner = score_move(m, move_foe);
    if (!winner)
      announce(wq, target, NULL, moves[m], NULL, moves[move_foe->move]);
    else if (winner == move_foe->key->foe)
      announce(wq, target, winner, moves[m], move_foe->key->self, moves[move_foe->move]);
    else
      announce(wq, target, winner, moves[move_foe->move], move_foe->key->foe, moves[m]);

    free(move_foe->key);
    free(move_foe);
  }
}

static void
test_cmd(dll_t *wq, char *prefix, char *target, char *tok)
{
  dll_enq(wq, prc_msg2("PRIVMSG", target, "\u270A \u270B \u270C"));
}

static void
stats_cmd(dll_t *wq, char *prefix, char *target, char *tok)
{
  score_ht_t *score;
  char **name;

  name = prc_parse_args(tok, 1);
  if (name == NULL) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
    return;
  }

  score = get_score(*name);

  dll_enq(wq, prc_msg2("PRIVMSG", target, "win: %ld; loss: %ld; draw: %ld",
                       score->win, score->loss, score->draw));
}

static void
watch_cmd(dll_t *wq, char *prefix, char *target, char *tok)
{
  dll_enq(&audience, strdup(target));

  dll_enq(wq, prc_msg2("PRIVMSG", target, "[success]"));
}

int
prc_dtor()
{
  dll_link_t *li, *t;
  move_ht_t *move, *mtemp;
  score_ht_t *score, *stemp;

  for (li = audience.head; li; li = t) {
    free(li->buf);
    t = li->next;
    free(li);
  }

  HASH_ITER(hh, moves_head, move, mtemp) {
    HASH_DEL(moves_head, move);
    free(move->key);
    free(move);
  }

  HASH_ITER(hh, scores_head, score, stemp) {
    HASH_DEL(scores_head, score);
    free(score->name);
    free(score);
  }

  return 0;
}
