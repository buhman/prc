#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dll.h"
#include "prc.h"
#include "cfg.h"
#include "network.h"
#include "proto.h"
#include "controller.h"

int
network_join_fds(int epfd, int fds[], int nfds)
{
  int *fdi, ret;
  struct sockaddr_in6 sa;
  socklen_t len = sizeof(sa);
  char *node;

  for (fdi = fds; fdi - fds < nfds; fdi++) {

    ret = getpeername(*fdi, (struct sockaddr*)&sa, &len);
    if (ret < 0)
      herror("getpeername()", ret);

    //raise(SIGINT);

    fprintf(stderr, "NET: node: %s\n", "irc.frenode.net");

    ret = proto_register(epfd, *fdi, "freenode", NULL, NULL);
    if (ret < 0)
      return ret;
  }

  return 0;
}

int
network_join_cfg(int epfd, int cfd, dll_t *networks)
{
  cfg_net_t *net;
  dll_link_t *li;
  dll_t *wq;

  int ret, sfd;

  li = networks->head;

  while (li) {

    net = li->buf;

    if (!net->nick) {
      fprintf(stderr, "no nick in %s\n", net->node);
      return -1;
    }

    fprintf(stderr, "NET: node: %s ; service: %s\n", net->node, net->service);

    sfd = proto_connect(net->node, net->service);
    if (sfd < 0) {
      perror("proto_connect()");
      return sfd;
    }

    ret = proto_register(epfd, sfd, net->node, net, &wq);
    if (ret < 0) {
      close(sfd);
      perror("proto_register()");
      li = li->next;
      continue;
    }

    fprintf(stderr, "controller_sendfd\n");
    ret = controller_sendfd(cfd, sfd);
    if (ret < 0)
      perror("controller_sendfd()");

    fprintf(stderr, "NET: write-queue: %p\n", wq);

    if (net->password)
      dll_enq(wq, prc_msg("CAP REQ :sasl", NULL));

    dll_enq(wq, prc_msg3("NICK %s\r\n", net->nick));

    dll_enq(wq, prc_msg3("USER %s foo bar :buhman's minion\r\n", net->username));

    li = li->next;
  }

  return 0;
}
