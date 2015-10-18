#include "buh/hash.h"
#include "bus/net.h"

#include "plugin.h"

static hash_table_t plugin_ht = {};

int
prc_plugin_load(char *name, int efd, int sfd, event_handler *eh)
{
  void *handle;

  handle = dlopen(name, RTLD_LAZY);
  if (handle == NULL) {
    fprintf(stderr, "%s\n", dlerror());
    return -1;
  }

  *(void **)(&entry_f) = dlsym(handle, "prc_plugin_entry");
  if ((error = dlerror()) == NULL) {
    err = (*entry_f)(efd, sfd, eh);
    if (err < 0) {
      fprintf(stderr, "prc_plugin_entry(): %d\n", err);
      return -1;
    }
  }

  return 0;
}

int
prc_plugin_spawn(char *name)
{
  int sfd, efd, ret;
  event_handler *eh;

  efd = epoll_create1(EPOLL_CLOEXEC);
  if (efd < 0)
    herror("epoll_create1");

  sfd = buh_connect_unix(efd, "/run/bbus/daemon.sock", &eh);
  if (sfd < 0)
    herror("buh_connect_unix");

  ret = fork();
  if (ret < 0)
    perror("fork");

  if (ret != 0) {
    close(efd);
    close(sfd);
    return ret;
  }

  return prc_plugin_load(efd, sfd, eh);
}
