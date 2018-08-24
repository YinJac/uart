#ifndef __DATA_FIFO_H__
#define __DATA_FIFO_H__
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
typedef struct{
	char *buffer;				//the buffer for storage data
	unsigned int buf_size;		//the buffer size
	int head;					//fifo head used for put data in
	int tail;					//fifo tail used for pop data out
	unsigned int data_len;		//record data length in fifo
	pthread_mutex_t rwlock;		//read and write mutex
}fifo;

#define DEBUG_ERR
#define DEBUG_INFO

void fifo_free(fifo *f);
int fifo_pop(fifo *f, char *buf, unsigned int maxlen);
int fifo_put(fifo *f, char *data, unsigned int len);
int is_fifo_empty(fifo *f);
int is_fifo_full(fifo *f);
int init_data_fifo(fifo **f, unsigned int length);
void print_graph(fifo *fifo);

#endif
