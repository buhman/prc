#pragma once

#include <sys/epoll.h>
#include <sys/socket.h>
#include <gnutls/gnutls.h>

#include "dll.h"
#include "cfg.h"

extern int evfd;

typedef struct event_handler event_handler_t;
typedef int (eh_fptr_t)(struct epoll_event*);

struct event_handler {
  int epfd;
  int fd;
  gnutls_session_t session;
  eh_fptr_t *rf;
  eh_fptr_t *wf;
  dll_t *wq;
  cfg_net_t *cfg;
};

int
event_ev_init(int epfd);

int
event_add(int epfd,
          int fd,
          uint32_t events,
          eh_fptr_t *rf,
          eh_fptr_t *wf,
          dll_t *wq,
          cfg_net_t *cfg,
          struct epoll_event *ev);

int
event_del(int epfd,
          struct epoll_event *ev);

int
event_bind(int domain, int efd, struct sockaddr *sa);

int
event_accept(int efd, int sfd);
