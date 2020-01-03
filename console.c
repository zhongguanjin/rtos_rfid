#include "console.h"
#include "delay.h"
#include "com.h"
#include "mycfg.h"
#include "osif.h"
#include "dbg.h"

int console_main(char * buf, int len);
void console_mainMenu(void);





uint8 val_getPara(uint8 *cp,char *string);

int cs_softtimetest(char * buf, int len);
void soft_timeMenu(void);


consoleCallback console_cb = NULL;

SemHandle_t csSem		= NULL;




/*****************************************************************************
 函 数 名  : console_mainMenu
 功能描述  : 主菜单
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年2月9日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void console_mainMenu(void)
{
	printf("\r\n\t cs menu:\r\n");
	printf("1,softtime test\r\n");
}
/*****************************************************************************
 函 数 名  : console_main
 功能描述  : 主菜单驱动
 输入参数  : char * buf
             int len
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年2月9日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
int console_main(char * buf, int len)
{
	if(memcmp(buf,"reboot",6) == 0)
	{
		dbg("reboot");
		delay_ms(1000);
		NVIC_SystemReset();
	}
   	switch(buf[0])
	{
		case '1':
		{
		    //soft_timeMenu();
		    break;
		}
		default:
		{
            return 1;
		}
    }
	return 0;
}
/*****************************************************************************
 函 数 名  : soft_timeMenu
 功能描述  : soft_time菜单函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年7月26日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void soft_timeMenu(void)
{
    console_cb = cs_softtimetest;
	printf("\r\n\t softtime menu:\r\n");
	printf("creat timer:1,arg\r\n");
	printf("kill timer: 2,arg\r\n");
	printf("reset timer:3,arg\r\n");
}
/*****************************************************************************
 函 数 名  : cs_softtimetest
 功能描述  : softtime测试驱动
 输入参数  : char * buf
             int len
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年2月9日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
int cs_softtimetest(char * buf, int len)
{
	union {
		uint8 u[2];
		struct {
			uint8		FUNC;
			uint8		MID;
		};
	} para;
    uint8 i=0;
    i=val_getPara(para.u,buf);
    dbg("%d,%d",para.FUNC,para.MID);
    if(i>3)
    {
        dbg("para err");
        return 1;
    }
    switch(para.FUNC)
    {
        case 1:
        {

            break;
        }
        case 2:
        {

            break;
        }
        case 3:
        {

            break;
        }
		default:
		{
            return 1;
		}
    }
    memset(para.u,0,sizeof(para));
    soft_timeMenu();
    return 0;
}


/*****************************************************************************
 函 数 名  : val_getPara
 功能描述  : 字符串转整形数组函数
 输入参数  : char *string  要转化的字符串
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年2月6日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
uint8 val_getPara(uint8 *cp,char *string)
{
    uint8 i=0,j=0,k=0;
    while(*string)
    {
       if(*string >= '0'&&*string <= '9')
       {
           k++;
           if(k==1)
           {
             cp[i]= *string-'0';
           }
           else if(k>=2)
           {
              cp[i]= cp[i]*10+(*string-'0');
           }
       }
       else if(*string==','||*string==' ')
       {
           k=0;
           i++;
           j++;
       }
       else if(*string == 0x0D) //回车符
       {
          j++;
          break;
       }
       string++;
    }
    return j;
}

void USART1_IRQHandler(void)
{
	if((console_uart->SR & USART_SR_RXNE) != 0) //0：数据没有收到；1：收到数据，可以读出。
	{
		/* Receive data & clear flag */
		USART_ClearFlag(console_uart, USART_FLAG_RXNE);
		comBuf[console_com].rx.buf[comBuf[console_com].rx.in++] = (u8)(console_uart->DR);
	    sem_postIsr(csSem);
	}
	else if((console_uart->SR & USART_SR_TXE) != 0)//0：数据还没有被转移到移位寄存器；1：数据已经被转移到移位寄存器。
	{
	    USART_ClearFlag(console_uart, USART_FLAG_TXE);
		if(comBuf[console_com].tx.in != comBuf[console_com].tx.out)
		{
			console_uart->DR = (u16)(comBuf[console_com].tx.buf[comBuf[console_com].tx.out++]);
		}
		else
		{
			USART_ITConfig(console_uart, USART_IT_TXE, DISABLE);
		}
	}
	else
	{
	    USART_ClearFlag(console_uart, USART_FLAG_FE|USART_FLAG_ORE);
	}

}


/*****************************************************************************
 函 数 名  : TaskConsole
 功能描述  : 控制台任务
 输入参数  : void *pvParameters
 输出参数  : 无
 返 回 值  : static
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年11月11日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void TaskConsole( void *pvParameters )
{
	csSem 	= sem_create(100,0);
	dbg("CC:%s %s",__DATE__,__TIME__);
	task_sleep(100);
    for(;;)
    {
        static char buf[256];
        static int len=0;
        char ch;
        sem_pend(csSem);
    	if(OK == com_getch(COM1,&ch))
    	{
            buf[len++] = ch;
            if(ch < 0x20)
            {
                if(len != 0)
                {   // 包含0D
                    buf[len] = 0;
                    if(console_cb == NULL)
                    {
                        console_cb = console_main;
                    }
                    if(console_cb(buf,len) != 0)
                    {
                        console_cb = NULL;
                        console_mainMenu();
                    }
                }
                len = 0;
            }
            else
            {
                //printf("%c\r\n",ch);
            }
    	}
    }

}










