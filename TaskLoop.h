/******************************************************************************

                  ��Ȩ���� (C), 2015-2050, ������ʤ�������޹�˾

 ******************************************************************************
  �� �� ��   : TaskLoop.h
  �� �� ��   : ����
  ��    ��   : zgj
  ��������   : 2019��11��14��
  ����޸�   :
  ��������   : ʱ��Ƭ��ѯ����ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��11��14��
    ��    ��   : zgj
    �޸�����   : �����ļ�

******************************************************************************/
#include "config.h"
#include "osIf.h"



/*	��ѯ�����壬LP_CHK_MAXֵ���Ϊ5�ı������Ա������ѯ��
	���ĳ�������Ҫ����Ƶ�ʣ�����ƽ�����������涨����ֵ������Ϊ�䴦��
*/
enum {
	LP_COM1_RX = 0,
	LP_COM2_RX,
	LP_COM3_RX,
    LP_COM4_RX,
    LP_LEDDISP,
	LP_CHK_MAX =10
} ;

#define LP_TIMES		100		// lp_timesΪÿ����ѯ����
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

