/******************************************************************************

                  版权所有 (C), 2015-2050, 厦门优胜卫厨有限公司

 ******************************************************************************
  文 件 名   : delay.c
  版 本 号   : 初稿
  作    者   : zgj
  生成日期   : 2019年11月5日
  最近修改   :
  功能描述   : delay延时函数
  函数列表   :
              delay_us
              SysTick_Handler
  修改历史   :
  1.日    期   : 2019年11月5日
    作    者   : zgj
    修改内容   : 创建文件

******************************************************************************/

#include "delay.h"

static __IO u32 TimingDelay;

/**
  * @brief   us延时程序,1us为一个单位
  * @param
  *		@arg nTime: delay_us( 1 ) 则实现的延时为 1 us
  * @retval  无
  */
void delay_us(__IO u32 nTime)
{
    #if(OS_DELAY_ON !=0)
        if (SysTick_Config(SystemCoreClock/ 1000000))   // ST3.5.0???
        {
            /* Capture error */
            while (1);
        }
        TimingDelay = nTime;
        // 使能滴答定时器
        SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
        while(TimingDelay != 0);

        SysTick->CTRL=0x00; //关闭计数器
        SysTick->VAL =0X00; //清空计数器
    #else
        nTime=nTime*60;
        while(nTime--);
    #endif
}

/**
  * @brief   us延时程序,1ms为一个单位
  * @param
  *		@arg nTime: delay_ms( 1 ) 则实现的延时为 1 ms
  * @retval  无
  */
void delay_ms(__IO u32 nTime)
{
    #if(OS_DELAY_ON !=0)
        if (SysTick_Config(SystemCoreClock/ 1000))   // ST3.5.0???
        {
            /* Capture error */
            while (1);
        }
        TimingDelay = nTime;
        // 使能滴答定时器
        SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
        while(TimingDelay != 0);

        SysTick->CTRL=0x00; //关闭计数器
        SysTick->VAL =0X00; //清空计数器
    #else
        nTime=nTime*6000;
        while(nTime--);
    #endif
}

#if(OS_DELAY_ON !=0)

void SysTick_Handler(void)
{
    if(TimingDelay)
        TimingDelay--;
}

#endif









/*********************************************END OF FILE**********************/
