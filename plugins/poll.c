#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "prc.h"
#include "poll.h"

prc_plugin_sym_t prc_sym[] = {
  {"poll.open", open_cmd},
  {"poll.vote", vote_cmd},
  {"poll.count", count_cmd},
  {"poll.close", close_cmd},
  {"poll.adhoc", adhoc_cmd},
  {NULL},
};

static poll_ht_t *poll_head = NULL;

static void
open_cmd(dll_t *wq, char *prefix, char *target, char *tok)
{
  char *ptr, *name, *tmp;
  ballot_ht_t *ballot_head = NULL, *item;
  poll_ht_t *poll;

  time_t ti = -1;

  if (!tok) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
    return;
  }

  name = tok;

  while (true) {

    ptr = strchr(tok, ' ');

    if (ptr && *(ptr + 1) != '\0')
      *ptr = '\0';

    fprintf(stderr, "%s\n", tok);

    if (ti < 0 && tok != name) {
      ti = strtoll(tok, &tmp, 10);
      if (*tmp != '\0') {
        dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
        return;
      }

      if (ti != 0)
        ti += time(NULL);
    }
    else if (tok != name) {
      item = calloc(1, sizeof(ballot_ht_t));
      item->key = strdup(tok);

      HASH_ADD_KEYPTR(hh, ballot_head, item->key, strlen(item->key), item);
    }
    else {
      HASH_FIND(hh, poll_head, name, strlen(name), poll);
      if (poll) {
        dll_enq(wq, prc_msg2("PRIVMSG", target, "[poll already exists]"));
        return;
      }
    }

    if (!ptr || *ptr != '\0')
      break;

    tok = ptr + 1;
  }

  poll = calloc(1, sizeof(poll_ht_t));
  poll->name = strdup(name);
  poll->ballot = ballot_head;
  poll->time = ti;

  HASH_ADD_KEYPTR(hh, poll_head, poll->name, strlen(poll->name), poll);

  dll_enq(wq, prc_msg2("PRIVMSG", target, "[success]"));
}

static void
vote_cmd(dll_t *wq, char *prefix, char *target, char *tok)
{
  poll_ht_t *poll = NULL;
  vote_ht_t *vote = NULL;
  ballot_ht_t *ballot = NULL;
  char *ptr, *nick, *ptr2;

  if (!tok) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
    return;
  }

  ptr = strchr(tok, ' ');
  if (ptr && *(ptr + 1) != '\0')
    *ptr = '\0';
  else {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
    return;
  }

  ptr2 = strchr(ptr + 1, ' ');
  if (ptr2)
    *ptr2 = '\0';

  HASH_FIND(hh, poll_head, tok, strlen(tok), poll);
  if (!poll) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[poll not found]"));
    return;
  }

  if (poll->time < time(NULL) && poll->time != 0) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[poll closed %d]", poll->time));
    return;
  }

  nick = prc_prefix_parse(prefix, NICK);
  HASH_FIND(hh, poll->vote, nick, strlen(nick), vote);
  if (vote) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[nick already voted]"));
    return;
  }

  HASH_FIND(hh, poll->ballot, ptr + 1, strlen(ptr + 1), ballot);
  if (!ballot) {
    if (poll->adhoc) {
      ballot = calloc(1, sizeof(ballot_ht_t));
      ballot->key = strdup(ptr + 1);

      HASH_ADD_KEYPTR(hh, poll->ballot, ballot->key, strlen(ballot->key), ballot);
    }
    else {
      dll_enq(wq, prc_msg2("PRIVMSG", target, "[key not in ballot]"));
      return;
    }
  }

  vote = calloc(1, sizeof(poll_ht_t));
  vote->nick = strdup(nick);
  vote->key = strdup(ptr + 1);

  HASH_ADD_KEYPTR(hh, poll->vote, vote->nick, strlen(vote->nick), vote);

  dll_enq(wq, prc_msg2("PRIVMSG", target, "[success]"));
}

static void
count_cmd(dll_t *wq, char *prefix, char *target, char *tok)
{
  poll_ht_t *poll;
  ballot_ht_t *ballot, *btemp;
  vote_ht_t *vote, *vtemp;
  int count;
  long long rem;

  if (!tok) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
    return;
  }

  HASH_FIND(hh, poll_head, tok, strlen(tok), poll);

  if (!poll) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[poll does not exist]"));
    return;
  }

  if (poll->time != 0) {
    rem = (long long)(poll->time - time(NULL));
    rem = rem > 0 ? rem : 0;
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[time]: %lld", rem));
  }
  else {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[time]: infinite"));
  }

  // = malloc(sizeof(int*) * HASH_CNT(hh, poll->ballot));

  HASH_ITER(hh, poll->ballot, ballot, btemp) {
    count = 0;

    HASH_ITER(hh, poll->vote, vote, vtemp) {

      if (strcmp(vote->key, ballot->key) == 0)
        count++;
    }

    dll_enq(wq, prc_msg2("PRIVMSG", target, "'%s': %d", ballot->key, count));
  }
}

static void
close_cmd(dll_t *wq, char *prefix, char *target, char *tok)
{
  poll_ht_t *poll;

  if (!tok) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
    return;
  }

  HASH_FIND(hh, poll_head, tok, strlen(tok), poll);

  if (!poll) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[poll does not exist]"));
    return;
  }

  poll->time = time(NULL);

  dll_enq(wq, prc_msg2("PRIVMSG", target, "[success]"));
}

static void
adhoc_cmd(dll_t *wq, char *prefix, char *target, char *tok)
{
  poll_ht_t *poll;

  if (!tok) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid]"));
    return;
  }

  HASH_FIND(hh, poll_head, tok, strlen(tok), poll);

  if (!poll) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[poll does not exist]"));
    return;
  }

  poll->adhoc = !poll->adhoc;

  dll_enq(wq, prc_msg2("PRIVMSG", target, "[adhoc: %d]", poll->adhoc));
}

static void
destroy_poll(poll_ht_t *poll)
{
  ballot_ht_t *ballot, *btemp;
  vote_ht_t *vote, *vtemp;

  HASH_DEL(poll_head, poll);
  free(poll->name);

  HASH_ITER(hh, poll->ballot, ballot, btemp) {
    HASH_DEL(poll->ballot, ballot);
    free(ballot->key);
    free(ballot);
  }

  HASH_ITER(hh, poll->vote, vote, vtemp) {
    HASH_DEL(poll->vote, vote);
    free(vote->nick);
    free(vote->key);
    free(vote);
  }

  free(poll);
}

int
prc_dtor()
{
  poll_ht_t *poll, *ptemp;

  fprintf(stderr, "poll_dtor\n");

  HASH_ITER(hh, poll_head, poll, ptemp) {
    destroy_poll(poll);
  }

  return 0;
}
