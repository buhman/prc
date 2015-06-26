#include "config.h"

#include <sys/epoll.h>

#include "buh/net.h"
#include "buh/buh.h"

#include "bus/net.h"

#include "prc/prc.h"
#include "proto.h"
#include "method.h"

event_handler *bus_eh;

int
main(int argc, char *argv[])
{
  int efd, sfd;
  event_handler *eh;

  efd = epoll_create1(EPOLL_CLOEXEC);
  if (efd < 0)
    herror("epoll_create1");

  sfd = buh_connect_unix(efd, "/run/bbus/daemon.sock", &eh);
  if (sfd < 0)
    herror("buh_connect_unix");

  eh->in.recv = prc_method_read_cb;
  bus_eh = eh;

  prc_method_map_init();

  bb_register(eh, "prccd");

  while (!buh_event_iter(efd)) {
  }

  return 0;
}
