#include <stdio.h>

#include "buf.h"

int
printbuf(void *buf, int len)
{
  char *bufi = buf;

  while (bufi < (char*)buf + len) {
    if (*bufi > 32 && *bufi < 127)
      fprintf(stderr, "%c ", (unsigned char)*bufi);
    else
      fprintf(stderr, "%02x ", (unsigned char)*bufi);

    bufi++;
  }

  fprintf(stderr, "\n");
  fflush(stderr);

  return 0;
}
