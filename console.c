#include "console.h"
#include "delay.h"
#include "com.h"
#include "mycfg.h"
#include "osif.h"
#include "dbg.h"

#include "TaskRfid.h"
#include "eeprom.h"

int console_main(char * buf, int len);
void console_mainMenu(void);
int cs_eepromtest(char * buf, int len);
void eeprom_Menu(void);





uint8 val_getPara(uint8 *cp,char *string);

int cs_rfmifaretest(char * buf, int len);
void RfMifareMenu(void);


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
	printf("1,rf mifare test\r\n");
	printf("2,eeprom test\r\n");
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
		    RfMifareMenu();
		    break;
		}
		case '2':
		{
		    eeprom_Menu();
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
 �� �� ��  : RfMifareMenu
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
void RfMifareMenu(void)
{
    console_cb = cs_rfmifaretest;
	printf("\r\n\t rf mifare menu:\r\n");

	printf("read rf blk:1,blk\r\n");
	printf("write rf blk:2,blk\r\n");

    printf("purse init:3,money\r\n");
    printf("purse pay:4,money\r\n");
    printf("purse cut:5,money\r\n");
    printf("purse balance:6,blk\r\n");

    printf("c:add rfid user\r\n");
    printf("d:del rfid user\r\n");
    printf("e:query rfid user\r\n");

}
/*****************************************************************************
 �� �� ��  : eeprom_Menu
 ��������  : eeprom�˵�����
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��8��15��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void eeprom_Menu(void)
{
    console_cb = cs_eepromtest;
	printf("\r\n\t eeprom menu:\r\n");
	printf("read:1,addr,len \r\n");
	printf("write:2,addr,len \r\n");
	printf("addr:eeprom address,len:byte len\r\n");
}


/*****************************************************************************
 �� �� ��  : cs_eepromtest
 ��������  : eeprom��������
 �������  : char * buf
             int len
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��8��15��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
int cs_eepromtest(char * buf, int len)
{
	union {
		uint8 u[3];
		struct {
		    uint8       fun;
			uint8		addr;
			uint8		len;
		};
	} para;
    uint8 i=0;
    i=val_getPara(para.u,buf);
    dbg("%d,%d,%d",para.fun,para.addr,para.len);
    if((i > 3 )&&( para.len > 127)&& (para.len!=0))
    {
        dbg("para err");
        return 1;
    }
    switch(para.fun)
    {
        case 1: //��
        {
			uint8 i;
            uint8 eeprom_buf[128];
            EEPROM_Read(para.addr,eeprom_buf, para.len);
            for (i=0; i<para.len; i++)
            {
                printf("0x%02X ", eeprom_buf[i]);
                if((i+1)%16 == 0)
                {
                    printf("\r\n");
                }
            }
            break;
        }
        case 2://д
        {
						uint8 i;
            uint8 eeprom_buf[128];
            for(i=0;i<para.len;i++)
            {
               eeprom_buf[i]=0xFF;
            }
            EEPROM_Write(para.addr,eeprom_buf, para.len);
            break;
        }
		default:
		{
            return 1;
		}
    }
    memset(para.u,0,sizeof(para));
    eeprom_Menu();
    return 0;

}


u8 datrry[16]={0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
                0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
/*****************************************************************************
 �� �� ��  : cs_rfmifaretest
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
int cs_rfmifaretest(char * buf, int len)
{
	union {
		uint8 u[10];
		struct {
			uint8		FUNC;
			u8          dat[9];
		};
	} para;
    uint8 i=0;
    i=val_getPara(para.u,buf);
    dbg("%s",buf);
    if(i>10)
    {
        dbg("para err");
        return 1;
    }

    switch(buf[0])
    {
        case '1':
        {
            if((para.dat[0]%4) != 3)
            {
                u32 type =msgType(EVENT_RFID_RDBLK,MSG_SRC_CONSOLE,MSG_DT_VAL,1);
                dbg("type:0x%.8x,blk:%d",type,para.dat[0]);
    			msg_sendVal(hMsgSz[MSGQ_RFID],type,para.dat[0]);
			}
			else
			{
			    dbg("err,pwd ctr blk");
			}
            break;
        }
        case '2':
        {
            if((para.dat[0]%4) != 3)
            {
            	u8 * p = mem_malloc(17);
    			*p = para.dat[0];	// for blockid//
    			memcpy(p+1,datrry,16);
    			//dbg_hex(p, 17);
                u32 type =msgType(EVENT_RFID_WRBLK,MSG_SRC_CONSOLE,MSG_DT_PTR_M,17);
                dbg("type:0x%.8x,blk:%d",type,para.dat[0]);
                msg_sendVal(hMsgSz[MSGQ_RFID],type,(u32)p);
    			mem_free(p);
			}
			else
			{
			    dbg("err,pwd ctr blk");
			}
            break;
        }
        case '3':
        {
            u32 type =msgType(EVENT_RFID_PURSE_INIT,MSG_SRC_CONSOLE,MSG_DT_CH,4);
            u32 money;
            memcpy(&money,para.dat,4);
            dbg("type:0x%.8x,money:%d",type,money);
			msg_sendVal(hMsgSz[MSGQ_RFID],type,money);
            break;
        }
        case '4':
        {
            u32 type =msgType(EVENT_RFID_PURSE_PAY,MSG_SRC_CONSOLE,MSG_DT_CH,4);
            u32 money;
            memcpy(&money,para.dat,4);
            dbg("type:0x%.8x",type);
            msg_sendVal(hMsgSz[MSGQ_RFID],type,money);
            break;
        }
        case '5':
        {
            u32 type =msgType(EVENT_RFID_PURSE_CUT,MSG_SRC_CONSOLE,MSG_DT_CH,4);
            u32 money;
            memcpy(&money,para.dat,4);
            dbg("type:0x%.8x",type);
            msg_sendVal(hMsgSz[MSGQ_RFID],type,money);
            break;
        }
        case '6':
        {
            u32 type =msgType(EVENT_RFID_PURSE_BLANCE,MSG_SRC_CONSOLE,0,0);
            dbg("type:0x%.8x",type);
            msg_sendVal(hMsgSz[MSGQ_RFID],type,0);
            break;
        }
		case 'c':
		{
		    UN32 bak_user;
		    bak_user.u = get_rf_uid();
		    if(rfUsr_append(bak_user.uch)==OK)
            {
                dbg("add user ok");
            }
            else
            {
                dbg("add user err");
            }
			break;
		}
		case 'd':
		{
		    UN32 bak_user;
		    bak_user.u = get_rf_uid();
		    if(rfUsr_pop(bak_user.uch)==OK)
            {
                dbg("del user ok");
            }
            else
            {
                dbg("del user err");
            }
			break;
		}
		case 'e':
		{
		    rfUsr_showRfSerial();
			break;
		}

		default:
		{
            return 1;
		}
    }
    memset(para.u,0,sizeof(para));
    RfMifareMenu();
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
	if((USART1->SR & USART_SR_RXNE) != 0) //0������û���յ���1���յ����ݣ����Զ�����
	{
		/* Receive data & clear flag */
		USART_ClearFlag(USART1, USART_FLAG_RXNE);
		comBuf[COM1].rx.buf[comBuf[COM1].rx.in++] = (u8)(USART1->DR);
	    sem_postIsr(csSem);
	}
	else if((USART1->SR & USART_SR_TXE) != 0)//0�����ݻ�û�б�ת�Ƶ���λ�Ĵ�����1�������Ѿ���ת�Ƶ���λ�Ĵ�����
	{
	    USART_ClearFlag(USART1, USART_FLAG_TXE);
		if(comBuf[COM1].tx.in != comBuf[COM1].tx.out)
		{
			USART1->DR = (u16)(comBuf[COM1].tx.buf[comBuf[COM1].tx.out++]);
		}
		else
		{
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		}
	}
	else
	{
	    USART_ClearFlag(USART1, USART_FLAG_FE|USART_FLAG_ORE);
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
	task_sleep(10);
	dbg("console FreeStack:%d",OSTaskGetFreeStackSpace(htaskget(0)));
    for(;;)
    {
        sem_pend(csSem);
        console_rxDeal();
    }
}



 void com1_rxDeal(void)
 {
     static char buf[256];
     static int len=0;
     char ch;
     if(OK == com_getch(console_com,&ch))
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








