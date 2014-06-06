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
#include "buf.h"
#include "prc.h"
#include "term.h"
#include "proto.h"

static struct winsize ws;
dll_t *term_wq;
static struct epoll_event stdout_ev;

static char *line_buf, *line_bufi;

static char scratch[MSG_SIZE * 2];

static char *ttarget = NULL;

int
term_stdout(int epfd)
{
  int err;

  if (term_wq->head != NULL) {
    err = event_add(epfd, STDOUT_FILENO, EPOLLOUT, NULL,
                    term_write, NULL, NULL, &stdout_ev);
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

  term_wq = calloc(1, sizeof(dll_t));

  err = term_setup();
  if (err < 0) {
    perror("term_setup()");
    return err;
  }

  err = event_add(epfd, STDIN_FILENO, EPOLLIN, term_read, NULL, term_wq, NULL, NULL);
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
      dll_enq(term_wq, strdup(scratch));

      break;
    case '\b':
    case 127:
      if (line_bufi > line_buf) {
        dll_enq(term_wq, strdup(BACKSPACE));
        line_bufi--;
      }
      break;
    case '\33':
      break;
    default:
      *line_bufi = *bufi;
      line_bufi++;
      dll_enq(term_wq, strndup(bufi, 1));
      break;
    }
  }

  return 1;
}

static int
term_me()
{
  char *tok;

  if (ttarget) {
    tok = strchr(line_buf, ' ');
    if (tok)
      dll_enq(proto.ceh->wq, prc_msg("PRIVMSG", ttarget,
                                     ":\001ACTION", tok + 1, "\001", NULL));
  }
  else
    return -1;

  return 0;
}

static int
term_msg()
{
  if (ttarget)
    dll_enq(proto.ceh->wq, prc_msg("PRIVMSG", ttarget, ":", line_buf, NULL));
  else
    return -1;

  return 0;
}

static int
term_target()
{
  char *tok;

  tok = strchr(line_buf, ' ');
  if (tok) {
    if (ttarget)
      free(ttarget);
    ttarget = strdup(tok + 1);
    term_printf("target: [%s]", ttarget);
  }

  return 0;
}

static int
term_network()
{
  char *tok;

  tok = strchr(line_buf, ' ');
  if (tok) {
    proto_set_node(tok + 1);
    term_printf("network: [%s]", proto.node);
  }

  return 0;
}

int
term_parse()
{
  *line_bufi = '\0';

  if (memcmp(line_buf, "/act", 4) == 0)
    term_me();
  else if (memcmp(line_buf, "/tgt", 4) == 0)
    term_target();
  else if (memcmp(line_buf, "/net", 4) == 0)
    term_network();
  else if (*line_buf == ':')
    dll_enq(proto.ceh->wq, prc_msg(line_buf + 1, NULL));
  else
    term_msg();

  line_bufi = line_buf;

  return 0;
}

int
term_write(struct epoll_event *ev)
{
  char *buf;

  while ((buf = dll_pop(term_wq)) != NULL) {

    write(STDOUT_FILENO, buf, strlen(buf));

    free(buf);
  }

  return 0;
}

void
term_printf(const char *format, ...)
{
  char *buf = malloc(MSG_SIZE);

  {
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, MSG_SIZE - (2 + strlen("" ERASE_LINE SCROLL_DOWN FORCE_CURSORd
                                          CURSOR_UP FORCE_CURSORd)), format, ap);
    va_end(ap);
  } /* ... */

  *line_bufi = '\0';
  snprintf(scratch, MSG_SIZE - 1,
           "" ERASE_LINE SCROLL_DOWN FORCE_CURSORd CURSOR_UP "%s" FORCE_CURSORd "%s",
           ws.ws_row, buf, ws.ws_row, line_buf);
  dll_enq(term_wq, strdup(scratch));

  free(buf);
}

int
term_setup(void)
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

  {
    line_buf = malloc(MSG_SIZE);
    line_bufi = line_buf;
  }

  return 0;
}
