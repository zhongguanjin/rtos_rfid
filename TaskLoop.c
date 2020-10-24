/******************************************************************************

                  版权所有 (C), 2015-2050, 厦门优胜卫厨有限公司

 ******************************************************************************
  文 件 名   : TaskLoop.c
  版 本 号   : 初稿
  作    者   : zgj
  生成日期   : 2019年11月14日
  最近修改   :
  功能描述   : 时间片轮询任务
  函数列表   :
  修改历史   :
  1.日    期   : 2019年11月14日
    作    者   : zgj
    修改内容   : 创建文件

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
 * 函数名称: led_init
 * 功能描述: led灯初始化
 * 参数说明:
		void --
 * 返回结果:
		无
 * 历史记录:
 ----------------------
	创建: 钟观金 2016年12月12日10:40:35
******************************************************************************/
void led_init(void)
{
	// for PA1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	gpio_config( GPIOA,GPIO_Pin_1, GPIO_Mode_Out_PP);
}

/*****************************************************************************
 函 数 名  : Led_SetMode
 功能描述  : LED灯光模式设置
 输入参数  : T_LED_MODE mode
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年11月20日
    作    者   : zgj
    修改内容   : 新生成函数

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

/* 每100ms执行一次 */
void Led_DispPro(void)
{
	if(LedTmr[LEDTMR_RED].uch[3] != 0)//高位
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
 函 数 名  : TaskLoop
 功能描述  : 分频率检查任务
 输入参数  : void *pvParameters
 输出参数  : 无
 返 回 值  : static
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年11月11日
    作    者   : zgj
    修改内容   : 新生成函数

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
        {   // 分频率检查项
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

