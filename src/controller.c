#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/epoll.h>

#include "prc.h"
#include "controller.h"

#define MAXEVENT 10

int
controller_recvfd(int sfd)
{
  struct msghdr msg = {0};
  struct cmsghdr *cmsg;
  int fd, *fdptr;

  char buf[CMSG_SPACE(sizeof(int))];

  struct iovec ib = {0};
  char ip;

  ssize_t retsize;

  {
    ib.iov_base = &ip;
    ib.iov_len = 1;

    msg.msg_iov = &ib;
    msg.msg_iovlen = 1;
  }

  msg.msg_control = buf;
  msg.msg_controllen = sizeof(buf);

  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof(int));

  retsize = recvmsg(sfd, &msg, 0);
  fprintf(stderr, "RECVMSG: %zd\n", retsize);
  if (retsize < 0)
    herror("recvmsg()", retsize);
  if (retsize == 0)
    return 0;

  cmsg = CMSG_FIRSTHDR(&msg);

  if (cmsg->cmsg_type != SCM_RIGHTS)
    herror("unknown cmsg_type", -1);

  fdptr = (int*)CMSG_DATA(cmsg);
  memcpy(&fd, fdptr, sizeof(int));

  return fd;
}

int
controller_epoll_create(int wfd)
{
  struct epoll_event ev;
  int ret, epfd;

  epfd = epoll_create1(EPOLL_CLOEXEC);
  if (epfd < 0)
    herror("epoll_create1()", epfd);

  ev.events = EPOLLIN;
  ev.data.fd = wfd;

  ret = epoll_ctl(epfd, EPOLL_CTL_ADD, wfd, &ev);
  if (ret < 0)
    herror("epoll_ctl()", ret);

  return epfd;
}

int
controller_main(int wfd, int fds[], int *nfds)
{
  struct epoll_event evs[10], *evi;
  int epfd, ret;

  epfd = controller_epoll_create(wfd);
  if (epfd < 0)
    herror("controller_epoll_create()", epfd);

  while (true) {

    ret = epoll_wait(epfd, evs, 10, -1);
    if (ret < 0)
      herror("epoll_wait()", ret);

    for (evi = evs; evi < evs + ret; evi++) {

      fprintf(stderr, "CONTROLLER: fd: %d\n", evi->data.fd);

      fds[*nfds] = controller_recvfd(evi->data.fd);

      if (fds[*nfds] < 0) {
        perror("controller_recvfd()");
        return -1; /* HACK? */
      }
      if (fds[*nfds] == 0) {
        close(wfd);
        close(epfd);
        return -1; /* FIXME 0 */
      }

      fprintf(stderr, "CONTROLLER: recvfd: %d\n", fds[*nfds]);

      if (*nfds < 256)
        (*nfds)++;
    }
  }

  close(wfd);
  close(epfd);
  return 0;
}
