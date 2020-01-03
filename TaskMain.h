/******************************************************************************

                  版权所有 (C), 2015-2050, 厦门优胜卫厨有限公司

 ******************************************************************************
  文 件 名   : TaskMain.h
  版 本 号   : 初稿
  作    者   : zgj
  生成日期   : 2019年11月14日
  最近修改   :
  功能描述   : 主任务头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年11月14日
    作    者   : zgj
    修改内容   : 创建文件

******************************************************************************/
#include "config.h"
#include "osif.h"
#include "dbg.h"


typedef struct
{
	u8 mode;	// 模式: 0-空闲 1-初始化 2-调试 ...
	u8 state;	// 状态
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

