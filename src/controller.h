#pragma once

#include "prc.h"

int
controller_recvfd(int sfd);

int
controller_epoll_create(int wfd);

prc_main_t controller_main;
