/******************************************************************************

                  ��Ȩ���� (C), 2015-2050, ������ʤ�������޹�˾

 ******************************************************************************
  �� �� ��   : bell.h
  �� �� ��   : ����
  ��    ��   : zgj
  ��������   : 2019��11��19��
  ����޸�   :
  ��������   : ����������ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��11��19��
    ��    ��   : zgj
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef _BELL_H_
#define _BELL_H_


#include "config.h"






#define BELL_PLAY_SHORT1	0x00060001	// ����һ��
#define BELL_PLAY_SHORT2	0x00080005	// ��������
#define BELL_PLAY_SHORT3	0x000A0015	// ��������
#define BELL_PLAY_SHORTx	0x00C81111	// ��������
#define BELL_PLAY_LONG1		0x000A003F	// ����һ��
#define BELL_PLAY_LONGx		0x00C8FFFF	// ����10��
#define BELL_PLAY_L1S2		0x000A03F5	// ����һ�� + ��������


#define bell_on()	{GPIOC->BSRR = GPIO_Pin_7;}
#define bell_off()	{GPIOC->BRR = GPIO_Pin_7;}



void bell_init(void);
void bell_proc(void);
void bell_set(u32 playVal);



#endif
