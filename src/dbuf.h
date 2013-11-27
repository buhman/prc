#pragma once

#define READ_BUFSIZE 512

typedef ssize_t (*dbuf_read_fptr)(int, void*, size_t);

enum dbuf_readf {
  DBUF_RECV,
  DBUF_READ,
};

typedef enum dbuf_readf dbuf_readf_t;

static ssize_t
_recv(int fd,
      void *buf,
      size_t count);

ssize_t
dbuf_read(int fd,
	  char **rbuf,
	  dbuf_readf_t readf);
