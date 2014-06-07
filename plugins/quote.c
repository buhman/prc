#include <stdio.h>

#include "prc.h"
#include "row.h"
#include "tag.h"
#include "util.h"

prc_plugin_ctor_t prc_ctor;
prc_plugin_dtor_t prc_dtor;

static prc_plugin_cmd_t cite_cmd;
static prc_plugin_cmd_t quote_cmd;

prc_plugin_sym_t prc_sym[] = {
  {"cite", cite_cmd},
  {"quote", quote_cmd},
};

bdb_t *bdb;
int tag_default;
int tag_quotes;

static int
add_citation(dll_t *wq, char *target, uint64_t row)
{
  uint64_t next = (uint64_t)-1, buf_o;

  while ((buf_o = bdb_row_iter(tag_quotes, &next, bdb)) != (uint64_t)-1) {
    if (buf_o == row) {
      dll_enq(wq, prc_msg2("PRIVMSG", target, "[duplicate]"));
      return -1;
    }
  }

  dll_enq(wq, prc_msg2("PRIVMSG", target, "[%lld]", row));

  return 0;
}

static void
cite_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  uint64_t next = (uint64_t)-1, msg_o;
  prc_db_msg_t *msg;
  char *qprefix;
  int ret;

  if (!args)
    return;

  while ((msg_o = bdb_row_iter(tag_default, &next, bdb)) != (uint64_t)-1) {

    msg = R(msg_o, bdb);

    if (msg->prefix != (uint64_t)-1) {
      qprefix = R(msg->prefix, bdb);

      qprefix = prc_prefix_parse(qprefix, NICK);

      if (strcmp(args, qprefix) == 0) {
        D(&qprefix, bdb);
        D(&msg, bdb);

        ret = add_citation(wq, target, msg_o);
        if (ret >= 0)
          bdb_row_push(tag_quotes, (void*)msg_o, (uint64_t)-1, bdb);

        return;
      }

      D(&qprefix, bdb);
    }
    D(&msg, bdb);
  }

  dll_enq(wq, prc_msg2("PRIVMSG", target, "[not found]"));
}

static void
quote_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  uint64_t next = (uint64_t)-1, msg_o;
  prc_db_msg_t *msg;
  char *qprefix;
  char *params;

  if (!args)
    return;

  while ((msg_o = bdb_row_iter(tag_quotes, &next, bdb)) != (uint64_t)-1) {

    msg = R(msg_o, bdb);
    qprefix = R(msg->prefix, bdb);

    qprefix = prc_prefix_parse(qprefix, NICK);
    if (strcmp(args, qprefix) == 0) {
      params = R(msg->params, bdb);
      dll_enq(wq, prc_msg2("PRIVMSG", target, "%s: %s", qprefix, params));
      D(&params, bdb);
      D(&qprefix, bdb);
      D(&msg, bdb);
      return;
    }

    D(&qprefix, bdb);
    D(&msg, bdb);
  }

  dll_enq(wq, prc_msg2("PRIVMSG", target, "[not found]"));
}

int
prc_ctor(bdb_t *b)
{
  bdb = b;

  tag_default = bdb_tag_find("default", bdb);

  tag_quotes = bdb_tag_find("quotes", bdb);
  if (tag_quotes < 0)
    tag_quotes = bdb_tag_add("quotes", bdb);

  return 0;
}
