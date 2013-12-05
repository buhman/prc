#pragma once

#include <stdint.h>

#define READ_BUFSIZE 512

typedef ssize_t (*dbuf_read_fptr)(int, void*, size_t);

enum dbuf_readf {
  DBUF_RECV,
  DBUF_READ,
};

typedef enum dbuf_readf dbuf_readf_t;

uint64_t
dbuf_pack(int fd,
          int readf);

ssize_t
dbuf_readp(uint64_t p,
           char **rbuf);

ssize_t
dbuf_read(int fd,
          dbuf_readf_t readf,
          char **rbuf);
