#ifndef _MYCFG_H_
#define _MYCFG_H_

#include "config.h"

void gpio_config(GPIO_TypeDef* GPIOx, u16 gpio_pin,GPIOMode_TypeDef gpio_mode);
void gpio_set(GPIO_TypeDef* GPIOx, u16 gpio_pin,u16 val);
void usart_config(USART_TypeDef *USARTx,u32 baud);
void nvic_config(u8 IRQn,u8 PreemptionPriority,u8 SubPriority);


#endif
