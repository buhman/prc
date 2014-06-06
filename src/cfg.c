#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/mman.h>

#include "dll.h"
#include "cfg.h"

cfg_t *
cfg_create(void)
{
  cfg_t *cfg;

  cfg = malloc(sizeof(cfg_t));

  cfg->file = malloc(sizeof(cfg_file_t));

  cfg->networks = calloc(1, sizeof(dll_t));

  cfg->plugins = calloc(1, sizeof(dll_t));

  return cfg;
}

void
cfg_free(cfg_t *cfg)
{
  cfg_net_t *net;

  while ((net = dll_pop(cfg->networks)) != NULL) {
    while (dll_pop(net->channels) != NULL);
    free(net->channels);
    free(net);
  }

  while(dll_pop(cfg->plugins) != NULL);

  free(cfg->networks);
  free(cfg->plugins);
  free(cfg->file);
  free(cfg);
}

int
cfg_open(const char *path, cfg_file_t *cfg)
{
  int err;
  struct stat s;

  {
    cfg->fd = open(path, O_RDONLY);
    if (cfg->fd < 0)
      return cfg->fd;

    err = fstat(cfg->fd, &s);
    if (err < 0)
      return err;

    cfg->buf = mmap(NULL, s.st_size + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, cfg->fd, 0);
    if (cfg->buf == MAP_FAILED)
      return -1;

    cfg->size = s.st_size;
  }

  return 0;
}

int
cfg_close(cfg_file_t *cfg)
{
  int err;

  err = munmap(cfg->buf, cfg->size);
  if (err < 0)
    return err;

  err = close(cfg->fd);
  if (err < 0)
    return err;

  return 0;
}

int
cfg_parse(cfg_t *cfg)
{
  char *bufi, *tok, *tok1, *tok2;
  long long count = 0;
  cfg_net_t *network = NULL;

  bufi = cfg->file->buf;

  tok = memchr(bufi, '\n', cfg->file->size - (bufi - cfg->file->buf));

  while (tok != NULL && tok < (cfg->file->buf + cfg->file->size) - 2) {

    count++;

    bufi = memchr(tok + 1, '\n', cfg->file->size - (tok - cfg->file->buf));
    if (bufi == NULL)
      *(cfg->file->buf + cfg->file->size + 1) = '\0';
    else
      *bufi = '\0';

    switch (*(tok + 1)) {
    case CFG_NETWORK:
      tok1 = strchr(tok + 2, '/');
      if (!tok1 || tok1 > bufi) {
        fprintf(stderr, "invalid network name declaration: %lld\n", count);
        return -1;
      }
      *tok1 = '\0';

      network = calloc(1, sizeof(cfg_net_t));
      network->channels = calloc(1, sizeof(dll_t));
      network->node = tok + 2;
      network->service = tok1 + 1;

      dll_enq(cfg->networks, network);

      fprintf(stderr, "node: %s ; service: %s\n", tok + 2, tok1 + 1);

      break;
    case CFG_CREDENTIAL:
      if (!network) {
        fprintf(stderr, "credential before network declaration %lld\n", count);
        return -1;
      }

      if (network->nick) {
        fprintf(stderr, "duplicate credential %lld\n", count);
        return -1;
      }

      tok1 = strchr(tok + 2, '/');
      if (!tok1 || tok1 > bufi) {
        network->nick = tok + 2;
        break;
      }
      *tok1 = '\0';

      network->nick = tok + 2;

      tok2 = strchr(tok1 + 1, '/');
      if (!tok2 || tok2 > bufi) {
        fprintf(stderr, "invalid credential declaration: %lld\n", count);
        return -1;
      }
      *tok2 = '\0';

      network->username = tok1 + 1;
      network->password = tok2 + 1;

      fprintf(stderr, "creds: [%s] / [%s] / [%s]\n", tok + 2, tok1 + 1, tok2 + 1);

      break;
    case CFG_CHANNEL:
      if (!network) {
        fprintf(stderr, "channel before network declaration %lld\n", count);
        return -1;
      }

      dll_enq(network->channels, tok + 2);

      fprintf(stderr, "channel: %s\n", tok + 2);

      break;
    case CFG_PLUGIN:

      dll_enq(cfg->plugins, tok + 2);

      fprintf(stderr, "plugin: %s\n", tok + 2);

      break;
    }

    tok = bufi;
  }

  return 0;
}
