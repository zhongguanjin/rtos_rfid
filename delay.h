#ifndef _DELAY_H
#define _DELAY_H

#include "stm32f10x.h"
#include "config.h"

#define USE_TINIUXOS  (0)

#ifndef USE_TINIUXOS
  #define    OS_DELAY_ON      ( 0 )
#else
  #define    OS_DELAY_ON      (USE_TINIUXOS)
#endif


void    delay_us(__IO u32 nTime);
void    delay_ms(__IO u32 nTime);

#if(OS_DELAY_ON !=0)
extern void SysTick_Handler(void);
#endif
#endif /* _DELAY_H */
