#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "config.h"
#include "com.h"

#define console_com  COM1
#define console_uart USART1

typedef int (*consoleCallback)(char * buf, int len);
extern consoleCallback console_cb;


extern void TaskConsole( void *pvParameters );






#endif
