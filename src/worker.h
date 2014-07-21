#pragma once

#include "event.h"

#define worker_quit(str, ret)                   \
  do {                                          \
    perror(str);                                \
                                                \
    handler_free();                             \
                                                \
    close(epfd);                                \
    close(evfd);                                \
                                                \
    cfg_close(cfg->file);                       \
    cfg_free(cfg);                              \
                                                \
    return ret;                                 \
  } while (0);

typedef int (worker_main_t)(int cfd, int fds[], int nfds);

int
worker_sendfd(int sfd, int fd);

int
worker_evfd(void);

int
worker_event(int epfd, struct epoll_event* evi, event_handler_t* ehi);

worker_main_t worker_main;
