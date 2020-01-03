#include "stm32f10x.h"
#include "TINIUX.h"
#include "config.h"
#include "console.h"
#include "TaskLoop.h"
#include "TaskMain.h"
#include "osif.h"
#include "dbg.h"
#include "com.h"


#include "TaskRfid.h"








int main()
{
    RCC_ClocksTypeDef RCC_Clocks;
    SystemInit();                                       //系统时钟初始化
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);    //将中断矢量放到Flash的0000地址
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);     //设置优先级配置的模式
    //初始化系统
    RCC_GetClocksFreq(&RCC_Clocks);
    __RESETPRIMASK();
    com_init(console_com,115200); //控制台com

    dbg("SYSCLK:%d,HCLK:%d,PCLK1:%d,PCLK2:%d",
            RCC_Clocks.SYSCLK_Frequency,
            RCC_Clocks.HCLK_Frequency,
            RCC_Clocks.PCLK1_Frequency,
            RCC_Clocks.PCLK2_Frequency
        );

    {
        TaskHandle_t          hTask       = NULL;
        //Initialize the parameter in TINIUX
        OSInit();
        // 创建消息队列
        hMsgSz_init();
    	// 创建任务
    	hTask 	= task_create(TaskConsole, NULL, 128, OSHIGHEAST_PRIORITY-1, "console");
    	chk(hTask == NULL);

    	hTask 	= task_create(TaskLoop, NULL, 128, OSHIGHEAST_PRIORITY-2, "loop");
    	chk(hTask == NULL);

    	hTask 	= task_create(TaskMain, NULL, 256, OSHIGHEAST_PRIORITY-3, "main");
    	chk(hTask == NULL);

        hTask = task_create(TaskRfid, NULL, 128, OSHIGHEAST_PRIORITY-4, "RFID");
        chk(hTask == NULL);


    	// Start the scheduler.
    	OSStart();
    	//if everything is ok, can't reach here
    	for( ;; );
	}
}








