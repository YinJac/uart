#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <libcchip/platform.h>
#define UARTD_DEBUG  

#ifdef UARTD_DEBUG  
#define DEBUG_ERR(x...)		err(x)
#define DEBUG_INFO(x...)	inf(x)
#define DEBUG_TRACE(x...)	trc(x)
#define DEBUG_PERR(x...)	show_errno(0,x)
#else
#define DEBUG_ERR(x...)
#define DEBUG_INFO(x...) 
#define DEBUG_TRACE(x...)
#define DEBUG_PERR(x...)
#endif

			

#endif