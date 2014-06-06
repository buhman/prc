#pragma once

typedef struct cfg cfg_t;
typedef struct cfg_file cfg_file_t;
typedef struct cfg_net cfg_net_t;

enum cfg_token {
  CFG_NETWORK = '@', /* network/port */
  CFG_CREDENTIAL = '%', /* user/nick/pass */
  CFG_CHANNEL = '*',
  CFG_PLUGIN = '$',
};

struct cfg {
  dll_t *networks; /* cfg_net */
  dll_t *plugins; /* char */
  cfg_file_t *file;
};

struct cfg_file {
  int fd;
  char *buf;
  size_t size;
};

struct cfg_net {
  char *node;
  char *service;
  char *nick;
  char *username;
  char *password;
  dll_t *channels; /* char */
};

cfg_t *
cfg_create(void);

void
cfg_free(cfg_t *cfg);

int
cfg_open(const char *path, cfg_file_t *cfg);

int
cfg_close(cfg_file_t *cfg);

int
cfg_parse(cfg_t *cfg);
