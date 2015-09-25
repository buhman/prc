#include <stdio.h>

#include "buh/hash.h"
#include "buh/net.h"
#include "bus/bus.h"
#include "bus/net.h"

#include "method.h"
#include "proto.h"
#include "handler.h"

static method_map_t methods[] = {
  {"connect", &prc_method_connect},
  {"subscribe.cmd", &prc_method_subscribe_cmd},
  {NULL}
};

static hash_table_t method_ht = {0};

void
prc_method_map_init(void)
{
  method_map_t *map;

  for (map = methods; map->name; map++)
    hash_put(&method_ht, map->name, map->fptr);
}

int
prc_method_connect(event_handler *eh, bb_message *msg)
{
  char *ptr;

  ptr = strchr(msg->data, ':');
  if (!ptr) {
    bb_error(eh, msg, "[connect] invalid proto '%s'", msg->data);
    return -1;
  }

  *ptr++ = '\0';

  // fixme: handle duplicate connection?

  return prc_proto_connect(eh->efd, msg->data, ptr);
  // fixme: reply with connect failure?
}

int
prc_method_subscribe_cmd(event_handler *eh, bb_message *msg)
{
  fprintf(stderr, "subscribe\n");
  prc_handler_subscribe_cmd(eh, msg->data, msg->sender);

  return 0;
}

int
prc_method_read_cb(event_handler *eh, void *buf, size_t len)
{
  bb_message msg = {0};
  method_fptr_t fptr;

  bb_parse_message(buf, len, &msg);

  fptr = hash_get(&method_ht, msg.method);
  if (fptr)
    (*fptr)(eh, &msg);
  else
    bb_error(eh, &msg, "unknown method '%s'", msg.method);

  return 0;
}
