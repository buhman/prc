#pragma once

#define TERM_ERASE_LINE "\033[K"

#define TERM_CURSOR_UP "\033[1A"
#define TERM_CURSOR_DOWN "\033[1B"
#define TERM_CURSOR_FORWARD "\033[1C"
#define TERM_CURSOR_BACK "\033[1D"

#include "sll.h"

extern sll_t *term_wq;

int
term_stdout(int epfd);

int
term_register(int epfd);

int
term_read(struct epoll_event *ev);

int
term_write(struct epoll_event *ev);

void
term_print(char *buf);

int
term_setup();
