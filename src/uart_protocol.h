#ifndef _UART_PROTOCOL_H__
#define _UART_PROTOCOL_H__
#include "uartd_test_mode.h"

typedef struct protocol_handle{
	const char *protocol;
	void (*handler)(void *arg);
}protocol_handle;

typedef struct{
	char *buf;
	unsigned int len;
}BUF_INF;

#define SSBUF_LEN_512    512 
#define SSBUF_LEN_256    256 
#define SSBUF_LEN_128    128 
#define SSBUF_LEN_64    64 
#define SSBUF_LEN_16    16

extern unsigned int uart_handle_analy[64];
extern unsigned int pipe_handle_analy[128];
extern protocol_handle uart_handle[];
extern protocol_handle pipe_handle[];
int get_sizeof_pipe_handle();
int get_sizeof_uart_handle();

#endif
