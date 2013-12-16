#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <termios.h>
#include <fcntl.h>
#include <assert.h>

#include <sys/ioctl.h>

#include "event.h"
#include "term.h"

/* static char input_buf[510]; */

static struct winsize ws;
sll_t *term_wq;
static struct epoll_event *stdout_ev;

int
term_stdout(int epfd)
{
  int err;

  if (term_wq->head != NULL) {
    err = event_add(epfd, STDOUT_FILENO, EPOLLOUT, NULL,
                    term_write, NULL,
                    &stdout_ev);
    if (err < 0)
      perror("event_add() stdout\n");
  }
  else if (stdout_ev != NULL) {
    err = event_del(epfd, &stdout_ev);
    if (err < 0)
      perror("event_del() stdout\n");
  }

  return 0;
}

int
term_register(int epfd)
{
  int err;

  term_wq = calloc(1, sizeof(sll_t));

  err = term_setup(term_wq);
  if (err < 0) {
    perror("term_setup()");
    return err;
  }

  err = event_add(epfd, STDIN_FILENO, EPOLLIN, term_read, NULL, term_wq, NULL);
  if (err < 0) {
    perror("event_add()");
    return err;
  }

  return 0;
}

int
term_read(struct epoll_event *ev)
{
  fprintf(stderr, "term_read(): STUB\n");

  return 0;
}

int
term_write(struct epoll_event *ev)
{
  char *buf;

  while (term_wq->head) {

    sll_pop(term_wq, &buf);

    printf("async_as_fuck: [%s]\n", buf);

    free(buf);
  }

  return 0;
}

int
term_setup()
{
  int err, mode;
  struct termios t;

  stdout_ev = NULL;

  {
    err = tcgetattr(STDIN_FILENO, &t);
    if (err < 0) {
      perror("tcgetattr()");
      return err;
    }

    t.c_lflag &= ~(ECHO | ECHONL | ICANON);

    err = tcsetattr(STDIN_FILENO, TCSADRAIN, &t);
    if (err < 0) {
      perror("tcsetattr()");
      return err;
    }
  }

  {
    mode = fcntl(STDIN_FILENO, F_GETFL);
    if (mode < 0) {
      perror("fcntl() GETFL");
      return mode;
    }

    err = fcntl(STDIN_FILENO, F_SETFL, mode | O_NONBLOCK);
    if (err < 0) {
      perror("fcntl() SETFL");
      return err;
    }
  }

  {
    err = ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    if (err < 0) {
      perror("TIOCGWINSZ");
      return err;
    }
  }

  return 0;
}
