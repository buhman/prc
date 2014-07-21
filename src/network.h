#pragma once

int
network_join_fds(int epfd, int fds[], int nfds);

int
network_join_cfg(int epfd, int cfd, dll_t *networks);
