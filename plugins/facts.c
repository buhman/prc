#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "prc.h"

#include "facts.h"

prc_plugin_sym_t prc_sym[] = {
  {"fact_find", facts_find_handler},
  {"fact_add", facts_add_handler},
};

static fact_ht_t *facts_head;
static const char *db_path;

static void
facts_find_handler(dll_t *wq, char *prefix, char *target, char *tok)
{
  char *fact;

  if (!tok)
    return;

  {
    fact = facts_get(tok);

    if (fact) {
      dll_enq(wq, prc_msg2("PRIVMSG", target, "%s", fact));
      free(fact);
    }
    else
      fprintf(stderr, "FACT [%s] not found\n", tok);
  }
}

static void
facts_add_handler(dll_t *wq, char *prefix, char *target, char *tok)
{
  char *key, *fact, *sp;
  const char *status;

  if (!tok)
    return;

  key = strtok_r(tok, "`", &sp);
  if (key == NULL)
    fprintf(stderr, "FACTADD: no key");

  fact = strtok_r(NULL, "", &sp);
  if (fact == NULL)
    fprintf(stderr, "FACTADD: no fact");

  if (fact != NULL && key != NULL)
    status = facts_add(key, fact) >= 0 ? "[success]" : "[failure]";
  else
    status = "[failure]";

  dll_enq(wq, prc_msg2("PRIVMSG", target, status));
}

static int
facts_add(const char *key,
          const char *fact)
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

static char *
facts_get(const char *key)
{
  fact_ht_t *item;
  HASH_FIND_STR(facts_head, key, item);

  if (item)
    return strdup(item->fact);
  else
    return NULL;

  free(item);
}

static int
facts_init_ht(const char *path)
{
  int dbfd, err;
  fact_ht_t *item;
  char *buf, *ibuf, *ptr, *sp, *tok;
  size_t size;

  facts_head = NULL;

  {
    db_path = path;

    {
      struct stat db_stat;

      err = stat(path, &db_stat);
      if (err < 0) {
        perror("stat");
        return err;
      }

      size = db_stat.st_size;
      buf = malloc(size);
      ibuf = buf;
    }

    {
      ssize_t ret;

      dbfd = open(path, O_RDONLY);
      if (dbfd < 0) {
        perror("open");
        return dbfd;
      }
      ret = read(dbfd, buf, size);

      if (ret < 0) {
        perror("read");
        return ret;
      }

      if ((size_t)ret != size) {
        fprintf(stderr, "ret != size\n");
        return -1;
      }
    }
  }

  {
    while ((ptr = memchr(ibuf, '\n', size - (ibuf - buf))) != NULL) {

      *ptr = '\0';

      item = malloc(sizeof(fact_ht_t));

      tok = strtok_r(ibuf, "`", &sp);
      item->key = strdup(tok);

      tok = strtok_r(NULL, "", &sp);
      item->fact = strdup(tok);

      HASH_ADD_KEYPTR(hh, facts_head, item->key, strlen(item->key), item);

      ibuf = ptr + 1;
    }
  }

  free(buf);
  close(dbfd);

  return 0;
}

static void
facts_destroy_ht()
{
  static fact_ht_t *item, *tmp;

  HASH_ITER(hh, facts_head, item, tmp) {
    HASH_DELETE(hh, facts_head, item);
    free(item->fact);
    free(item->key);
    free(item);
  }
}

int
prc_ctor(bdb_t *bdb, int evfd)
{
  int err;

  err = facts_init_ht("/home/zack/db");
  if (err < 0) {
    fprintf(stderr, "facts_init_ht()\n");
    return -1;
  }

  return 0;
}

int
prc_dtor()
{
  facts_destroy_ht();

  return 0;
}
