/******************************************************************************

                  版权所有 (C), 2015-2050, 厦门优胜卫厨有限公司

 ******************************************************************************
  文 件 名   : bell.h
  版 本 号   : 初稿
  作    者   : zgj
  生成日期   : 2019年11月19日
  最近修改   :
  功能描述   : 蜂鸣器驱动头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年11月19日
    作    者   : zgj
    修改内容   : 创建文件

******************************************************************************/
#ifndef _BELL_H_
#define _BELL_H_


#include "config.h"






#define BELL_PLAY_SHORT1	0x00060001	// 短响一声
#define BELL_PLAY_SHORT2	0x00080005	// 短响两声
#define BELL_PLAY_SHORT3	0x000A0015	// 短响三声
#define BELL_PLAY_SHORTx	0x00C81111	// 连续短响
#define BELL_PLAY_LONG1		0x000A003F	// 长响一声
#define BELL_PLAY_LONGx		0x00C8FFFF	// 长响10秒
#define BELL_PLAY_L1S2		0x000A03F5	// 长响一声 + 短响两声


#define bell_on()	{GPIOC->BSRR = GPIO_Pin_7;}
#define bell_off()	{GPIOC->BRR = GPIO_Pin_7;}



void bell_init(void);
void bell_proc(void);
void bell_set(u32 playVal);



#endif
