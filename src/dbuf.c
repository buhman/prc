#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>

#include "dbuf.h"

static dbuf_read_fptr readf_map[2] = {
  [DBUF_RECV] = _recv,
  [DBUF_READ] = read,
};

static ssize_t
_recv(int fd,
      void *buf,
      size_t count)
{
  return recv(fd, buf, count, 0);
}

uint64_t
dbuf_pack(int fd,
	  int readf)
{
  return fd | (uint64_t)readf << 32;
}

ssize_t
dbuf_readp(uint64_t p,
	   char **rbuf)
{
  int fd = (int)p;
  int readf = (int)(p >> 32);

  return dbuf_read(fd, readf, rbuf);
}

ssize_t
dbuf_read(int fd,
	  dbuf_readf_t readf,
	  char **rbuf)
{
  char *buf, *ibuf;
  size_t count;
  ssize_t size;

  {
    count = READ_BUFSIZE;

    buf = malloc(count);
    ibuf = buf;

    while(true) {

      size = readf_map[readf](fd, ibuf, count - (ibuf - buf));
      if (size < 0) {
	perror("read()");
	return size;
      }

      assert(size <= count - (ibuf - buf));

      if (size == count - (ibuf - buf)) {

	buf = realloc(buf, count * 2);
	ibuf = buf + count;
	count *= 2;

	continue;
      }

      *(ibuf + size) = '\0';

      *rbuf = buf;
      return size + count / 2;
    }
  }
}
