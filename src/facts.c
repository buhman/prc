#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "dbuf.h"
#include "facts.h"
#include "sll.h"

static fact_ht_t *facts_head;
static char *db_path;

int
facts_add(char *key,
          char *fact)
{
  {
    fact_ht_t *item;

    HASH_FIND_STR(facts_head, key, item);

    if (item) {
      fprintf(stderr, "already exists\n");
      return -1;
    }

    item = malloc(sizeof(fact_ht_t));
    item->fact = strdup(fact);

    HASH_ADD_KEYPTR(hh, facts_head, strdup(key), strlen(key), item);
  } /* ... */

  {
    int dbfd, err;
    off_t offset;

    dbfd = open(db_path, O_WRONLY);
    if (dbfd < 0) {
      perror("open()");
      return dbfd;
    }

    offset = lseek(dbfd, 0, SEEK_END);
    if (offset < 0) {
      perror("lseek()");
      return offset;
    }

    err = dprintf(dbfd, "%s`%s\n", key, fact);
    if (err < 0) {
      perror("dprintf()");
      return err;
    }

    close(dbfd);
  } /* ... */

  return 0;
}

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
    db_path = path;
    dbfd = open(path, O_RDONLY);

    size = dbuf_read(dbfd, DBUF_READ, &buf);
    if (size < 0)
      return size;

    ibuf = buf;
  }

  {
    while ((ptr = memchr(ibuf, '\n', size - (ibuf - buf))) != NULL) {

      *ptr = '\0';

      tok = strtok_r(ibuf, "`", &sp);
      key = strdup(tok);

      item = malloc(sizeof(fact_ht_t));

      tok = strtok_r(NULL, "", &sp);
      item->fact = strdup(tok);

      HASH_ADD_KEYPTR(hh, facts_head, key, strlen(key), item);

      ibuf = ptr + 1;
    }
  }

  free(buf);
  close(dbfd);

  return 0;
}
