#include "config.h"

#include <string.h>

#include "buh/buh.h"
#include "buh/net.h"

#include "proto.h"
#include "handler.h"
#include "prc/prc.h"

int
prc_proto_connect(int efd, const char *node, const char *service)
{
  int ret;
  event_handler *eh;
  prc_proto_ctx_t *ctx;

  ret = buh_connect_inet(efd, node, service, &eh);
  if (ret < 0)
    return -1;

  eh->in.recv = prc_proto_read_cb;
  eh->in.close = prc_proto_close_cb;

  ctx = eh->ptr = calloc(1, sizeof (prc_proto_ctx_t));

  ctx->proto.node = strdup(node);
  ctx->proto.service = strdup(service);

  return 0;
}

int
prc_proto_close_cb(event_handler *eh)
{
  int ret;
  event_handler *oeh;
  prc_proto_ctx_t *ctx;

  // reconnect
  ctx = eh->ptr;

  ret = buh_connect_inet(eh->efd, ctx->proto.node, ctx->proto.service, &oeh);
  if (ret < 0) {
    free(eh->ptr);
    return -1;
  }

  oeh->ptr = ctx;

  oeh->in.recv = prc_proto_read_cb;
  oeh->in.close = prc_proto_close_cb;

  //free(eh->ptr);

  return 0;
}

int
prc_proto_read_cb(event_handler *eh, void *buf, size_t len)
{
  char *ptr, *ibuf;
  prc_proto_ctx_t *ctx;
  size_t blen;

  ctx = eh->ptr;

  while (len) {
    ptr = memmem(buf, len, "\r\n", 2);
    if (!ptr)
      break;

    blen = ptr - (char*)buf;

    if (ctx->parse.len + blen > 512)
      goto invalid;

    len -= blen + 2;

    if (ctx->parse.len) {
      ibuf = memcpy(ctx->parse.buf + ctx->parse.len, buf, blen);
      blen += ctx->parse.len;
      ctx->parse.len = 0;
    }
    else
      ibuf = buf;

    *(ibuf + blen) = '\0';
    prc_handler(eh, ibuf, blen + 1);

    buf = ptr + 2;
  }

  if (!len)
    return 0;

  if (ctx->parse.len + len > 512)
    goto invalid;

  memcpy(ctx->parse.buf, buf, len);
  ctx->parse.len += len;
  return 0;

 invalid:
  fprintf(stderr, "[proto] invalid protocol message length > %ld\n", len);
  ctx->parse.len = 0;
  return 0;
}

/*
message    =  [ ":" prefix " " ] command [ params ] "\r\n" .
prefix     =  servername | ( nickname [ [ "!" user ] "@" host ] ) .
command    =  letter {letter} | digit digit digit .
params     =  { " " middle } [ " " [ ":" ] trailing ] .
*/

static inline void *
prc_proto_parse_prefix(prc_msg_t *msg, char *prefix)
{
  char *ptr, tok;

  msg->prefix = prefix;

  while((ptr = strpbrk(prefix, "!@ ")) != NULL) {

    prefix = ptr + 1;
    tok = *ptr;
    *ptr = '\0';

    if (tok == '!')
      msg->user = prefix;
    if (tok == '@')
      msg->host = prefix;
    if (tok == ' ')
      return prefix;
  }

  return NULL;
}

static inline void
prc_proto_parse_params(prc_msg_t *msg, char *buf)
{
  msg->params.middle = buf;

  strsep(&buf, ":");
  if (buf) {
    msg->params.trailing = buf;

    buf -= 2;
    if (*buf == ' ')
      *buf = '\0';
  }
}

int
prc_proto_parse_msg(event_handler *eh, char *buf, prc_msg_t *msg)
{
  //fprintf(stderr, "got: '%s'\n", buf);

  if (*buf == ':')
    buf = prc_proto_parse_prefix(msg, buf + 1);

  if (!buf) // command is not optional
    return -1;

  msg->command = buf;

  strsep(&buf, " ");
  if (buf)
    prc_proto_parse_params(msg, buf);

  return 0;
}
