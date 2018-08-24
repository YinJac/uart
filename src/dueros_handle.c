#include "dueros_handle.h"
#include <stdarg.h>
#include<stdlib.h>
#include<string.h>
#include "debug.h"

#define DIRECT_URL_BUFSIZE 1024
#define DIRECT_BUTTON_BUFSIZE 64
#define DELIMITER '`'
#define TERMINATOR '\n'

/*
函数名：format_duer_button_data
功能：
	将按键数据格式化为dueros定制的数据格式，返回格式化后的数据结构体
para instruction:
	int sum: 参数个数(不算sum.从1开始数)
只能传int型参数
*/
BUF_INF *format_duer_button_data(int sum, int src, ...)
{
  int para;
  int argno = 1, i = 0, type = BUTTON;  
  BUF_INF *data = NULL;
  char *dest = (char *)malloc(DIRECT_BUTTON_BUFSIZE);
  if(dest){
	  memset(dest, 0, DIRECT_BUTTON_BUFSIZE);
  }else{
	  err("malloc error");
	  goto ERR;
  }
  memcpy(&dest[i], &type, sizeof(int));
  i += sizeof(int);
  dest[i++] = DELIMITER;
  va_list args;
  va_start(args,src);
  inf("fmt: %d", src);
  memcpy(&dest[i], &src, sizeof(int));
  i += sizeof(int);
  dest[i++] = DELIMITER;
  inf("contex: %s", dest);
  while(argno < sum){
    para = va_arg(args, int );
    inf("para: %d, i: %d", para, i);
    memcpy(&dest[i], &para, sizeof(int));
    i += sizeof(int);
    dest[i++] = DELIMITER;
    argno++;
  }
  va_end(args);
  dest[i-1] = TERMINATOR;
  
  data = (BUF_INF *)calloc(1, sizeof(BUF_INF));
  if(data == NULL){
	  err("malloc error");
	  goto ERR;
  }
  data->buf = dest;
  data->len = i;

  return data;

ERR:
	if(dest){
		free(dest);
	}
	if(data){
		free(data);
	}
	return NULL;
}

/*
函数名：format_duer_button_data
功能：
	将字符串数据格式化为dueros定制的数据格式，返回格式化后的数据结构体

para instruction:
	int sum: 参数个数(不算sum.从1开始数)
只能传char *型参数
*/
BUF_INF *format_duer_string_data(int sum, const char *src, ...)
{
  char *para;
  BUF_INF *data = NULL;
  int argno = 1, i = 0, type = URL;
  char *dest = (char *)malloc(DIRECT_URL_BUFSIZE);
  if(dest){
	  memset(dest, 0, DIRECT_URL_BUFSIZE);
  }else{
	  err("malloc error");
	  goto ERR;
  }
  memcpy(&dest[i], &type, sizeof(int));
  i += sizeof(int);
  dest[i++] = DELIMITER;
  
  va_list args;
  va_start(args,src);
  vsprintf(&dest[i],src,args);
  i += strlen(src);
  dest[i++] = DELIMITER;
  inf("contex: %s", dest);
  while(argno < sum){
	para = va_arg(args, char *);
	inf("para: %s, i: %d", para, i);
	vsprintf(&dest[i],para,args);
	i += strlen(para);
	dest[i++] = DELIMITER;
	argno++;
  }
  va_end(args);
  dest[i-1] = TERMINATOR;
  
  data = (BUF_INF *)calloc(1, sizeof(BUF_INF));
  data->buf = dest;
  data->len = i;

  return data;

ERR:
  if(dest){
	  free(dest);
  }
  if(data){
	  free(data);
  }
  return NULL;
}

/*
函数名：format_duer_button_data
功能：
	将二进制数据格式化为dueros定制的数据格式，返回格式化后的数据结构体
*/
BUF_INF *format_duer_binary_data(const void *data, unsigned int len)
{
  int i = 0, type = URL;
  BUF_INF *data_ret = NULL;
  char *dest = (char *)malloc(DIRECT_URL_BUFSIZE);
  if(dest){
	  memset(dest, 0, DIRECT_URL_BUFSIZE);
  }else{
	  err("malloc error");
	  goto ERR;
  }
  memcpy(&dest[i], &type, sizeof(int));
  i += sizeof(int);
  dest[i++] = DELIMITER;
  memcpy(&dest[i], data, len);
  i += len;
  dest[i++] = TERMINATOR;
  
	data_ret = (BUF_INF *)calloc(1, sizeof(BUF_INF));
	data_ret->buf = dest;
	data_ret->len = i;
  
	return data_ret;
  
  ERR:
	if(dest){
		free(dest);
	}
	if(data_ret){
		free(data_ret);
	}
	return NULL;
}



