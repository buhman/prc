#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>

#include "sll.h"

void
sll_push(sll_t *ol,
         char *buf)
{
  sll_link_t *li;

  li = calloc(1, sizeof(sll_link_t));
  li->buf = buf;

  if (ol->head == NULL) {
    ol->head = li;
  }
  else {
    assert(ol->tail != NULL);
    ol->tail->next = li;
  }

  ol->tail = li;
}

void
sll_pop(sll_t *ol,
        char **obuf)
{
  sll_link_t *li;

  /*
  if (*obuf != NULL) {
    free(*obuf);
  }
  */

  li = ol->head;
  *obuf = li->buf;
  ol->head = li->next;
  free(li);
}
