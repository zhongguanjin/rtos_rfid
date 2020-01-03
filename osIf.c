
#include "osIF.h"
#include "dbg.h"


MsgHandle_t hMsgSz[MSGQ_MAX];

uOSBool_t msg_send( OSMsgQHandle_t MsgQHandle, Msg_t pMsg)
{
    return OSMsgQSend( MsgQHandle, pMsg, 0);
}

sOSBase_t msg_sendVal( MsgHandle_t MsgQHandle, int type, int val)
{
    tMsg_t msg;
    msg.type =type;
    msg.ptr = (int *)val;
    return OSMsgQSend( MsgQHandle, &msg, 0);
}

uOSBool_t msg_recv( OSMsgQHandle_t MsgQHandle, Msg_t pMsg)
{
     return OSMsgQReceive( MsgQHandle, pMsg, PEND_FOREVER);
}


sOSBase_t msg_sendIsr( MsgHandle_t MsgQHandle, Msg_t pMsg)
{
    return OSMsgQSendFromISR( MsgQHandle, pMsg);
}


OSTimerHandle_t TimerCreate(u32 Ticks, OSTimerFunction_t Func)
{
    return OSTimerCreate( Ticks, OS_FALSE, Func, NULL, "timefunc");   //一次性定时器
}

uOSBool_t TimerStart(TimerHandle_t TimerHandle,u32 Ticks)
{
    uOSBool_t sta=OS_FALSE;
    sta=OSTimerSetTicks(TimerHandle, Ticks);
    if(sta != OS_FALSE)
    {
      sta= OSTimerStart(TimerHandle);
    }
    return sta;
}



void hMsgSz_init(void)
{
	int i;
	for(i=0;i<MSGQ_MAX;i++)
	{
		hMsgSz[i] = msg_create(MSGQ_DEF_LENGTH,MSGQ_DEF_SIZE);
		chk(hMsgSz[i]==NULL);
	}
}

