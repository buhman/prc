#include <string.h>

#include "prc/prc.h"

int
prc_pack_msg(prc_msg_pack_t *pack, prc_msg_t *pmsg, char *buf, size_t len)
{
  char **field;
  short *pfield;

  for (field = (char**)pmsg, pfield = pack->fields;
       field < (char**)pmsg + PRC_MSG_FIELDS;
       ++field, ++pfield)
    *pfield = *field - buf;

  pack->buflen = len;

  /* fixme: double-memcpy is ehhh */
  memcpy(pack->buf, buf, len);

  return sizeof (prc_msg_pack_t) + len;
}

int
prc_unpack_msg(bb_message *bmsg, prc_msg_t *pmsg)
{
  prc_msg_pack_t *pack;
  char **field;
  short *pfield;

  pack = bmsg->d.ptr;

  if (bmsg->d.len < sizeof (prc_msg_pack_t) ||
      sizeof (prc_msg_pack_t) + pack->buflen != bmsg->d.len)
    return -1;

  for (field = (char**)pmsg, pfield = pack->fields;
       field < (char**)pmsg + PRC_MSG_FIELDS;
       ++field, ++pfield)
    *field = pack->buf + *pfield;

  return 0;
}
