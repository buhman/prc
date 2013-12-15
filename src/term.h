#pragma once

#define TERM_ERASE_LINE "\033[K"

#define TERM_CURSOR_UP "\033[1A"
#define TERM_CURSOR_DOWN "\033[1B"
#define TERM_CURSOR_FORWARD "\033[1C"
#define TERM_CURSOR_BACK "\033[1D"

#include "sll.h"

void
term_read(int fd, sll_t *wq);

void
term_write(int fd, sll_t *wq);

int
term_setup();
