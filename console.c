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
 �� �� ��  : console_mainMenu
 ��������  : ���˵�
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��2��9��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void console_mainMenu(void)
{
	printf("\r\n\t cs menu:\r\n");
	printf("1,softtime test\r\n");
}
/*****************************************************************************
 �� �� ��  : console_main
 ��������  : ���˵�����
 �������  : char * buf
             int len
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��2��9��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
 �� �� ��  : soft_timeMenu
 ��������  : soft_time�˵�����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��7��26��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
 �� �� ��  : cs_softtimetest
 ��������  : softtime��������
 �������  : char * buf
             int len
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��2��9��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
 �� �� ��  : val_getPara
 ��������  : �ַ���ת�������麯��
 �������  : char *string  Ҫת�����ַ���
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��2��6��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
       else if(*string == 0x0D) //�س���
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
	if((console_uart->SR & USART_SR_RXNE) != 0) //0������û���յ���1���յ����ݣ����Զ�����
	{
		/* Receive data & clear flag */
		USART_ClearFlag(console_uart, USART_FLAG_RXNE);
		comBuf[console_com].rx.buf[comBuf[console_com].rx.in++] = (u8)(console_uart->DR);
	    sem_postIsr(csSem);
	}
	else if((console_uart->SR & USART_SR_TXE) != 0)//0�����ݻ�û�б�ת�Ƶ���λ�Ĵ�����1�������Ѿ���ת�Ƶ���λ�Ĵ�����
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
 �� �� ��  : TaskConsole
 ��������  : ����̨����
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
                {   // ����0D
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










