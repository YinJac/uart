#include "uart_start.h"
#include "uart_protocol.h"
#include <sys/epoll.h>
#include <sys/un.h>
#include <sys/socket.h>

#define NEED_SOCK_SERVER 0

#define FIFO_FILE "/tmp/UartFIFO"
#define SOCK_FILE "/tmp/UNIX.nuart"

int G_uartfd;
int G_epollfd;
int G_pipefd;
int G_sockfd;
single_linklist_head *G_recv_linklist;
single_linklist_head *G_send_linklist;

pthread_cond_t recv_linklist_not_empty;
pthread_mutex_t recv_linklist_mutex;    
pthread_cond_t send_linklist_not_empty;
pthread_mutex_t send_linklist_mutex;    


int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio, oldtio;

    if ( tcgetattr( fd,&oldtio) != 0)
    {
        DEBUG_INFO("SetupSerial 1");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch( nBits )
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    switch( nEvent )
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }
    switch( nSpeed )
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    if ( nStop == 1 )
    {
        newtio.c_cflag &= ~CSTOPB;
    }
    else if ( nStop == 2 )
    {
        newtio.c_cflag |= CSTOPB;
    }
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN]  = 1;
    tcflush(fd,TCIFLUSH);
    if ((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        DEBUG_INFO("com set error");
        return -1;
    }

    return 0;
}


int open_port(void)
{
   int fd;

    fd = open("/dev/ttyS1", O_RDWR|O_NOCTTY|O_NDELAY);
    if (-1 == fd)
    {
        perror("Can't Open Serial Port");
        return(-1);
    }
	
    set_opt(fd,115200,8,'N',1);
    return fd;
}

/*
函数名：init_uart
功能说明：
	将串口初始化并以非阻塞形式打开
*/
void init_uart()
{
	G_uartfd = open_port();
	if(G_uartfd == -1){
		perror("open /dev/ttyS1 failed\n");
		exit(0);
	}
	printf("G_uartfd: %d\n", G_uartfd);
}

/*
函数名：init_pipe
功能说明：
	初始化管道并打开
*/
void init_pipe(const char *pipename, int *pipefd)
{	
	unlink(pipename);
	mkfifo(pipename, 0777);
	
	if ((*pipefd = open(pipename, O_RDWR | O_EXCL)) == -1)
	{
		printf("pipe open error\n");
		return;
	}
	printf("pipefd: %d\n", *pipefd);
}

ssize_t whole_read(int fd, char *buf, size_t count)
{
	ssize_t n = 0, c = 0;
	size_t maxlen = count;

	char *tmp_buf[1024] = {0};
	do {
		memset(tmp_buf, 0, sizeof(tmp_buf));
		n = read(fd, tmp_buf, sizeof(tmp_buf));
		if(n > 0){
			if(c+n >= maxlen){
				break;
			}
			memcpy(&buf[c], tmp_buf, n);
			c += n;
			usleep(10*1000);
		}else{
			break;
		}
	} while (1);

	return c;
}

/*
函数名：ready_send_data
功能：
	将数据加入到数据发送链表
参数：
	enum port send_to：数据发送目标(参考enum port)
	data：待发送数据
	len：待发送数据长度
*/
int ready_send_data(enum port send_to, const char *data, int len)
{
	if(len <= 0 || data == NULL){
		return -1;
	}
	single_linklist *send_datanode = singlelinklist_init_node(len);

	send_datanode->data->data_len = len;
	send_datanode->data->send_to = send_to;
	memcpy(send_datanode->data->pdata, data, send_datanode->data->data_len);

	inf("data has been inserted in G_send_linklist");
	singlelinklist_insert_node(G_send_linklist, send_datanode);
	pthread_cond_signal(&send_linklist_not_empty);	

	return 0;
}

void *send_out_handler(void *arg)
{
	int ret = 0;

	single_linklist *datanode = singlelinklist_init_node(1024);
	while(1){
		pthread_mutex_lock(&send_linklist_mutex);
		if(is_singlelinklist_empty(G_send_linklist)){
			pthread_cond_wait(&send_linklist_not_empty, &send_linklist_mutex);
		}
		pthread_mutex_unlock(&send_linklist_mutex);
		
		singlelinklist_pop_node(G_send_linklist, datanode);			//从数据发送链表中取出数据
		inf("sending to [%d]: {%s}", datanode->data->send_to, datanode->data->pdata);
		switch(datanode->data->send_to){							
			case DUER:							//将数据发送给duer_linux进程
				ret = send_to_dueros(datanode->data->pdata, datanode->data->data_len);
				if(ret < 0){
					err("send data {%s} to [%d] error", datanode->data->pdata, datanode->data->send_to);
				}
				break;
			case UART:							//将数据通过串口发送给蓝牙
				ret = write(G_uartfd, datanode->data->pdata, datanode->data->data_len);
				if(ret != datanode->data->data_len){
					err("send data {%s} to [%d] error", datanode->data->pdata, datanode->data->send_to);
				}
				break;
			default:
				break;
		}
		
		memset(datanode->data->pdata, 0, 1024);
		datanode->data->data_len = 0;
		datanode->data->recv_fd = -1;
		
	}
	singlelinklist_destory_node(datanode);
}

void data_parse(data_inf *src)
{	
	if(src == NULL){
		err("paragrams error");
		return;
	}
	int i = 0, max = 0;
	BUF_INF data;

	inf("fd is handled: [%d], uart: [%d], pipe: [%d], sockserv: [%d]", src->recv_fd, G_uartfd, G_pipefd, G_sockfd);
	if(src->recv_fd == G_uartfd){
		//串口数据处理
		inf("uart message comes");
		max = get_sizeof_uart_handle();
		for(; i < max; i++){
			if(!strncmp(uart_handle[i].protocol, src->pdata, strlen(uart_handle[i].protocol))){
				uart_handle_analy[i]++;
				data.buf = src->pdata;
				data.len = src->data_len;
				uart_handle[i].handler(&data);
			}
		}
	}else if(src->recv_fd == G_pipefd){
		//管道数据处理
		inf("pipe message comes");
		max = get_sizeof_pipe_handle();
		for(; i < max; i++){
			if(!strncmp(pipe_handle[i].protocol, src->pdata, strlen(pipe_handle[i].protocol))){
				pipe_handle_analy[i]++;
				data.buf = src->pdata;
				data.len = src->data_len;
				pipe_handle[i].handler(&data);
			}
		}
		
	}
#if NEED_SOCK_SERVER
	else if(src->recv_fd == G_sockfd){
		//socket数据处理，待完善
		inf("socket message comes");
		
	}
#endif
	else{		
		inf("socket client message comes");
		
	}
}

void *data_process(void *arg)
{
	single_linklist *datanode = singlelinklist_init_node(1024);
	while(1){
		inf("data process...");
		pthread_mutex_lock(&recv_linklist_mutex);
		if(is_singlelinklist_empty(G_recv_linklist)){
			//waiting
			inf("recv linklist is empty");
			pthread_cond_wait(&recv_linklist_not_empty, &recv_linklist_mutex);
		}
		
		memset(datanode->data->pdata, 0, 1024);
		singlelinklist_pop_node(G_recv_linklist, datanode);		//从数据接收链表中取出数据
		pthread_mutex_unlock(&recv_linklist_mutex);
		
		//get data and parase
		inf("data recv from [%d]: %s", datanode->data->recv_fd, datanode->data->pdata);

		data_parse(datanode->data);			//处理取出的数据
	}
	return;
}

void *sock_recv_handler(void *arg)
{
	int listen_fd = *((int *)arg);
	int ret = 0;
	char recv_buf[1024] = {0};
	ret = read(listen_fd, recv_buf, sizeof(recv_buf));
	if(ret > 0){
		//将接收到的数据加入到数据接收链表中待处理
		single_linklist *sock_linknode = NULL;
		if((sock_linknode = singlelinklist_init_node(ret)) == NULL){
			return;
		}
		sock_linknode->data->recv_fd = listen_fd;
		sock_linknode->data->data_len = ret;
		memcpy(sock_linknode->data->pdata, recv_buf, ret);
		singlelinklist_insert_node(G_recv_linklist, sock_linknode);
		pthread_cond_signal(&recv_linklist_not_empty);  
	}else{
		printf("received no data\n");
	}
}

void *sock_accept_handler(void *arg)
{
	int ret = 0;
	char recv_buf[1024] = {0};
	struct sockaddr_un client_addr;  
	memset(&client_addr, 0, sizeof(client_addr));  
	int clilen = sizeof(struct sockaddr);   
	int new_sock = accept(G_sockfd, (struct sockaddr *)&client_addr, &clilen);	
	if(new_sock < 0){
		printf("socket accept failed\n");
		return;
	}
	struct epoll_event ev;	
	ev.data.fd = new_sock;	
	ev.events  = EPOLLIN;  
	epoll_ctl(G_epollfd, EPOLL_CTL_ADD, new_sock, &ev);  
}

void *uart_connect_handler(void *arg)
{
	int ret = 0;
	char recv_buf[1024] = {0};
	ret = whole_read(G_uartfd, recv_buf, sizeof(recv_buf));
	
	struct epoll_event ev;	
	ev.data.fd = G_uartfd;	
	ev.events  = EPOLLIN;  
	epoll_ctl(G_epollfd, EPOLL_CTL_ADD, G_uartfd, &ev);  	//数据接收完后立刻将串口加入epoll监听
	
	if(ret > 0){
		//将接收到的数据加入到数据接收链表中待处理
		single_linklist *uart_linknode = NULL;
		if((uart_linknode = singlelinklist_init_node(ret)) == NULL){
			return;
		}
		uart_linknode->data->recv_fd = G_uartfd;
		uart_linknode->data->data_len = ret;
		memcpy(uart_linknode->data->pdata, recv_buf, ret);
		singlelinklist_insert_node(G_recv_linklist, uart_linknode);
		pthread_cond_signal(&recv_linklist_not_empty);  
	}
	
}

void *pipe_connect_handler(void *arg)
{
	int ret = 0;
	char recv_buf[1024] = {0};
	ret = read(G_pipefd, recv_buf, sizeof(recv_buf));
	inf("pipe recv: {%s}", recv_buf);
	if(ret > 0){
		//将接收到的数据加入到数据接收链表中待处理
		single_linklist *pipe_linknode = NULL;
		if((pipe_linknode = singlelinklist_init_node(ret)) == NULL){
			return;
		}
		pipe_linknode->data->recv_fd = G_pipefd;
		pipe_linknode->data->data_len = ret;
		memcpy(pipe_linknode->data->pdata, recv_buf, ret);
		inf("pipe insert: [%d], {%s}", pipe_linknode->data->recv_fd, pipe_linknode->data->pdata);
		singlelinklist_insert_node(G_recv_linklist, pipe_linknode);
		pthread_cond_signal(&recv_linklist_not_empty);  
	}
}

void epoll_start()
{
	struct epoll_event ev[3];
	G_epollfd=epoll_create(32);

	/************ add uart fd ************/	
	setnonblocking(G_uartfd); 							//把fd设置为非阻塞方式
	ev[0].events=EPOLLIN|EPOLLET;						//设置要处理的事件类型	
	ev[0].data.fd=G_uartfd;
	epoll_ctl(G_epollfd,EPOLL_CTL_ADD,G_uartfd,&ev[0]); //注册epoll事件

#if NEED_SOCK_SERVER
	/************ add socket fd ************/
	setnonblocking(G_sockfd);
	ev[1].events=EPOLLIN|EPOLLET;
	ev[1].data.fd=G_sockfd;
	epoll_ctl(G_epollfd,EPOLL_CTL_ADD,G_sockfd,&ev[1]);
#endif
	
	/************ add pipe fd ************/
	setnonblocking(G_pipefd); 							//把fd设置为非阻塞方式
	ev[2].events=EPOLLIN|EPOLLET;						//设置要处理的事件类型	
	ev[2].data.fd=G_pipefd;
	epoll_ctl(G_epollfd,EPOLL_CTL_ADD,G_pipefd,&ev[2]); //注册epoll事件

	/**********	创建线程池      **********/
	threadpool *pool = threadpool_init(5, 10);			//参数：5-->最多5个线程				10-->队列中最多10个任务
	pthread_cond_init(&recv_linklist_not_empty, NULL);
	if (pthread_mutex_init(&recv_linklist_mutex, NULL))
    {
        err("failed to init mutex!\n");
		return;
    }
	pthread_cond_init(&send_linklist_not_empty, NULL);
	if (pthread_mutex_init(&send_linklist_mutex, NULL))
	{
		err("failed to init mutex!\n");
		return;
	}
	
	threadpool_add_job(pool, data_process, NULL);					//数据处理线程，从数据接收链表中获取数据并进行处理
	threadpool_add_job(pool, send_out_handler, NULL);				//数据发送线程，从数据发送链表中获取数据并发送
	threadpool_add_job(pool, init_data_to_bt_onstart, NULL);		//上电开机发送需要与MCU同步的数据
	
	struct epoll_event events[32];
	int nfds = -1, i = 0, ret = 0;
	int connfd;
	char recv_buf[1024] = {0};
	while(1){
		inf("epoll waiting...\n");
		nfds=epoll_wait(G_epollfd,events,32,-1); //监听epoll事件
        for(i=0;i<nfds;++i){ //处理所有接收事件
        	
			if(events[i].data.fd ==G_uartfd){
				/**********		串口数据接收			 **********/
				inf("uart fd: %d\n", events[i].data.fd);
				//因为处理串口数据时间较长，此过程中epoll不再监听串口，否则会有多个线程被启动，并导致数据遗失
				epoll_ctl(G_epollfd, EPOLL_CTL_DEL, G_uartfd, NULL);  		
				threadpool_add_job(pool, uart_connect_handler, NULL);	

			}else if(events[i].data.fd ==G_pipefd){
				/**********		管道数据接收			 **********/
				inf("pipe fd: %d", events[i].data.fd);
				threadpool_add_job(pool, pipe_connect_handler, NULL);

			}
#if NEED_SOCK_SERVER
			else if(events[i].data.fd == G_sockfd){
				/**********		socket client接入		 **********/
			 	inf("sock fd: %d\n", events[i].data.fd);
				if(events[i].events & EPOLLIN){
					threadpool_add_job(pool, sock_accept_handler, NULL);
				}
			
			}else if (events[i].events & EPOLLIN){
				/**********		socket client数据接收		 **********/
				inf("EPOLLIN\n");
#if 1
				threadpool_add_job(pool, sock_recv_handler, (void *)&(events[i].data.fd));
#else
				memset(recv_buf, 0, sizeof(recv_buf));
				ret = read(events[i].data.fd, recv_buf, sizeof(recv_buf));
				printf("recv from [%d]: %s\n", events[i].data.fd, recv_buf);
#endif
			}
#endif
			else{
				inf("epoll nothing!");
			}
		}
	}
}

/*
功能说明：
	本例程中包含串口通信、管道通信和SOCKET进程间通信，其中SOCKET进程间通信需要打开NEED_SOCK_SERVER宏；
	串口通信、管道通信和SOCKET进程间通信的数据接收入口处于		（1）epoll机制监听下，通过相应的回调函数进行数据接收；
	串口通信、管道通信和SOCKET进程间通信的数据发送出口统一在（2）send_out_handler()线程中，数据发送先进先出；
	数据接收全部放入（3）G_recv_linklist数据链表中，并在（5）data_process()线程中处理；
	data_process()线程中处理后待发送数据全部放入（4）G_send_linklist数据链表中，在（2）send_out_handler()线程中发送；

 		   ||											  //\\ 	
		   ||      										 //||\\
	     \\||//											   ||
	      \\//											   ||
	---------------									---------------	
	|	 （1）	  |									|	 （2）	  |
	|	数据接收	  |									|	数据发送      |
	|			  |									|			  |
	---------------									---------------	
		   ||											  ||
			\\											 //	
			 \\											//
			 ------------------			------------------
			 |		（3）		  |			|	   （4）		 |
			 |	接收数据存储        |			|  发送数据存储        |
			 |				  |			|				 |
			 ------------------			------------------
				 		 ||					  ||
				 		  \\				 //
				 		   \\				//
				 		   ------------------
				 		   |	  （5）	    |
				 		   |	数据处理        |
				 		   |			    |
				 		   ------------------
*/
int main(int argc, char **argv)
{
	log_init(argv[1]);
	init_uart();								//打开串口准备读写数据
	init_pipe(FIFO_FILE, &G_pipefd);			//创建并打开管道，准备接收数据
#if NEED_SOCK_SERVER
	init_socket_server(SOCK_FILE, &G_sockfd);	//创建socket进程间通信服务器，准备接收client接入
#endif
	G_recv_linklist = init_singlelinklist();	//创建数据接收链表：存放串口、管道、socket服务器接收到的数据
	G_send_linklist = init_singlelinklist();	//创建数据发送链表：存放所有需要发送的数据
	
	epoll_start();								//用epoll机制监听串口、管道、socket服务器数据接收
	
	return 0;
}

