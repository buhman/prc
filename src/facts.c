#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "dbuf.h"
#include "facts.h"
#include "sll.h"

static fact_ht_t *facts_head;

char *
facts_get(char *key)
{
  fact_ht_t *item;
  HASH_FIND_STR(facts_head, key, item);

  if (item)
    return strdup(item->fact);
  else
    return NULL;

  free(item);
}

int
facts_init_ht(char *path)
{
  int dbfd;
  fact_ht_t *item;
  char *buf, *ibuf, *ptr, *sp, *tok, *key;
  ssize_t size;

  facts_head = NULL;

  {
    dbfd = open(path, O_RDONLY);

    size = dbuf_read(dbfd, DBUF_READ, &buf);
    if (size < 0)
      return size;

    ibuf = buf;
  }

  {
    printf("rem %d\n", size - (ibuf - buf));
    while ((ptr = memchr(ibuf, '\n', size - (ibuf - buf))) != NULL) {

      *ptr = '\0';

      tok = strtok_r(ibuf, " ", &sp);
      key = malloc(strlen(tok) + 1);
      strcpy(key, tok);

      item = malloc(sizeof(fact_ht_t));

      tok = strtok_r(NULL, "", &sp);
      item->fact = malloc(strlen(tok) + 1);
      strcpy(item->fact, tok);

      HASH_ADD_KEYPTR(hh, facts_head, key, strlen(key), item);

      ibuf = ptr + 1;
    }
  }

  free(buf);

  return 0;
}
