#pragma once

#include "prc.h"
#include "event.h"

#define worker_quit(str, ret)                   \
  do {                                          \
    perror(str);                                \
                                                \
    handler_free();                             \
                                                \
    plugin_unload_all();                        \
                                                \
    close(epfd);                                \
    close(evfd);                                \
                                                \
    cfg_close(cfg->file);                       \
    cfg_free(cfg);                              \
                                                \
    return ret;                                 \
  } while (0);

int
worker_sendfd(int sfd, int fd);

int
worker_evfd(void);

int
worker_event(int epfd, struct epoll_event* evi, event_handler_t* ehi);

prc_main_t worker_main;
