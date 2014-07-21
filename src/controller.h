#pragma once

int
controller_recvfd(int sfd);

int
controller_epoll_create(int wfd);

int
controller_main(int wfd, int fds[], int *nfds);
