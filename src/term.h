#pragma once

#define ERASE_LINE "\033[2K"

#define SCROLL_DOWN "\033D"

#define FORCE_CURSORd "\033[%d;0f"

#define CURSOR_UP "\033[1A"
#define CURSOR_DOWN "\033[1B"
#define CURSOR_FORWARD "\033[1C"
#define CURSOR_BACK "\033[1D"

#define BACKSPACE "\033[D \033[D"

#include <sys/epoll.h>
#include "sll.h"

extern sll_t *term_wq;

int
term_stdout(int epfd);

int
term_register(int epfd);

int
term_read(struct epoll_event *ev);

int
term_parse();

int
term_write(struct epoll_event *ev);

void
term_printf(char *format, ...);

int
term_setup();
