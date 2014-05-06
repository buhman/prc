#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include <sys/ioctl.h>

#include "event.h"
#include "term.h"
#include "buf.h"
#include "prc.h"

static struct winsize ws;
sll_t *term_wq;
static struct epoll_event stdout_ev;

static char line_buf[MSG_SIZE];
static char *line_bufi = line_buf;

static char scratch[MSG_SIZE * 2];

extern event_handler_t *proto_ceh;

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
  else if (stdout_ev.data.ptr != NULL) {
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
  event_handler_t *eh = (event_handler_t*)ev->data.ptr;
  ssize_t ret;
  size_t rsize = 0;
  char raw_buf[MSG_SIZE];

  char *bufi;

  while (true) {
    ret = read(eh->fd, raw_buf, sizeof(raw_buf));
    if (ret < 0) {
      switch (errno) {
      case EAGAIN:
        break;
      default:
        perror("read()");
        return -1;
      }
      break;
    }
    else
      rsize += ret;
  }

  for(bufi = raw_buf; bufi < raw_buf + rsize; bufi++) {

    switch (*bufi) {
    case '\n':
      term_parse();

      snprintf(scratch, MSG_SIZE, "" FORCE_CURSORd ERASE_LINE, ws.ws_row);
      sll_push(term_wq, strdup(scratch));

      break;
    case '\b':
    case 127:
      if (line_bufi > line_buf) {
        sll_push(term_wq, strdup(BACKSPACE));
        line_bufi--;
      }
      break;
    case '\33':
      break;
    default:
      *line_bufi = *bufi;
      line_bufi++;
      sll_push(term_wq, strndup(bufi, 1));
      //write(STDOUT_FILENO, bufi, 1);
      break;
    }
  }

  return 1;
}

int
term_parse()
{
  *line_bufi = '\0';
  char *s = strndup(line_buf, line_bufi - line_buf);
  sll_push(proto_ceh->wq, prc_msg(s));
  free(s);
  line_bufi = line_buf;

  return 0;
}

int
term_write(struct epoll_event *ev)
{
  char *buf;

  while (term_wq->head) {

    sll_pop(term_wq, &buf);

    write(STDOUT_FILENO, buf, strlen(buf));

    free(buf);
  }

  return 0;
}

void
term_printf(char *format, ...)
{
  char *buf = malloc(MSG_SIZE);

  {
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, MSG_SIZE, format, ap);
    va_end(ap);
  } /* ... */

  *line_bufi = '\0';
  snprintf(scratch, MSG_SIZE,
           "" ERASE_LINE SCROLL_DOWN FORCE_CURSORd CURSOR_UP "%s" FORCE_CURSORd "%s",
           ws.ws_row, buf, ws.ws_row, line_buf);
  sll_push(term_wq, strdup(scratch));

  free(buf);
}

int
term_setup()
{
  int err, mode;
  struct termios t;

  stdout_ev.data.ptr = NULL;

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
