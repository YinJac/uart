#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "epoll_server.h"
#include "uart_start.h"

extern int G_sockfd;
void setnonblocking(int sock)//将套接字设置为非阻塞
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

int accept_new_client(int epollfd, int listen_fd)  
{  
	struct sockaddr_un client_addr;  
	memset(&client_addr, 0, sizeof(client_addr));  
	socklen_t clilen = sizeof(struct sockaddr);   
	int new_sock = accept(listen_fd, (struct sockaddr *)&client_addr, &clilen);	
	struct epoll_event ev;	
	ev.data.fd = new_sock;	
	ev.events  = EPOLLIN;  
	epoll_ctl(epollfd, EPOLL_CTL_ADD, new_sock, &ev);  
	return new_sock;  
}  

//此处为代码最初设计select操作，后期改为epoll，此处弃置
void select_start()
{	
	char recv_buf[1024] = {0};
	int ret ;
	printf("epoll waiting...\n");
	fd_set rd;
	int maxfd;
	while(1){
		FD_ZERO(&rd);
		FD_SET(G_uartfd , &rd);
		maxfd = G_uartfd+1;
		ret = select(maxfd,&rd,NULL,NULL,NULL);
		switch(ret)
		{
			case -1:
					perror("select");
					break;
			case 0:
					perror("time is over!");
					break;
			default:
				if(FD_ISSET(G_uartfd,&rd))	//uart_revecedata
				{
					memset(recv_buf, 0, sizeof(recv_buf));		
					ret = read(G_uartfd,recv_buf,sizeof(recv_buf));
					printf("%s\n", recv_buf);
				}					
			}
	}
}
