/******************************************************************************

                  版权所有 (C), 2015-2050, 厦门优胜卫厨有限公司

 ******************************************************************************
  文 件 名   : bell.c
  版 本 号   : 初稿
  作    者   : zgj
  生成日期   : 2019年11月19日
  最近修改   :
  功能描述   : 蜂鸣器驱动
  函数列表   :
  修改历史   :
  1.日    期   : 2019年11月19日
    作    者   : zgj
    修改内容   : 创建文件

******************************************************************************/
#include "bell.h"
#include "osif.h"
#include "dbg.h"
#include "Mycfg.h"



typedef union {
	struct {
		u16 playBit;
		u8	playCnt;
		u8	res;
	};
	u32 playVal;
} tBellParam_t;

TimerHandle_t   BellTimeHandle	= NULL;



tBellParam_t bellParam;


void BellTimerFunc( void *pvParameters);

/*****************************************************************************
 函 数 名  : BellTimerFunc
 功能描述  : 蜂鸣器定时查询函数
 输入参数  : void *pvParameters
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年11月19日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void BellTimerFunc( void *pvParameters)
{
    bell_proc();
}

/******************************************************************************
 * 函数名称: bell_init
 * 功能描述: 蜂鸣器初始化，系统初始化会调用
 * 参数说明:
		void --
 * 返回结果:
		无
 * 历史记录:
 ----------------------
	创建: 钟观金 2016年12月12日10:40:35
******************************************************************************/
void bell_init(void)
{
	// for Pc7
	gpio_config( GPIOC,GPIO_Pin_7, GPIO_Mode_Out_PP);

    BellTimeHandle = TimerCreate(1, BellTimerFunc);

	bell_set(BELL_PLAY_L1S2);
}

/******************************************************************************
 * 函数名称: bell_proc
 * 功能描述: 蜂鸣器执行函数，系统定时任务自行调用。
 * 参数说明:
		void --
 * 返回结果:
		无
 * 历史记录:
 ----------------------
	创建: 钟观金 2016年12月12日10:41:28
******************************************************************************/
void bell_proc(void)
{
	u32 ticks = 100;
    // eg:bellParam.playVal = 0x000A03F5 BELL_PLAY_L1S2
	if (bellParam.playCnt != 0)
	{
		u16 bitVal = 1;
		u16 bitChk;

		bellParam.playCnt--;
		bitVal <<= (bellParam.playCnt & 0x0F);  //bitVal = 1 << (bellParam.playCnt & 0x0F) = 1<<10 = 0x0400

		bitChk = bitVal & bellParam.playBit;    //bitChk = 0x0400 & 0x03F5 = 0

		//检测 bellParam.playBit有几个1/0
		while(bellParam.playCnt != 0)
		{
			bitVal /= 2;      //0x0400/2 = 0x0200 右移1位

			if(bitChk != 0)   //chk=1
			{
				if((bitVal & bellParam.playBit) == 0)   //则检查playBit下一个位的值为 0 ?
				{
					break;
				}
			}
			else            //chk=0
			{
				if((bitVal & bellParam.playBit) != 0)   //则检查 playBit下一个位的值为 1 ?
				{
					break;
				}
			}

			ticks += 100;
			bellParam.playCnt--;

		}

		if(bitChk != 0)
		{
			bell_on();
		}
		else
		{
			bell_off();
		}

		TimerStart(BellTimeHandle, ticks);
	}
	else
	{
		bell_off();
	}
	return;
}

/******************************************************************************
 * 函数名称: bell_set
 * 功能描述: 蜂鸣器响声设置，同一时刻只能一种响声，以最后设置为准，
                之前未执行完的会被提起结束。
 * 参数说明:
		u32 playVal -- 不同响声设置值
 * 返回结果:
		无
 * 历史记录:
 ----------------------
	创建: 钟观金 2016年12月12日10:44:02
******************************************************************************/
void bell_set(u32 playVal)
{
	bellParam.playVal = playVal;
	TimerStart(BellTimeHandle, 1);
}


