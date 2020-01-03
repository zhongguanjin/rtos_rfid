/******************************************************************************

                  ��Ȩ���� (C), 2015-2050, ������ʤ�������޹�˾

 ******************************************************************************
  �� �� ��   : bell.c
  �� �� ��   : ����
  ��    ��   : zgj
  ��������   : 2019��11��19��
  ����޸�   :
  ��������   : ����������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��11��19��
    ��    ��   : zgj
    �޸�����   : �����ļ�

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
 �� �� ��  : BellTimerFunc
 ��������  : ��������ʱ��ѯ����
 �������  : void *pvParameters
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2019��11��19��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void BellTimerFunc( void *pvParameters)
{
    bell_proc();
}

/******************************************************************************
 * ��������: bell_init
 * ��������: ��������ʼ����ϵͳ��ʼ�������
 * ����˵��:
		void --
 * ���ؽ��:
		��
 * ��ʷ��¼:
 ----------------------
	����: �ӹ۽� 2016��12��12��10:40:35
******************************************************************************/
void bell_init(void)
{
	// for Pc7
	gpio_config( GPIOC,GPIO_Pin_7, GPIO_Mode_Out_PP);

    BellTimeHandle = TimerCreate(1, BellTimerFunc);

	bell_set(BELL_PLAY_L1S2);
}

/******************************************************************************
 * ��������: bell_proc
 * ��������: ������ִ�к�����ϵͳ��ʱ�������е��á�
 * ����˵��:
		void --
 * ���ؽ��:
		��
 * ��ʷ��¼:
 ----------------------
	����: �ӹ۽� 2016��12��12��10:41:28
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

		//��� bellParam.playBit�м���1/0
		while(bellParam.playCnt != 0)
		{
			bitVal /= 2;      //0x0400/2 = 0x0200 ����1λ

			if(bitChk != 0)   //chk=1
			{
				if((bitVal & bellParam.playBit) == 0)   //����playBit��һ��λ��ֵΪ 0 ?
				{
					break;
				}
			}
			else            //chk=0
			{
				if((bitVal & bellParam.playBit) != 0)   //���� playBit��һ��λ��ֵΪ 1 ?
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
 * ��������: bell_set
 * ��������: �������������ã�ͬһʱ��ֻ��һ�����������������Ϊ׼��
                ֮ǰδִ����Ļᱻ���������
 * ����˵��:
		u32 playVal -- ��ͬ��������ֵ
 * ���ؽ��:
		��
 * ��ʷ��¼:
 ----------------------
	����: �ӹ۽� 2016��12��12��10:44:02
******************************************************************************/
void bell_set(u32 playVal)
{
	bellParam.playVal = playVal;
	TimerStart(BellTimeHandle, 1);
}


