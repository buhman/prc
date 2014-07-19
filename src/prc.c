#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <stdbool.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "prc.h"
#include "event.h"
#include "controller.h"
#include "worker.h"

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
      return worker_main(sfds[1], fds, nfds);
    }

    close(sfds[1]);
    ret = controller_main(sfds[0], fds, &nfds);
    if (ret < 0)
      return ret;

    close(sfds[0]);

  } /* .. */

  return 0;
}
