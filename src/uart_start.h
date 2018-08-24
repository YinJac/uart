#ifndef __UART_START_H__
#define __UART_START_H__
#include "epoll_server.h"
#include "thread_pool.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h> 
#include <pthread.h>
#include <sys/wait.h>
#include <limits.h>
#include "linklist.h"

//typedef struct{
//	int uart_fd;
//	pthread_mutex_t fd_mtx;
//}UART;

int ready_send_data(enum port send_to, const char *data, int len);

extern int G_uartfd;

#endif
