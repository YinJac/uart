#include "data_fifo.h"

#define HORIZON 64
void print_graph(fifo *fifo)
{
    int i, j = 0;
	int in_count[2] = {0}, out_count[2] = {0};
    int len = fifo->buf_size;
    int in = fifo->head;
    int out = fifo->tail;
    char inprint[2] = {0};
    char outprint[2] = {0};
    const char *linedata = fifo->buffer;
    printf("\033[2J");
	for(i = 0; i < len;){
		memset(inprint, 0, sizeof(inprint));
		memset(outprint, 0, sizeof(outprint));
		for(j = 0; j < HORIZON; j++){
			inprint[0] = ' ';
			if(in_count[0] == in){
				inprint[0] = '|';
			}
			printf(" %c ", inprint[0]);
			in_count[0]++;
		}
		printf("\n");
		for(j = 0; j < HORIZON; j++){
			inprint[1] = ' ';
			if(in_count[1] == in){
				inprint[1] = '0';
			}
			printf(" %c ", inprint[1]);
			in_count[1]++;
		}
		printf("\n");
		for(j = 0; j < HORIZON; j++){
			printf("===");
		}
		printf("\n");
		for(j = 0; j < HORIZON; j++){
			printf("|%02hhx", linedata[i++]);
		}
		printf("\n");
		for(j = 0; j < HORIZON; j++){
			printf("===");
		}
		printf("\n");
		for(j = 0; j < HORIZON; j++){
			outprint[0] = ' ';
			if(out_count[0] == out){
				outprint[0] = '|';
			}
			printf(" %c ", outprint[0]);
			out_count[0]++;
		}
		printf("\n");
		for(j = 0; j < HORIZON; j++){
			outprint[1] = ' ';
			if(out_count[1] == out){
				outprint[1] = '0';
			}
			printf(" %c ", outprint[1]);
			out_count[1]++;
		}
		printf("\n");
    }

	return ;
}


fifo *fifo_alloc(unsigned int length)
{
	fifo *f	= (fifo *)calloc(1, sizeof(fifo));
	if(f == NULL){
		DEBUG_ERR("fifo calloc failed");
		goto exit;
	}
	f->buffer = (char *)calloc(1, length);
	if(f->buffer == NULL){
		DEBUG_ERR("fifo buffer calloc failed");
		goto exit;
	}

	return f;

exit:
	if(f != NULL){
		free(f);
	}
	return NULL;
}

void fifo_free(fifo *f)
{
	if(f != NULL){
		if(f->buffer != NULL){
			free(f->buffer);
		}
		free(f);
	}
}

int fifo_pop(fifo *f, char *buf, unsigned int maxlen)
{
	int i = 0;
	
	pthread_mutex_lock(&(f->rwlock));
	for(i = 0; i < maxlen; i++){
		if(f->data_len == 0){
			DEBUG_INFO("fifo is empty");
			break;
		}
		buf[i] = f->buffer[f->tail++];
		f->data_len--;
		f->tail %= f->buf_size;
	}
	pthread_mutex_unlock(&(f->rwlock));
	return i;
}

int fifo_put(fifo *f, char *data, unsigned int len)
{
	int i = 0;
	pthread_mutex_lock(&(f->rwlock));
	for(i = 0; i < len; i++){
		if(f->data_len == f->buf_size){
			DEBUG_INFO("fifo is full");
			break;
		}
		f->buffer[f->head++] = data[i];
		f->data_len++;
		f->head %= f->buf_size;
	}
	pthread_mutex_unlock(&(f->rwlock));
	return i;
}

int is_fifo_full(fifo *f)
{
	int ret = -1;
	if(f){
		pthread_mutex_lock(&(f->rwlock));
			if(f->data_len == f->buf_size){
				ret = 1;
			}else{
				ret = 0;
			}
		pthread_mutex_unlock(&(f->rwlock));
	}
	return ret;

}

int is_fifo_empty(fifo *f)
{
	int ret = -1;
	if(f){
		pthread_mutex_lock(&(f->rwlock));
			if(f->data_len == 0){
				ret = 1;
			}else{
				ret = 0;
			}
		pthread_mutex_unlock(&(f->rwlock));
	}
	return ret;
}

int init_data_fifo(fifo **f, unsigned int length)
{
	*f = fifo_alloc(length);
	if(NULL == *f)
		return -1;
	(*f)->buf_size = length;
	(*f)->data_len = 0;
	(*f)->head = (*f)->tail = 0;
	pthread_mutex_init(&((*f)->rwlock), NULL);

	return 0;	
}


