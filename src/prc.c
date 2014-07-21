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
#include "worker.h"

static int
start_worker(int cfd, int fds[], int nfds)
{
  int ret;
  void *whandle;
  char *error;
  worker_main_t *worker;

  /* HACK: RTLD_GLOBAL so plugins can access libprc symbols--the build
     system is broken and plugins aren't properly linking libprc */

  whandle = dlopen("libworker.so", RTLD_NOW | RTLD_GLOBAL);
  if (whandle == NULL) {
    fprintf(stderr, "%s\n", dlerror());
    return -1;
  }

  *(void **)(&worker) = dlsym(whandle, "worker_main");
  if ((error = dlerror()) != NULL) {
    fprintf(stderr, "%s\n", error);
    dlclose(whandle);
    return -1;
  }

  ret = (*worker)(cfd, fds, nfds);
  /* TODO: handle this */

  dlclose(whandle);

  return -1;
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

    pid = fork();
    if (pid < 0)
      herror("fork()", pid);

    fprintf(stderr, "SOCKPAIR; c: %d w: %d\n\n", sfds[0], sfds[1]);

    if (pid == 0) {
      close(sfds[0]);
      return start_worker(sfds[1], fds, nfds);
    }

    close(sfds[1]);
    ret = controller_main(sfds[0], fds, &nfds);
    if (ret < 0)
      return ret;

    close(sfds[0]);

  } /* .. */

  return 0;
}
