/******************************************************************************

                  ��Ȩ���� (C), 2015-2050, ������ʤ�������޹�˾

 ******************************************************************************
  �� �� ��   : osIf.h
  �� �� ��   : ����
  ��    ��   : zgj
  ��������   : 2019��11��8��
  ����޸�   :
  ��������   : os�ӿں����ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��11��8��
    ��    ��   : zgj
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __OS_IF_H
#define __OS_IF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "TINIUX.h"


#ifndef NULL
  #define	NULL			( OS_NULL )
#endif


#define sys_ticks()			OSGetTickCount()
#define tick_timeout(x)		((int)(sys_ticks() - x) > 0)



#define PEND_FOREVER	OSPEND_FOREVER_VALUE

// ���� �ӿں���
typedef	OSTaskHandle_t		TaskHandle_t;
#define	task_sleep(ms)		OSTaskSleep(ms)     //����˯��

#define	task_suspend(h)		OSTaskSuspend(h)    //�����ӻ�

#define	task_resume(h)		OSTaskResume(h)     //����ָ�

//#define	isr_disable()		FitEnterCritical()

//#define isr_enable()		FitExitCritical()

#define	task_create(func,para,stack,priority,name)	OSTaskCreate(func,para,stack,priority,name) //���񴴽�

// �ڴ� �ӿں���
#define mem_malloc(size)	OSMemMalloc(size)
#define mem_free(ptr)		OSMemFree(ptr)
//#define mem_free(ptr);		{dbg("free:%P",ptr);OSMemFree(ptr);}

// ���� �ӿں���
typedef	OSMutexHandle_t		MutexHandle_t;
#define	mutex_create()		OSMutexCreate()
#define	mutex_lock(h)		OSMutexLock(h,PEND_FOREVER)
#define	mutex_unlock(h)		OSMutexUnlock(h)


// �ź��� �ӿں���
typedef	OSSemHandle_t		SemHandle_t;

#define	sem_pend(h)			OSSemPend(h,PEND_FOREVER)
#define	sem_post(h)			OSSemPost(h)
#define	sem_postIsr(h)		OSSemPostFromISR(h)
#define	sem_create(xC,iC)	OSSemCreateCount(xC,iC)

//��ʱ���ӿں���

typedef OSTimerHandle_t			TimerHandle_t;

OSTimerHandle_t TimerCreate(u32 Ticks,OSTimerFunction_t Func);

uOSBool_t TimerStart(TimerHandle_t TimerHandle,u32 ticks);

#define  TimerStop(h)           OSTimerStop(h)


// ��Ϣ �ӿں���
//#pragma anon_unions

typedef struct {
	union {	// ��Ϣ���ͷ
		struct {
			unsigned char	id;		// ��Ϣ����: �ɽ���������
			unsigned char	src;	// ��Ϣ��Դ: 0-invalid other-��������ӳ��; ����TaskMain�������� MAIN_MSG_SRC
			unsigned char	dt;		// ��������: 0-val  1-ptr need free  2-ptr no free 3-ch[]
			unsigned char	len;	// ָ�����ݳ��Ȼ�ch����(��ָ�õĲ��֣�����ָʾ�ɸ�0)
		};
		int		type;
	};
	union {	// ��������
		int		val;
		u32		u;		// u=0x01020304
		i32		i;
		u16		ush[2];
		i16		sh[2];
		u8		uch[4];	// ch[0]=0x04 ch[1]=03
		i8		ch[4];
		void * 	ptr;	// ָ�����ݽṹ����Ϣͨ��˫������Լ��
	};
} tMsg_t;
typedef tMsg_t *	Msg_t;

typedef	OSMsgQHandle_t			MsgHandle_t;


typedef enum {
	MSGQ_RFID = 0,
	MSGQ_MAIN,
	MSGQ_MAX
} MSGQ_TASK;

typedef enum {
	MSG_SRC_TIMER = 0, //for tMsg_t.src
	MSG_SRC_RFID,
	MSG_SRC_CONSOLE,
	MSG_SRC_MAIN,
	MSG_SRC_KEY,
	MSG_SRC_MAX
} MAIN_MSG_SRC;

enum {	// for tMsg_t.dt
	MSG_DT_VAL = 0,	// ��ֵ
	MSG_DT_PTR_M,	// ��Ҫ�ͷŵ�ָ��
	MSG_DT_PTR_H,	// �����ͷŵ�ָ��
	MSG_DT_CH,	// ��ch[4]����ʹ��
	MSG_DT_MAX
};



// TaskRFID
enum {
	EVENT_RFID_TIMER = 0,
	EVENT_RFID_CHKCARD,	/* ��⵽�� */
	EVENT_RFID_RDBLK,	/* �������� */
	EVENT_RFID_WRBLK,	/* д������ */

	//EVENT_RFID_SETVAL,   /* ����ֵ���ֵ */
	//EVENT_RFID_GETVAL,   /* ��ȡֵ���ֵ */
    //EVENT_RFID_VALCTR,   /*ֵ��Ӽ�����*/

	EVENT_RFID_PURSE_INIT,   /* Ǯ����ʼ�� */
    EVENT_RFID_PURSE_PAY,     //Ǯ����ֵ
    EVENT_RFID_PURSE_CUT,     //Ǯ���ۿ�
    EVENT_RFID_PURSE_BLANCE,  //Ǯ�����

	EVENT_RFID_MAX
};

// TaskRFID
enum {
    RFID_GET = 0,
    RFID_LEAVE,
    RFID_NSF,
    RFID_ERR,
    RFID_MAX
};


#define MSGQ_DEF_LENGTH	10   //��Ϣ����
#define MSGQ_DEF_SIZE	30   //ÿ����Ϣ���ֽ���


extern MsgHandle_t hMsgSz[MSGQ_MAX];



#define msgType(id,src,dt,len)		(((u32)len<<24)|((u32)dt<<16)|((u32)src<<8)|(u32)id)



sOSBase_t msg_sendVal( MsgHandle_t MsgQHandle, int type, int val );
uOSBool_t msg_send( MsgHandle_t MsgQHandle, Msg_t pMsg);
sOSBase_t msg_sendIsr( MsgHandle_t MsgQHandle, Msg_t pMsg);
uOSBool_t msg_recv( MsgHandle_t MsgQHandle, Msg_t pMsg);

#define msg_recvTime(h,msg,t)	 OSMsgQReceive(h,msg,t)
#define msg_create(len,size)	OSMsgQCreate(len,size)

extern void hMsgSz_init(void);
extern TaskHandle_t htaskget(u8 num);

#ifdef __cplusplus
}
#endif

#endif //__OS_TIMER_HPP
