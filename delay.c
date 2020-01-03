/******************************************************************************

                  ��Ȩ���� (C), 2015-2050, ������ʤ�������޹�˾

 ******************************************************************************
  �� �� ��   : delay.c
  �� �� ��   : ����
  ��    ��   : zgj
  ��������   : 2019��11��5��
  ����޸�   :
  ��������   : delay��ʱ����
  �����б�   :
              delay_us
              SysTick_Handler
  �޸���ʷ   :
  1.��    ��   : 2019��11��5��
    ��    ��   : zgj
    �޸�����   : �����ļ�

******************************************************************************/

#include "delay.h"

static __IO u32 TimingDelay;

/**
  * @brief   us��ʱ����,1usΪһ����λ
  * @param
  *		@arg nTime: delay_us( 1 ) ��ʵ�ֵ���ʱΪ 1 us
  * @retval  ��
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
        // ʹ�ܵδ�ʱ��
        SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
        while(TimingDelay != 0);

        SysTick->CTRL=0x00; //�رռ�����
        SysTick->VAL =0X00; //��ռ�����
    #else
        nTime=nTime*60;
        while(nTime--);
    #endif
}

/**
  * @brief   us��ʱ����,1msΪһ����λ
  * @param
  *		@arg nTime: delay_ms( 1 ) ��ʵ�ֵ���ʱΪ 1 ms
  * @retval  ��
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
        // ʹ�ܵδ�ʱ��
        SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
        while(TimingDelay != 0);

        SysTick->CTRL=0x00; //�رռ�����
        SysTick->VAL =0X00; //��ռ�����
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
