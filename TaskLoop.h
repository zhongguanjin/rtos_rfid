/******************************************************************************

                  版权所有 (C), 2015-2050, 厦门优胜卫厨有限公司

 ******************************************************************************
  文 件 名   : TaskLoop.h
  版 本 号   : 初稿
  作    者   : zgj
  生成日期   : 2019年11月14日
  最近修改   :
  功能描述   : 时间片轮询任务头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年11月14日
    作    者   : zgj
    修改内容   : 创建文件

******************************************************************************/
#include "config.h"
#include "osIf.h"



/*	轮询处理定义，LP_CHK_MAX值最好为5的倍数，以便均衡轮询，
	如果某个检查需要更高频率，可以平均插入在下面定义多个值，都作为其处理。
*/
enum {
	LP_COM1_RX = 0,
	LP_COM2_RX,
	LP_COM3_RX,
    LP_COM4_RX,
    LP_LEDDISP,
	LP_CHK_MAX =10
} ;

#define LP_TIMES		100		// lp_times为每秒轮询次数
#define LP_SLEEP_TICKS	(1000/(LP_CHK_MAX * LP_TIMES))



enum
{
	LEDTMR_GREEN=0,
	LEDTMR_RED,
	LEDTMR_MAX
};
/*
ch0 off time;
ch1 on time;
ch2 blink flg;
ch3 blink
*/
#define LED_BLINK_FAST		0x01010203
#define LED_BLINK_SLOW		0x01010212
#define LED_KEEP_VALUE		0
static UN32 LedTmr[LEDTMR_MAX];
static UN32 LedTmrBak[LEDTMR_MAX];

typedef enum
{
	LED_INIT = 0,
	LED_IDLE,
	LED_WORKING,
	LED_ALARM,
	LED_ERROR,
	LED_MAX
} T_LED_MODE;


#define PA0_off()	{GPIOA->BSRR = GPIO_Pin_1;}
#define PA0_on()	{GPIOA->BRR = GPIO_Pin_1;}



void Led_SetMode(T_LED_MODE mode);


extern void TaskLoop( void *pvParameters );

