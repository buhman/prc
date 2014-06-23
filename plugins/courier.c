#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "prc.h"

prc_plugin_ctor_t prc_ctor;
prc_plugin_dtor_t prc_dtor;

static prc_plugin_cmd_t consecrate_cmd;
static prc_plugin_cmd_t denounce_cmd;

prc_plugin_sym_t prc_sym[] = {
  {"consecrate", consecrate_cmd},
  {"denounce", denounce_cmd},
  {NULL},
};

static pthread_t main_thread;
static int thread_evd;
static int main_evfd;
static dll_t congregation = {NULL};
static dll_t *pwq = NULL;

typedef struct fdbuf fdbuf_t;

struct fdbuf {
  int fd;
  unsigned char off;
  char buf[256];
};

static int
lbind(unsigned short port)
{
  int sfd, ret;
  struct sockaddr_in6 sa;

  sfd = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (sfd < 0) {
    perror("socket");
    return sfd;
  }

  ret = 1;
  ret = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &ret, sizeof(int));
  if (ret < 0) {
    perror("SO_REUSEADDR");
    return sfd;
  }

  memset(&sa, 0, sizeof(sa));
  sa = (struct sockaddr_in6){
    .sin6_family = AF_INET6,
    .sin6_port = htons(port),
    .sin6_addr = IN6ADDR_ANY_INIT,
  };

  ret = bind(sfd, (struct sockaddr*)&sa, sizeof(sa));
  if (ret < 0) {
    perror("bind");
    return ret;
  }

  ret = listen(sfd, 1024);
  if (ret < 0) {
    perror("listen");
    return ret;
  }

  return sfd;
}

static void
pump_msg(const char *buf)
{
  int ret;
  dll_link_t *li;
  char evbuf[8] = {0};

  li = congregation.head;

  while (li) {

    dll_enq(pwq, prc_msg2("PRIVMSG", li->buf, buf));

    li = li->next;
  }

  evbuf[PSIG_PLUGIN] = 1;

  fprintf(stderr, "pump main_evfd: %d\n", main_evfd);

  ret = write(main_evfd, evbuf, 8);
  if (ret < 0)
    perror("write(thread_evd)");
}

static int
eaccept(int efd, int sfd)
{
  struct epoll_event ev;
  fdbuf_t *fb;
  int ret, afd;

  afd = accept4(sfd, NULL, NULL, SOCK_NONBLOCK);
  if (afd < 0) {
    perror("accept4");
    return afd;
  }

  fb = malloc(sizeof(fdbuf_t));
  fb->fd = afd;
  fb->off = 0;

  ev.events = EPOLLIN;
  ev.data.ptr = fb;

  ret = epoll_ctl(efd, EPOLL_CTL_ADD, afd, &ev);
  if (ret < 0) {
    perror("epoll_ctl");
    free(fb);
    return ret;
  }

  return 0;
}

static void *
courier_main(void *arg)
{
  struct epoll_event evs[10], *evi;
  int sfd, ret, efd;
  fdbuf_t *fb;
  ssize_t rsize;

  sfd = lbind(9001);

  efd = epoll_create1(EPOLL_CLOEXEC);
  if (efd < 0) {
    perror("epoll_create1");
    return NULL;
  }

  {
    evs->events = EPOLLIN;
    evs->data.fd = sfd;

    ret = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, evs);
    if (ret < 0) {
      perror("epoll_ctl");
      return NULL;
    }

    evs->data.fd = thread_evd;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, thread_evd, evs);

    if (ret < 0) {
      perror("epoll_ctl");
      return NULL;
    }
  }

  while (true) {

    ret = epoll_wait(efd, evs, 10, -1);
    if (ret < 0) {
      perror("epoll_wait");
      return NULL;
    }

    for (evi = evs; evi < evs + ret; evi++) {

      if (evi->data.fd == sfd) {

        ret = eaccept(efd, sfd);
        if (ret < 0)
          continue;

      }
      else if (evi->data.fd == thread_evd) {

        fprintf(stderr, "thread_evd");
        goto quit;
      }
      else {

        fb = evi->data.ptr;

        while (true) {
          rsize = recv(fb->fd, fb->buf + fb->off, 254 - fb->off, 0);
          if (rsize < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
              break;
            perror("recv");
          }
          else if (rsize != 254 - fb->off && rsize != 0) {
            fb->off += rsize;
            continue;
          }

          close(fb->fd);
          if (fb->off != 0) {
            *(fb->buf + fb->off) = '\0';
            pump_msg(fb->buf);
          }
          free(fb);
          break;
        }
      }
    }
  }

 quit:
  ret = close(sfd);
  ret = close(efd);

  return NULL;
}

void
consecrate_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  if (!pwq)
    pwq = wq;

  dll_enq(&congregation, strdup(target));

  dll_enq(wq, prc_msg2("PRIVMSG", target, "[anointed]"));
}

void
denounce_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  dll_link_t *li;

  if (!target)
    return;

  li = congregation.head;

  while (li) {

    if (strcmp(li->buf, target) == 0) {
      free(li->buf);
      dll_remove(&congregation, li);
      dll_enq(wq, prc_msg2("PRIVMSG", target, "[expunged]"));
      return;
    }

    li = li->next;
  }

  dll_enq(wq, prc_msg2("PRIVMSG", target, "[not found]"));
}

int
prc_ctor(bdb_t *b, int fd)
{
  int ret;

  main_evfd = fd;

  thread_evd = eventfd(0, EFD_NONBLOCK);
  if (thread_evd < 0) {
    perror("eventfd");
    return thread_evd;
  }

  ret = pthread_create(&main_thread, NULL, courier_main, NULL);
  if (ret < 0)
    return ret;

  return 0;
}

int
prc_dtor(void)
{
  int ret;
  char buf[8] = {0}, *ibuf;

  *buf = 1;

  ret = write(thread_evd, buf, 8);
  if (ret < 0) {
    perror("write");
    return ret;
  }

  ret = pthread_join(main_thread, NULL);
  if (ret < 0) {
    perror("pthread_join");
    return ret;
  }

  while ((ibuf = dll_pop(&congregation)) != NULL)
    free(ibuf);

  return 0;
}
