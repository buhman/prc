#include <string.h>
#include <dlfcn.h>
#include <stdio.h>

#include "prc.h"

#include "proto.h"
#include "plugin.h"
#include "term.h"

static prc_plugin_ht_t *plugin_head;

static int
plugin_load(char *name)
{
  reg_fp_t *reg_fp;
  void *handle;
  char *error;

  handle = dlopen(name, RTLD_LAZY);
  if (handle == NULL) {
    fprintf(stderr, "%s\n", dlerror());
    return -1;
  }

  dlerror();

  *(void **)(&reg_fp) = dlsym(handle, "prc_reg");
  if ((error = dlerror()) != NULL) {
    fprintf(stderr, "%s\n", error);
    return -1;
  }

  return (*reg_fp)(&plugin_head);
}

void
plugin_lookup(sll_t *wq, char *prefix, char *target,
              char *cmd, char *args) {

  prc_plugin_ht_t *item;

  HASH_FIND_STR(plugin_head, cmd, item);

  if (item != NULL)
    (item->func)(wq, prefix, target, args);
}

void
plugin_cmd(sll_t *wq, char *prefix, char *target, char *buf)
{
  int err;
  char *cmd, *tok, *args;

  tok = strchr(buf, ' ');
  if (!tok) {
    term_printf("cmd(): no tok");
    return;
  }

  *tok = '\0';
  args = tok + 1;
  cmd = buf;

  term_printf("cmd(): [%s] [%s]", cmd, args);

  if (strcmp(cmd, "load") == 0) {
    err = plugin_load(args);
    if (err < 0)
      sll_push(wq, prc_msg("PRIVMSG", target, "[failure]", NULL));
    else
      sll_push(wq, prc_msg("PRIVMSG", target, "[success]", NULL));
  }
}
