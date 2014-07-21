#pragma once

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

int
worker_evfd(void);

int
worker_event(int epfd, struct epoll_event* evi, event_handler_t* ehi);

int
worker_main(int cfd, int fds[], int nfds);
