/******************************************************************************

                  ��Ȩ���� (C), 2015-2050, ������ʤ�������޹�˾

 ******************************************************************************
  �� �� ��   : TaskMain.h
  �� �� ��   : ����
  ��    ��   : zgj
  ��������   : 2019��11��14��
  ����޸�   :
  ��������   : ������ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��11��14��
    ��    ��   : zgj
    �޸�����   : �����ļ�

******************************************************************************/
#include "config.h"
#include "osif.h"
#include "dbg.h"


typedef struct
{
	u8 mode;	// ģʽ: 0-���� 1-��ʼ�� 2-���� ...
	u8 state;	// ״̬
} tDevInfo_t;

extern tDevInfo_t DevInfo;

/********************************************************************/
// for h file
typedef void (*appIf_func)(Msg_t pm);

typedef struct {
	appIf_func if_timer;    //tMsg_t.src
	appIf_func if_rfid;
	appIf_func if_pin;
	appIf_func if_state;
	appIf_func if_key;
} tAppMode_t;

extern const tAppMode_t TabAppMode[];


void app_funcDef(Msg_t pm);
void app_stateSet(u32 state);
void app_modeSet(u32 mode);


extern  void TaskMain( void *pvParameters );

