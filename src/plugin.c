#include <string.h>
#include <dlfcn.h>
#include <stdio.h>

#include "prc.h"

#include "proto.h"
#include "plugin.h"
#include "term.h"

static plugin_handle_ht_t *handle_head;
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
    dlclose(handle);
    return -1;
  }

  {
    plugin_handle_ht_t *item;
    item = malloc(sizeof(plugin_handle_ht_t));
    item->handle = handle;

    HASH_ADD_KEYPTR(hh, handle_head, strdup(name), strlen(name), item);
  }

  return (*reg_fp)(&plugin_head);
}

static int
plugin_unload(char *name)
{
  int err;
  char *error;
  plugin_handle_ht_t *item;
  reg_fp_t *reg_fp;

  HASH_FIND_STR(handle_head, name, item);
  if (!item) {
    fprintf(stderr, "plugin [%s] not found\n", name);
    return -1;
  }

  *(void **)(&reg_fp) = dlsym(item->handle, "prc_dereg");
  if ((error = dlerror()) != NULL) {
    fprintf(stderr, "%s\n", error);
    return -1;
  }

  if ((*reg_fp)(&plugin_head) < 0)
    return -1;

  err = dlclose(item->handle);
  if (err < 0) {
    fprintf(stderr, "%s\n", dlerror());
    return -1;
  }

  HASH_DELETE(hh, handle_head, item);

  return 0;
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

  if (strcmp(cmd, "load") == 0)
    err = plugin_load(args);
  else if (strcmp(cmd, "unload") == 0)
    err = plugin_unload(args);
  else if (strcmp(cmd, "reload") == 0) {
    err = plugin_unload(args);
    if (err >= 0)
      err = plugin_load(args);
  }
  else
    return;

  if (err < 0)
    sll_push(wq, prc_msg("PRIVMSG", target, "[failure]", NULL));
  else
    sll_push(wq, prc_msg("PRIVMSG", target, "[success]", NULL));
}
