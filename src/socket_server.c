#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket_server.h"

/*
函数名：init_socket_server
功能说明：
	创建socket进程间通信服务器
*/
int init_socket_server(const char *sock_file_name, int *sockfd)
{  
	struct sockaddr_un server_addr;  
	remove(sock_file_name);
	memset(&server_addr, 0, sizeof(&server_addr));	
	server_addr.sun_family = AF_UNIX;  
	strncpy(server_addr.sun_path, sock_file_name, sizeof(server_addr.sun_path) - 1);
  
	*sockfd = socket(AF_UNIX, SOCK_STREAM, 0);  
	if(*sockfd == -1)  
	{  
		DEBUG_ERR("socket create failed");  
		return -1;
	}  
	  
	if(bind(*sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)	
	{  
		DEBUG_ERR("socket bind failed");  
		return -1;
	}  
	  
	if(listen(*sockfd, 5) == -1)	
	{  
		DEBUG_ERR("socket listen failed");  
		return -1;
	}  
	printf("socket fd: %d\n", *sockfd);
}


