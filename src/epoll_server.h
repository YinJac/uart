#ifndef _EPOLL_SERVER_H__
#define _EPOLL_SERVER_H__

#include "debug.h"

typedef struct{
	int epollfd;
	int *listenedfd;
	int listenfd_num;
	int listenfd_max;
}epoll_listen;


void epoll_wait_start(int epollfd);
void epoll_add_fd(int epollfd, int newfd);
int epoll_init(int epollfd);

#endif
