#ifndef _DBG_H
#define _DBG_H


#include "config.h"

#define DBG_USE  1

#if (DBG_USE != 0)
#define dbg(fmt,args...)	printf("[%s]:" fmt "\r\n",__func__,## args)
#define chk(x);				if(x)dbg("[Chk]%s:%d\r\n",__FILE__,(int)__LINE__);

void dbg_out(char * buf, u32 len);


#else
#define dbg(fmt,args...)
#define chk(x);
void dbg_out(char * buf, u32 len);


#endif







extern void dbg_hex(uint8 *buf,uint16 len);

extern uint8 CRC8_SUM(void *p,uint8 len);



#endif

