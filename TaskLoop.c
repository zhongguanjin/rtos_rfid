/******************************************************************************

                  ��Ȩ���� (C), 2015-2050, ������ʤ�������޹�˾

 ******************************************************************************
  �� �� ��   : TaskLoop.c
  �� �� ��   : ����
  ��    ��   : zgj
  ��������   : 2019��11��14��
  ����޸�   :
  ��������   : ʱ��Ƭ��ѯ����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��11��14��
    ��    ��   : zgj
    �޸�����   : �����ļ�

******************************************************************************/
#include "TaskLoop.h"
#include "osif.h"
#include "dbg.h"
#include "com.h"
#include "TaskRfid.h"
#include "Syn6658.h"
#include "mycfg.h"
#include "ccu.h"

void Led_DispPro(void);
void led_init(void);


/******************************************************************************
 * ��������: led_init
 * ��������: led�Ƴ�ʼ��
 * ����˵��:
		void --
 * ���ؽ��:
		��
 * ��ʷ��¼:
 ----------------------
	����: �ӹ۽� 2016��12��12��10:40:35
******************************************************************************/
void led_init(void)
{
	// for PA1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	gpio_config( GPIOA,GPIO_Pin_1, GPIO_Mode_Out_PP);
}

/*****************************************************************************
 �� �� ��  : Led_SetMode
 ��������  : LED�ƹ�ģʽ����
 �������  : T_LED_MODE mode
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2019��11��20��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Led_SetMode(T_LED_MODE mode)
{
	switch(mode)
	{
		case LED_INIT:
		{
			dbg("Led Mode:Init");
			LedTmr[LEDTMR_RED].u = LED_KEEP_VALUE;
			break;
		}
		case LED_IDLE:
		{
			dbg("Led Mode:Idle");
			LedTmr[LEDTMR_RED].u = LED_BLINK_SLOW;
			break;
		}
		case LED_WORKING:
		{
			dbg("Led Mode:Working");
			LedTmr[LEDTMR_RED].u = LED_BLINK_FAST;
			break;
		}
		case LED_ALARM:
		{
			dbg("Led Mode:Alarm");
			LedTmr[LEDTMR_RED].u = LED_BLINK_FAST;
			break;
		}
		case LED_ERROR:
		{
			dbg("Led Mode:Error");
			LedTmr[LEDTMR_RED].u = LED_BLINK_FAST;
			break;
		}
		default:
		{
			dbg("mode undefine!!");
			break;
		}
	}
	LedTmrBak[LEDTMR_RED].u = LedTmr[LEDTMR_RED].u;
}

/* ÿ100msִ��һ�� */
void Led_DispPro(void)
{
	if(LedTmr[LEDTMR_RED].uch[3] != 0)//��λ
	{
		if(LedTmr[LEDTMR_RED].uch[2] == 0)
		{
			if(--LedTmr[LEDTMR_RED].uch[0] == 0)
			{
				LedTmr[LEDTMR_RED].uch[2] = 1;
				PA1_on();
				LedTmr[LEDTMR_RED].uch[1] = LedTmrBak[LEDTMR_RED].uch[1];
			}
		}
		else
		{
			if(--LedTmr[LEDTMR_RED].uch[1] == 0)
			{
				LedTmr[LEDTMR_RED].uch[2] = 0;
				PA1_off();
				LedTmr[LEDTMR_RED].uch[0] = LedTmrBak[LEDTMR_RED].uch[0];
			}
		}
	}
}



/*****************************************************************************
 �� �� ��  : TaskLoop
 ��������  : ��Ƶ�ʼ������
 �������  : void *pvParameters
 �������  : ��
 �� �� ֵ  : static
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2019��11��11��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void TaskLoop( void *pvParameters )
{
    u8 idx=0;
	com_init(syn6658_com,9600);	//com2
	com_init(ccu_com,19200);	//
	com_init(rfid_com,9600);	// for rfid
	led_init();
    dbg("CC:%s %s",__DATE__,__TIME__);
    task_sleep(10);
    dbg("loop FreeStack:%d",OSTaskGetFreeStackSpace(htaskget(1)));
    for( ;; )
    {
        if(idx == 0)
        {
            idx = LP_CHK_MAX;
        }
        idx--;
        switch(idx)
        {   // ��Ƶ�ʼ����
            case LP_COM2_RX:
            {
                syn6658_rxDeal();
                break;
            }
            case LP_COM3_RX:
            {
                ccu_rxDeal();
                break;
            }
            case LP_COM4_RX:
            {
                rfid_rxDeal();
                break;
            }
			case LP_LEDDISP:
			{
				static int loop = 10;
				if(--loop == 0)
				{
					loop = 10;
					Led_DispPro();
				}
				break;
			}

            default:
            {
                break;
            }
        }
        task_sleep(LP_SLEEP_TICKS);
    }
}

