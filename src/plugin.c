#include <string.h>
#include <dlfcn.h>
#include <stdio.h>

#include "prc.h"

#include "proto.h"
#include "plugin.h"

static handler_ht_t *plugin_head;

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

  *(void **)(&reg_fp) = dlsym(handle, "register");
  if ((error = dlerror()) != NULL) {
    fprintf(stderr, "%s\n", error);
    return -1;
  }

  return (*reg_fp)(&plugin_head);
}

int
plugin_handler(sll_t *wq, char *target, char **sp)
{
  char *cmd;
  handler_ht_t *item;

  cmd = strtok_r(NULL, " ", sp);
  if (cmd != NULL) {
    HASH_FIND_STR(plugin_head, cmd, item);
    if (item)
      (item->func)(wq, target, sp);
    else
      fprintf(stderr, "[PLUGIN] bad cmd [%s]\n", cmd);
  }
  else {
    fprintf(stderr, "[PLUGIN] no cmd\n");
  }
}

static void
plugin_cmd(sll_t *wq, char *target, char **sp)
{
  char *action, *pred;
  int err;

  action = strtok_r(NULL, " ", sp);
  if (action == NULL) {
    fprintf(stderr, "[PLUGIN] no action\n");
    return;
  }

  pred = strtok_r(NULL, " ", sp);
  if (pred == NULL) {
    fprintf(stderr, "[PLUGIN] no pred\n");
    return;
  }

  if (strcmp(action, "load") == 0) {
    err = plugin_load(pred);
    if (err < 0) {
      sll_push(wq, prc_msg("PRIVMSG", target, "[failure]", NULL));
    }
    else {
      sll_push(wq, prc_msg("PRIVMSG", target, "[success]", NULL));
    }
  }
}

void
plugin_init(handler_ht_t **admin_head)
{
  handler_ht_t *item;
  item = malloc(sizeof(handler_ht_t));
  item->func = plugin_cmd;

  HASH_ADD_KEYPTR(hh, *admin_head, "plugin", strlen("plugin"), item);
}
