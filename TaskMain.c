/******************************************************************************

                  ��Ȩ���� (C), 2015-2050, ������ʤ�������޹�˾

 ******************************************************************************
  �� �� ��   : TaskMain.c
  �� �� ��   : ����
  ��    ��   : zgj
  ��������   : 2019��11��14��
  ����޸�   :
  ��������   : ������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��11��14��
    ��    ��   : zgj
    �޸�����   : �����ļ�

******************************************************************************/
#include "TaskMain.h"

#include "bell.h"
#include "TaskLoop.h"

#include "Syn6658.h"

#include "TaskRfid.h"
#include "ccu.h"
/********************************************************************/
// �¼ܹ�: ģʽ + ״̬��


/*0-���� 1-��ʼ�� 2-����,3-����,4-����*/
enum {
	MODE_IDLE = 0,
	MODE_INIT,
	MODE_DEBUG,
	MODE_WORK,
	MODE_UPDATE,
	MODE_MAX
};

enum
{
    ST_INIT_INIT = 0, //��ʼ����ʼ
    ST_INIT_CHECK, //���
    ST_INIT_MAX
};

tDevInfo_t DevInfo;

ccubuf_t mainccubuf;


TimerHandle_t   MainTimeHandle	= NULL;

void MainTimerFunction( void *pvParameters);

void AppInit_timer(Msg_t pm);
void AppInit_state(Msg_t pm);

void AppWork_timer(Msg_t pm);
void AppWork_state(Msg_t pm);

void App_Rfid(Msg_t pm);

/* ״̬���������� */
const tAppMode_t TabAppMode[MODE_MAX] =
{
	/* IDLE */
	{	app_funcDef,	// TIMER
		App_Rfid,	// RFID
		app_funcDef,	// PIN
		app_funcDef,	// STATE
		app_funcDef,	// KEY
	},
	/* INIT */
	{	AppInit_timer,	// TIMER
		App_Rfid,	// RFID
		app_funcDef,	// PIN
		AppInit_state,	// STATE
		app_funcDef,	// KEY
	},
	/* DEBUG */
	{	app_funcDef,	// TIMER
		App_Rfid,	// RFID
		app_funcDef,	// PIN
		app_funcDef,	// STATE
		app_funcDef,	// KEY
	},
	/* WORK */
	{	AppWork_timer,	// TIMER
		App_Rfid,	// RFID
		app_funcDef,	// PIN
		AppWork_state,	// STATE
		app_funcDef,	// KEY
	}
};


void AppInit_timer(Msg_t pm)
{
    switch (DevInfo.state)
    {
        case ST_INIT_INIT :
        {
            app_stateSet(ST_INIT_CHECK);
            break;
        }
        case ST_INIT_CHECK:
        {
            app_modeSet(MODE_IDLE);
            break;
        }
        default:
            break;
    }
}

void AppInit_state(Msg_t pm)
{
    switch (DevInfo.state)
    {
        case ST_INIT_INIT :
        {
            TimerStart(MainTimeHandle,1000);
            break;
        }
        case ST_INIT_CHECK:
        {
            TimerStart(MainTimeHandle,2000);
            break;
        }
        default:
            break;
    }
}


void AppWork_timer(Msg_t pm)
{

}

void AppWork_state(Msg_t pm)
{

}

void App_Rfid(Msg_t pm)
{
	//dbg("id:%d, serial:%02X %02X %02X %02X",pm->id,
	//	pm->uch[3],pm->uch[2],pm->uch[1],pm->uch[0]);
    u32 money;
    memcpy(&money,pm->uch,4);
    switch(pm->id)
    {
        case RFID_GET:
        {
           #if (RFID_TYPE == MASTER_TYPE)
            memcpy(mainccubuf.info,pm->uch,4);
            mainccubuf.cmdType=0x01;
            mainccubuf.cmd=0xA1;
            com3_tx_rsp(mainccubuf,0,0x0A);
            if(rfUsr_isRfok(&pm->uch[0])!=ERR)
            {
                char str[]={"��Ч��"};
                Syn6658_Play(str,strlen(str));
                bell_set(BELL_PLAY_SHORT2);
                //main_reportRf(EVENT_RFID_PURSE_CUT,1);
                dbg("rf_get ok");
            }
            else
            {
                char str[]={"��Ч������ˢ"};
                Syn6658_Play(str,strlen(str));
                dbg("rf_get err");
            }
            #else
            bell_set(BELL_PLAY_SHORT2);
            dbg("money:%d",money);
           #endif
            break;
        }
        case RFID_LEAVE:
        {
            dbg("rf_leave");
            mainccubuf.cmdType=0x01;
            mainccubuf.cmd=0xA1;
            com3_tx_rsp(mainccubuf,1,0x0A);
            break;
        }
        case RFID_NSF:
        {
            char str[]={"�������ֵ"};
            Syn6658_Play(str,strlen(str));
            dbg("money:%d",money);
            break;
        }
        case RFID_ERR:
        {
            char str[]={"����ˢ"};
            Syn6658_Play(str,strlen(str));
            break;
        }
        default:
            break;
    }

}


void app_modeSet(u32 mode)
{
	const T_LED_MODE TAB_LED[3] = {LED_IDLE,LED_INIT,LED_ALARM};	// 0-idle 1-init 2-debug *-work
	if(mode > 2)
	{
		Led_SetMode(LED_WORKING);
	}
	else
	{
		Led_SetMode(TAB_LED[mode]);
	}
    if(msg_sendVal(hMsgSz[MSGQ_MAIN],msgType(1,MSG_SRC_MAIN,MSG_DT_VAL,1),mode) == OS_FALSE)
    {
        dbg("%d",mode);
    }
}


void app_stateSet(u32 state)
{
  if(msg_sendVal(hMsgSz[MSGQ_MAIN],msgType(0,MSG_SRC_MAIN,MSG_DT_VAL,1),state) == OS_FALSE)
  {
      dbg("%d",state);
  }
}


void app_funcDef(Msg_t pm)
{
  dbg("msg type:0x%.8x val:0x%x",pm->type,pm->val);
}


/*-----------------------------------------------------------*/
void MainTimerFunction( void *pvParameters )
{
    tMsg_t msg;
    msg.type = msgType(0,MSG_SRC_TIMER,MSG_DT_PTR_H,0);
    if(msg_send(hMsgSz[MSGQ_MAIN],&msg) == OS_FALSE)
    {
            chk(1);
    }
}

/*****************************************************************************
 �� �� ��  : TaskMain
 ��������  : ������
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
void TaskMain( void *pvParameters )
{
    dbg("CC:%s %s",__DATE__,__TIME__);
    task_sleep(1000);
    syn6658_check();
    bell_init();
    MainTimeHandle=TimerCreate(500, MainTimerFunction);
    app_modeSet(1);
    dbg("main FreeStack:%d",OSTaskGetFreeStackSpace(htaskget(2)));
    for( ;; )
    {
        tMsg_t msg;

	    if(msg_recv(hMsgSz[MSGQ_MAIN],&msg) != OS_FALSE)
        {
            switch(msg.src)
            {
                case MSG_SRC_TIMER:
                {
                    TabAppMode[DevInfo.mode].if_timer(&msg);
                    break;
                }
                case MSG_SRC_RFID:
                {
                    TabAppMode[DevInfo.mode].if_rfid(&msg);
                    break;
                }
                case MSG_SRC_MAIN:
                {
                    if(msg.id != 0) //mode�л�
                    {
                        if(DevInfo.mode != msg.uch[0])
                        {
                            dbg("mode:%d->%d",DevInfo.mode,msg.val);
                            DevInfo.mode = msg.uch[0];
                        }
                        else
                        {
                            dbg("mode:%d",DevInfo.mode);
                        }
                        DevInfo.state = 0;
                    }
                    else//state �л�
                    {
                        if(DevInfo.state != msg.uch[0])
                        {
                            dbg("state:%d->%d",DevInfo.state,msg.val);
                            DevInfo.state = msg.uch[0];
                        }
                        else
                        {
                            dbg("state:%d",DevInfo.state);
                        }
                    }
                    TabAppMode[DevInfo.mode].if_state(&msg);
                    break;
                }
                case MSG_SRC_KEY:
                {
                    break;
                }
                default:
                {
                    dbg("msg type:0x%.8x val:0x%x",msg.type,msg.val);
                    break;
                }
            }
           mem_free(&msg);
        }
    }
}


