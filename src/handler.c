#include "buh/hash.h"
#include "bus/net.h"
#include "prc/prc.h"

#include "handler.h"
#include "proto.h"

#include <stdio.h>

static hash_table_t command_ht = {0};

void
prc_handler_subscribe_cmd(event_handler *eh, const char *command, const char *target)
{
  prc_handler_t *handler;

  handler = malloc(sizeof (prc_handler_t));
  handler->target = strdup(target);
  handler->eh = eh;

  hash_dump(&command_ht);

  hash_put(&command_ht, command, handler);

  hash_dump(&command_ht);
}

static inline void
prc_handler_cmd_run(prc_msg_t *pmsg, bb_message *bmsg)
{
  int ret;
  hash_iter_t iter;
  prc_handler_t *handler;

  ret = hash_iter(&command_ht, pmsg->command, &iter);
  printf("%s; iter ret: %d\n", pmsg->command, ret);
  if (ret < 0)
    return;

  while ((handler = hash_next(&iter)) != NULL) {
    bmsg->target = handler->target;
    bb_sendmsg(handler->eh, bmsg);
    /* fixme: do something with sendmsg return? */
  }
}

int
prc_handler(event_handler *eh, void *buf, size_t len)
{
  prc_msg_pack_t *pack;
  prc_msg_t pmsg = {0};
  bb_message bmsg = {
    .sender = "prccd", /* fixme? */
  };

  prc_proto_parse_msg(eh, buf, &pmsg);

  pack = prc_pack_alloca(len);

  bmsg.d.len = prc_pack_msg(pack, &pmsg, buf, len);
  bmsg.d.ptr = pack;

  prc_handler_cmd_run(&pmsg, &bmsg);
  //prc_handler_plugin_run(pmsg, &bmsg);

  return 0;
}
