/******************************************************************************

                  版权所有 (C), 2015-2050, 厦门优胜卫厨有限公司

 ******************************************************************************
  文 件 名   : osIf.h
  版 本 号   : 初稿
  作    者   : zgj
  生成日期   : 2019年11月8日
  最近修改   :
  功能描述   : os接口函数文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年11月8日
    作    者   : zgj
    修改内容   : 创建文件

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

// 任务 接口函数
typedef	OSTaskHandle_t		TaskHandle_t;
#define	task_sleep(ms)		OSTaskSleep(ms)     //任务睡眠

#define	task_suspend(h)		OSTaskSuspend(h)    //任务延缓

#define	task_resume(h)		OSTaskResume(h)     //任务恢复

//#define	isr_disable()		FitEnterCritical()

//#define isr_enable()		FitExitCritical()

#define	task_create(func,para,stack,priority,name)	OSTaskCreate(func,para,stack,priority,name) //任务创建

// 内存 接口函数
#define mem_malloc(size)	OSMemMalloc(size)
#define mem_free(ptr)		OSMemFree(ptr)
//#define mem_free(ptr);		{dbg("free:%P",ptr);OSMemFree(ptr);}

// 互斥 接口函数
typedef	OSMutexHandle_t		MutexHandle_t;
#define	mutex_create()		OSMutexCreate()
#define	mutex_lock(h)		OSMutexLock(h,PEND_FOREVER)
#define	mutex_unlock(h)		OSMutexUnlock(h)


// 信号量 接口函数
typedef	OSSemHandle_t		SemHandle_t;

#define	sem_pend(h)			OSSemPend(h,PEND_FOREVER)
#define	sem_post(h)			OSSemPost(h)
#define	sem_postIsr(h)		OSSemPostFromISR(h)
#define	sem_create(xC,iC)	OSSemCreateCount(xC,iC)

//定时器接口函数

typedef OSTimerHandle_t			TimerHandle_t;

OSTimerHandle_t TimerCreate(u32 Ticks,OSTimerFunction_t Func);

uOSBool_t TimerStart(TimerHandle_t TimerHandle,u32 ticks);

#define  TimerStop(h)           OSTimerStop(h)


// 消息 接口函数
//#pragma anon_unions

typedef struct {
	union {	// 消息组合头
		struct {
			unsigned char	id;		// 消息类型: 由接收任务定义
			unsigned char	src;	// 消息来源: 0-invalid other-按任务定义映射; 发给TaskMain任务例外 MAIN_MSG_SRC
			unsigned char	dt;		// 数据类型: 0-val  1-ptr need free  2-ptr no free 3-ch[]
			unsigned char	len;	// 指针数据长度或ch长度(仅指用的部分，无需指示可赋0)
		};
		int		type;
	};
	union {	// 附带数据
		int		val;
		u32		u;		// u=0x01020304
		i32		i;
		u16		ush[2];
		i16		sh[2];
		u8		uch[4];	// ch[0]=0x04 ch[1]=03
		i8		ch[4];
		void * 	ptr;	// 指针数据结构，消息通信双方自行约定
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
	MSG_DT_VAL = 0,	// 单值
	MSG_DT_PTR_M,	// 需要释放的指针
	MSG_DT_PTR_H,	// 无需释放的指针
	MSG_DT_CH,	// 当ch[4]数组使用
	MSG_DT_MAX
};



// TaskRFID
enum {
	EVENT_RFID_TIMER = 0,
	EVENT_RFID_CHKCARD,	/* 检测到卡 */
	EVENT_RFID_RDBLK,	/* 读块数据 */
	EVENT_RFID_WRBLK,	/* 写块数据 */

	//EVENT_RFID_SETVAL,   /* 设置值块的值 */
	//EVENT_RFID_GETVAL,   /* 获取值块的值 */
    //EVENT_RFID_VALCTR,   /*值块加减操作*/

	EVENT_RFID_PURSE_INIT,   /* 钱包初始化 */
    EVENT_RFID_PURSE_PAY,     //钱包充值
    EVENT_RFID_PURSE_CUT,     //钱包扣款
    EVENT_RFID_PURSE_BLANCE,  //钱包余额

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


#define MSGQ_DEF_LENGTH	10   //消息队列
#define MSGQ_DEF_SIZE	30   //每个消息的字节数


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
