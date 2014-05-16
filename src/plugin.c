#include <string.h>
#include <dlfcn.h>
#include <stdio.h>
#include <assert.h>

#include "prc.h"

#include "proto.h"
#include "plugin.h"
#include "term.h"

static plugin_handle_ht_t *handle_head;
static plugin_ht_t *plugin_head;

static int
plugin_register(char *key, prc_plugin_cmd_t *func)
{
  plugin_ht_t *item, *_item;

  item = malloc(sizeof(plugin_ht_t));
  item->func = func;
  _item = item;

  HASH_ADD_KEYPTR(hh, plugin_head, key, strlen(key), item);

  HASH_FIND(hh, plugin_head, key, strlen(key), item);

  if (!item) {
    free(_item);
    return -1;
  }

  return 0;
}

static int
plugin_deregister(char *key)
{
  plugin_ht_t *item;

  HASH_FIND(hh, plugin_head, key, strlen(key), item);
  if (!item)
    return -1;

  HASH_DELETE(hh, plugin_head, item);

  free(item);

  return 0;
}

static int
plugin_load(char *name)
{
  prc_plugin_sym_t *reg_sym, *reg_symi;
  void *handle;
  char *error;
  int err;

  {
    plugin_handle_ht_t *item;

    HASH_FIND(hh, handle_head, name, strlen(name), item);
    if (item) {
      fprintf(stderr, "plugin [%s] already loaded\n", name);
      return -1;
    }
  }

  handle = dlopen(name, RTLD_LAZY);
  if (handle == NULL) {
    fprintf(stderr, "%s\n", dlerror());
    return -1;
  }

  {
    prc_plugin_init_t *reg_ctor;

    *(void **)(&reg_ctor) = dlsym(handle, "prc_ctor");
    if ((error = dlerror()) == NULL) {
      err = (*reg_ctor)();
      if (err < 0) {
        fprintf(stderr, "prc_ctor(): %d\n", err);
        return -1;
      }
    }
  } /* ... */

  {
    dlerror();

    reg_sym = dlsym(handle, "prc_sym");
    if ((error = dlerror()) != NULL) {
      fprintf(stderr, "%s\n", error);
      dlclose(handle);
      return -1;
    }

    for (reg_symi = reg_sym; (*reg_symi).key != NULL; reg_symi++) {

      err = plugin_register((*reg_symi).key, (*reg_symi).func);

      assert(!err);
    }
  } /* ... */

  {
    plugin_handle_ht_t *item;

    item = malloc(sizeof(plugin_handle_ht_t));
    item->handle = handle;
    item->sym = reg_sym;

    HASH_ADD_KEYPTR(hh, handle_head, strdup(name), strlen(name), item);
  }

  return 0;
}

static int
plugin_unload(char *name)
{
  prc_plugin_sym_t *reg_symi;
  plugin_handle_ht_t *item;
  int err;
  char *error;

  HASH_FIND_STR(handle_head, name, item);
  if (!item) {
    fprintf(stderr, "plugin [%s] not found\n", name);
    return -1;
  }

  {
    prc_plugin_init_t *reg_dtor;

    *(void **)(&reg_dtor) = dlsym(item->handle, "prc_dtor");
    if ((error = dlerror()) == NULL) {
      err = (*reg_dtor)();
      if (err < 0) {
        fprintf(stderr, "prc_dtor(): %d\n", err);
        return -1;
      }
    }
  } /* ... */

  {
    for (reg_symi = item->sym; (*reg_symi).key != NULL; reg_symi++) {

      err = plugin_deregister((*reg_symi).key);

      assert(!err);
    }
  } /* ... */

  {
    void *handle;
    handle = item->handle;
    HASH_DELETE(hh, handle_head, item);

    free(item);

    err = dlclose(handle);
    if (err < 0) {
      fprintf(stderr, "%s\n", dlerror());
      return -1;
    }
  } /* ... */

  return 0;
}

void
plugin_lookup(dll_t *wq, char *prefix, char *target,
              char *cmd, char *args) {

  plugin_ht_t *item;

  HASH_FIND(hh, plugin_head, cmd, strlen(cmd), item);

  if (item == NULL)
    return;

  (item->func)(wq, prefix, target, args);
}

void
plugin_cmd(dll_t *wq, char *prefix, char *target, char *buf)
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
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[failure]"));
  else
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[success]"));
}
