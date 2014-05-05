#include <stdio.h>

int
printbuf(char *buf, int len)
{
  char *bufi = buf;

  while (bufi < buf + len) {
    if (*bufi > 31 && *bufi < 127)
      fprintf(stderr, "%c", *bufi);
    else
      fprintf(stderr, "%#02x", *bufi);

    bufi++;
  }

  fprintf(stderr, "\n");
  fflush(stderr);

  return 0;
}
