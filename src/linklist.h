#ifndef _LINKLIST_H__
#define _LINKLIST_H__
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

//#define err(x...) 

enum port{
	DUER = 1,
	UART
};

typedef struct data_inf{
	char *pdata;
	int data_len;
	int recv_fd;
	enum port send_to;
}data_inf;

typedef struct single_linklist{
	struct single_linklist *next;
	data_inf *data;	
}single_linklist;

typedef struct single_linklist_head{
	single_linklist *linklist_head;
	single_linklist *linklist_tail;
	int linknode_count;
    pthread_mutex_t mutex;            //互斥信号量
}single_linklist_head;

single_linklist_head *init_singlelinklist();				//create a singlelinklist head;
single_linklist *singlelinklist_init_node(int buf_len);					//create a singlelinklist node
void singlelinklist_pop_node(single_linklist_head *head, single_linklist *datanode);				//pop a node out from a singlelinklist
void singlelinklist_insert_node(single_linklist_head *head, single_linklist *datanode);					//insert a node into a singlelinklist
void singlelinklist_destory_node(single_linklist *datanode);								//destory a singlelinklist node
void destory_singlelinklist(single_linklist_head *head);									//destory the whole singlelinklist

#endif
