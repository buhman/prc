#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <stdbool.h>

#include <dlfcn.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "prc.h"
#include "controller.h"

static int
dlstart(const char *path, const char *symbol, int cfd, int fds[], int *nfds)
{
  int ret;
  void *handle;
  char *error;
  prc_main_t *_main;

  handle = dlopen(path, RTLD_LAZY);
  if (handle == NULL) {
    fprintf(stderr, "%s\n", dlerror());
    return -1;
  }

  *(void **)(&_main) = dlsym(handle, symbol);
  if ((error = dlerror()) != NULL) {
    fprintf(stderr, "%s\n", error);
    dlclose(handle);
    return -1;
  }

  ret = (*_main)(cfd, fds, nfds);

  dlclose(handle);

  return ret;
}

int
main(int argc, char *argv[])
{
  int ret, sfds[2], fds[256], nfds = 0;

  pid_t pid;

  while (true) {

    ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sfds);
    if (ret < 0)
      herror("socketpair()", ret);

    fprintf(stderr, "SOCKPAIR; c: %d w: %d\n", sfds[0], sfds[1]);

    pid = fork();
    if (pid < 0)
      herror("fork()", pid);

    if (pid == 0) {
      close(sfds[0]);
      return dlstart("libworker.so", "worker_main", sfds[1], fds, &nfds);
    }

    close(sfds[1]);
    ret = dlstart("libcontroller.so", "controller_main", sfds[0], fds, &nfds);
    if (ret < 0)
      return ret;

    close(sfds[0]);

  } /* .. */

  return 0;
}
