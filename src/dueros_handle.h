#ifndef __DUEROS_HANDLE_H__
#define __DUEROS_HANDLE_H__

#include "uart_protocol.h"

#include <stdio.h>

enum INSTRUCTION_TYPE{
	BUTTON,
	URL
};

BUF_INF *format_duer_button_data(int sum, int src, ...);
BUF_INF *format_duer_string_data(int sum, const char *src, ...);
BUF_INF *format_duer_binary_data(const void *data, unsigned int len);
#endif
