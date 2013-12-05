#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

#include <sys/epoll.h>
#include <sys/socket.h>

#include "config.h"

#include "dbuf.h"
#include "epoll.h"
#include "sll.h"
#include "proto.h"

static sll_t *write_queue;

int
main(int argc,
     char **argv)
{
  int efd, err;

  {
    efd = epoll_create1(EPOLL_CLOEXEC);
    if (efd < 0) {
      perror("epoll_create1()");
      return EXIT_FAILURE;
    }
  } /* ... */

  {
    err = epoll_input(efd);
    if (err < 0) {
      perror("epoll_input()");
      return EXIT_FAILURE;
    }

    err = epoll_connect(efd, IRC_HOST, IRC_PORT);
    if (err < 0) {
      perror("epoll_connect()");
      return EXIT_FAILURE;
    }
  } /* ... */

  {
    proto_init_ht();

    write_queue = calloc(1, sizeof(sll_t));

    proto_register(write_queue);
  }

  {
    struct epoll_event *evs, *evi;
    char *buf;
    ssize_t size;
    int nfds;

    evs = calloc(MAX_EVENTS, sizeof(struct epoll_event));

    while (true) {

      nfds = epoll_wait(efd, evs, MAX_EVENTS, -1);
      if (nfds < 0) {
        perror("epoll_wait()");
        return EXIT_FAILURE;
      }

      for (evi = evs; evi < evs + nfds; evi++) {

        if (evi->events & EPOLLOUT && (int)evi->data.u64 != STDIN_FILENO) {

          if (write_queue->head != NULL) {

            buf = write_queue->head->buf;

            size = send((int)evi->data.u64, buf, strlen(buf), 0);
            if (size < 0) {
              if (errno == EAGAIN || errno == EWOULDBLOCK) {
                fprintf(stderr, "send(): EAGAIN\n");
                continue;
              }
              else {
                perror("send()");
                return EXIT_FAILURE;
              }
            }
            else if ((size_t)size != strlen(buf)) {
              fprintf(stderr, "s: %zd sl %zd\n", size, strlen(buf));
              return EXIT_FAILURE;
            }

            sll_pop(write_queue, &buf);
            printf("C: [%s]\n", buf);
            free(buf);
          }
        }
        if (evi->events & EPOLLIN) {

          err = dbuf_readp(evi->data.u64, &buf);
          if (err < 0) {
            perror("dbuf_readp()");
          }

          if ((int)evi->data.u64 == STDIN_FILENO) {
            sll_push(write_queue, buf);
          }
          else {
            char *ptr, *ibuf;

            ibuf = buf;

            while ((ptr = strchr(ibuf, '\r')) != NULL) {
              *ptr = '\0';
              proto_process(write_queue, ibuf);
              ibuf = ptr + 2;
            }

            free(buf);
          }
        }
      }
    }
  } /* ... */

  return EXIT_SUCCESS;
}
