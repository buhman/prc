#include <sys/epoll.h>

#include "buh/buh.h"
#include "buh/net.h"

#include "bus/bus.h"
#include "bus/dump.h"
#include "bus/net.h"

#include "prc/prc.h"

static char cname[] = "prcc";
static event_handler *bus_eh;

static int
read_cb(event_handler *eh, void *buf, size_t len)
{
  int ret;
  bb_message msg = {0};
  prc_msg_t pmsg = {0};

  bb_parse_message(buf, len, &msg);

  ret = prc_unpack_msg(&msg, &pmsg);
  if (ret < 0)
    return 0;

  printf("%s: %s\n", pmsg.command, pmsg.params.trailing);

  return 0;
}

static int
stdin_cb(event_handler *eh)
{
  char buf[1024], *ptr = buf;
  ssize_t len;
  bb_message msg = {
    .target = "prccd",
    .sender = cname,
  };

  len = read(eh->sfd, buf, 1023);
  if (len < 0)
    herror("read");

  ptr = memchr(ptr, ' ', len);
  if (!ptr) {
    fprintf(stderr, "[stdin] invalid\n");
    return 0;
  }

  *ptr++ = '\0';
  *(buf + --len) = '\0';

  msg.method = buf;
  msg.data = ptr;

  bb_sendmsg(bus_eh, &msg);

  return 0;
}

int
main(int argc, char *argv[])
{
  int efd, sfd, ret;
  event_handler *eh;

  efd = epoll_create1(EPOLL_CLOEXEC);
  if (efd < 0)
    herror("epoll_create1");

  sfd = buh_connect_unix(efd, "/run/bbus/daemon.sock", &eh);
  if (sfd < 0)
    herror("buh_connect_unix");
  eh->in.recv = read_cb;

  bb_register(eh, cname);

  bus_eh = eh;

  ret = buh_event_add(efd, 0, EPOLLIN, &eh);
  if (ret < 0)
    herror("buh_event_add");
  eh->in.handler = stdin_cb;

  while (!buh_event_iter(efd)) {
  }

  return 0;
}
