#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>

#include "dll.h"

/* fail fast, fail hard */
#ifdef DEBUG_ASSERT
static void
list_assert(const dll_t *ol)
{
  /* head and tail must either both exist or this is an empty list */
  assert((!ol->head && !ol->tail) || (ol->head && ol->tail));

  /* if the head exists, there is no previous element */
  assert(!ol->head || !ol->head->prev);

  /* if the tail exists, there is no next element */
  assert(!ol->tail || !ol->tail->next);

  /* link prev/next must match the state of head/tail */
  assert(!ol->head ||
         ((ol->tail->prev == ol->head) && (ol->tail->prev == ol->head &&
                                           ol->head->next == ol->tail)) ||
         ((ol->tail->prev != ol->head) && (ol->tail->prev != ol->head &&
                                           ol->head->next != ol->tail))
         );
}

static void
link_assert(const dll_link_t *li)
{
  assert(!li->prev || (li->prev->next == li));

  assert(!li->next || (li->next->prev == li));

  /* that's it? really? I think so.. provided list_assert() is also true */
}
#endif

/* enqueue adds an element at the tail of the list */
void
dll_enq(dll_t *ol,
        void *buf)
{
  dll_link_t *li;

  li = calloc(1, sizeof(dll_link_t));
  li->buf = buf;

#ifdef DEBUG_ASSERT
  list_assert(ol);
#endif

  {
    /* prev is NULL if tail does not yet exist */
    li->prev = ol->tail;

    /* this is the head element if no head element exists */
    if (!ol->head)
      ol->head = li;

    /* old tail has a new next */
    if (ol->tail)
      ol->tail->next = li;

    /* this is always the tail element */
    ol->tail = li;

    /* this is at the end of the list, next is always NULL */
    li->next = NULL;
  } /* ... */

#ifdef DEBUG_ASSERT
  list_assert(ol);
#endif
}

/* pop removes an element at the head of the list */
void*
dll_pop(dll_t *ol)
{
  char *obuf;
  dll_link_t *li;

#ifdef DEBUG_ASSERT
  list_assert(ol);
#endif

  {
    if (!ol->head)
      return NULL;
    li = ol->head;

    /* the next element is now the first */
    ol->head = li->next;

    /* update the tail if the head no longer exists */
    if (!ol->head)
      ol->tail = NULL;

    /* the prev element no longer exists */
    if (li->next)
      li->next->prev = NULL;
  } /* ... */

#ifdef DEBUG_ASSERT
  list_assert(ol);
#endif

  obuf = li->buf;
  free(li);

  return obuf;
}

/* push adds an element to the head of the list */
void
dll_push(dll_t *ol,
         void *buf)
{
  dll_link_t *li;

  li = calloc(1, sizeof(dll_link_t));
  li->buf = buf;

#ifdef DEBUG_ASSERT
  list_assert(ol);
#endif

  {
    /* next is NULL if head does not yet exist */
    li->next = ol->head;

    /* this is the tail element if no tail exists */
    if (!ol->tail)
      ol->tail = li;

    /* old head prev is this element */
    if (ol->head)
      ol->head->prev = li;

    /* we are the new head */
    ol->head = li;

    /* prev does not exist */
    li->prev = NULL;
  } /* ... */

#ifdef DEBUG_ASSERT
  list_assert(ol);
#endif
}

/* remove link from list by updating neighboring links */
void
dll_remove(dll_t *ol,
           dll_link_t *li)
{
#ifdef DEBUG_ASSERT
  list_assert(ol);
  link_assert(li);
#endif

  {
    /* previous link's next should be this link's next */
    if (li->prev)
      li->prev->next = li->next;
    else
      ol->head = li->next;

    /* next link's prev should be this link's prev */
    if (li->next)
      li->next->prev = li->prev;
    else
      ol->tail = li->prev;
  } /* ... */

#ifdef DEBUG_ASSERT
  list_assert(ol);
  link_assert(li->next);
  link_assert(li->prev);
#endif

  free(li);
}

/* insert new link after given link (->next) */

void
dll_insert(dll_t *ol,
           dll_link_t *li,
           void *buf)
{
  dll_link_t *nli;

  nli = malloc(sizeof(dll_link_t));
  nli->buf = buf;

#ifdef DEBUG_ASSERT
  list_assert(ol);
  link_assert(li);
#endif

  {
    /* the new link's prev is this link */
    nli->prev = li;

    /* the new link's next is this link's next link */
    nli->next = li->next;

    /* the current next's prev is now this link */
    if (li->next)
      li->next->prev = nli;
    else
      ol->tail = nli;

    li->next = nli;
  } /* ... */

#ifdef DEBUG_ASSERT
  list_assert(ol);
  link_assert(li);
  link_assert(nli);
#endif
}
