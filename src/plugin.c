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

  *(void **)(&reg_fp) = dlsym(handle, "prc_reg");
  if ((error = dlerror()) != NULL) {
    fprintf(stderr, "%s\n", error);
    return -1;
  }

  return (*reg_fp)(&plugin_head);
}

static void
plugin_cmd(sll_t *wq, char *target, char *buf)
{
  char *action, *pred;
  int err;

  action = strtok_r(NULL, " ", &buf);
  if (action == NULL) {
    fprintf(stderr, "[PLUGIN] no action\n");
    return;
  }

  printf("action: [%s]\n", action);

  pred = strtok_r(NULL, " ", &buf);
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
