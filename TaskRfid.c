/******************************************************************************

                  ��Ȩ���� (C), 2015-2050, ������ʤ�������޹�˾

 ******************************************************************************
  �� �� ��   : TaskRfid.c
  �� �� ��   : ����
  ��    ��   : zgj
  ��������   : 2019��11��17��
  ����޸�   :
  ��������   : RFIDԴ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��11��17��
    ��    ��   : zgj
    �޸�����   : �����ļ�

******************************************************************************/

#include "TaskRfid.h"
#include "osif.h"
#include "dbg.h"
#include "delay.h"
#include "eeprom.h"
#include "Syn6658.h"




TimerHandle_t   RfidTimeHandle	= NULL;
void RfidTimerFunction( void *pvParameters );

const uint8 C1E_info[]={0x60,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	// for key1
const uint8 C2N_info[]={0x03,0x03,0x26,0x45,0x60,0x01,0x08};	// def. key1 for blk1
const uint8 C2M_info[]={0x00,0x26};
const uint8  C2O_info[] ={0x00};

rfMux_t rfMux;

T_RFUSER grfUser; /*�û���Ϣ*/

void rf_reInitRx(uint8 mode);
int16 rf_sendCmd(uint32 cmdHead, const uint8 * info);
uint8 rf_init(void);
uint8 rf_wait( void );

void rf_over_check(void);


/*�жϿ��Ƿ���Ч������1��Ч��0��Ч*/
int rfUsr_isRfok(u8 *buf)
{
	int loop=grfUser.cnt;
	while(loop--)
	{
		if(memcmp(buf, (u8 *)&grfUser.serial[loop][0], 4) == NULL)
		{/*�ҵ���ͬ����*/
			return 1;
		}
	}
	return 0;
}

/*����1����*/
int rfUsr_isRfFull(void)
{
	if(grfUser.cnt>=RFUSER_MAX)
	{
	    dbg("user full");
		return 1;
	}
	return 0;
}
/*����1����*/
int rfUsr_isRFEmpty(void)
{
	if(grfUser.cnt==0)
	{
	    dbg("user empty");
		return 1;
	}
	return 0;
}


/*����һ��������*/
int rfUsr_append(u8 *buf)
{
    if(rfUsr_isRfFull()==OK)
    {
        return 0;
    }
	memcpy((u8 *)(&grfUser.serial[grfUser.cnt][0]), buf, 4);
	grfUser.cnt++;
	grfUser.crc =0xff;
	EEPROM_Write(ADDR_RFUSER, (u8 *)(&grfUser),sizeof(T_RFUSER));
	dbg("save rf serial[%d]",grfUser.cnt);
	return 1;
}

/*ɾ����*/
int rfUsr_pop(u8 *buf)
{
	int loop=grfUser.cnt;
	while(loop--)
	{
		if(memcmp(buf, (u8 *)&grfUser.serial[loop][0], 4) == NULL)
		{
		    /*�ҵ���ͬ����*/
		    memcpy((u8 *)(&grfUser.serial[loop][0]), 0, 4);
            grfUser.cnt--;
            grfUser.crc =0xff;
            EEPROM_Write(ADDR_RFUSER, (u8 *)(&grfUser),sizeof(T_RFUSER));
            dbg("save rf serial[%d]",grfUser.cnt);
			return 1;
		}
	}
	return 0;
}
/*****************************************************************************
 �� �� ��  : rfUsr_showRfSerial
 ��������  : �鿴�����û���ʹ������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2019��11��17��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void rfUsr_showRfSerial(void)
{
	int loop=0;
	dbg("crc::%04X",grfUser.crc);
	while(loop < grfUser.cnt)
	{
		dbg("id:%d, serial:%02X %02X %02X %02X",loop,
		grfUser.serial[loop][0],grfUser.serial[loop][1],grfUser.serial[loop][2],grfUser.serial[loop][3]);
		loop++;
	}
	return;
}
/*****************************************************************************
 �� �� ��  : rfUsr_setDefault
 ��������  : �û�RFʹ�����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2019��11��17��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void rfUsr_setDefault(void)
{
	memset((u8 *)(&grfUser),0, sizeof(T_RFUSER));
	grfUser.crc = 0xff;
	EEPROM_Write(ADDR_RFUSER, (u8 *)(&grfUser),sizeof(T_RFUSER));
	return;
}
/*****************************************************************************
 �� �� ��  : rfUsr_init
 ��������  : �û�RFID����ʼ��
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2019��11��17��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void rfUsr_init(void)
{
	EEPROM_Read(ADDR_RFUSER,(u8 *)(&grfUser),sizeof(T_RFUSER));
	dbg("--rf user cnt::%d---",grfUser.cnt);
	if(rfUsr_isRfFull()==OK)
    {
        dbg("cnt fail");
		rfUsr_setDefault();
        return ;
    }
	{
		u16 crc = 0xff;
		if(grfUser.crc != crc)
		{
			dbg("crc fail:0x%04X != 0x%04X",crc,grfUser.crc);
			rfUsr_setDefault();
		}
		else
		{
		    rfUsr_showRfSerial();
		}
	}
	return;
}

/*****************************************************************************
 �� �� ��  : rf_bccCalc
 ��������  : У���뺯��
 �������  : uint8 * buf
             uint8 len
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��7��11�� ���ڶ�
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8 rf_bccCalc(uint8 * buf,uint8 len)
{
	uint8 bcc = 0;
	while(len--)
	{
		bcc ^= *buf++;
	}
	return ~bcc;
}

void rf_reInitRx(uint8 mode)
{
	rfMux.rIdx = 0;
	rfMux.frameOK = 0;
	rfMux.mode = mode;
}

uint8 rf_wait( void )
{
    uint32 rfRxTicks = sys_ticks() + 2000;
	while(1)
	{
		delay_ms(5);
        if(rfMux.frameOK == 1)
        {
        	//printf("rfMux.frameOK:%d\r\n",rfMux.frameOK);
            return OK;
        }
        if(tick_timeout(rfRxTicks))
        {
           dbg("rfMux.frame_err");
           return ERR;
        }
	}
}

/*****************************************************************************
 �� �� ��  : rf_sendCmd
 ��������  : RF��������֡����
 �������  : u32 cmdHead
             const u8 * info
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��7��11�� ���ڶ�
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
int16 rf_sendCmd(uint32 cmdHead, const uint8 * info)
{
	rfPkt_t rfSend;
	int16 len;
	rfSend.cmdHead = cmdHead;
	len = (uint32)(rfSend.infoLen);
	if((len != 0) && (info != NULL))
	{
		memcpy(rfSend.info,info,len);
	}
	//֡�����벻С��6 �ֽڣ�����ܳ���70 �ֽڣ���֡�����������Ϣ���ȼ�6��
	rfSend.frameLen = rfSend.infoLen + 6;
	//У��ͣ���FrameLen ��ʼ��Info �����һ�ֽ����ȡ��
	rfSend.info[len++] = rf_bccCalc((uint8 *)&rfSend,(rfSend.infoLen+4));
	rfSend.info[len++] = 0x03;
	len += 4;
	rf_reInitRx(0);  //��������ģʽ
	com_send(rfid_com,(uint8 *)&rfSend,len); //��������֡����
	//for recv
    if(rf_wait() == OK)
    {
        if(rfMux.rxBuf[2] == 0)  //״̬�ɹ�
        {	// �ɹ�
        	return OK;
        }
        else
        {
        	//dbg("rsp status: %x\r\n",rfMux.rxBuf[2]);
        	return ERR;
        }
    }
    else
    {
        dbg("overtime\r\n");
        return ERR;
    }

}
/*****************************************************************************
 �� �� ��  : rf_init
 ��������  : RFID��ʼ������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��7��13�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8 rf_init(void)
{
    uint8 times = 1;
	memset(&rfMux,0,sizeof(rfMux));
	while(times-- != 0)
	{	// �������ո񣬳�ʼ������ͨ��
        com_send(rfid_com," ",1);
		delay_us(40); //40us
		com_send(rfid_com," ",1);
		delay_ms(20);//20ms
		if(rfMux.rxBuf[0] == 0x06)
		{
		    dbg("rfMux.rxBuf[0]: %d",rfMux.rxBuf[0]);
		    break;
		}
		delay_ms(1000);
	}
    times = 2;
	while(times-- != 0)
	{
		if(rf_sendCmd(RF_HEAD_C1A,NULL) == OK)
		{
			if(rf_sendCmd(RF_HEAD_C1E1,C1E_info) == OK)
			{
			    dbg("RF_HEAD_C1E1 ok");
				return OK;
			}
			else
			{
			     dbg("RF_HEAD_C1E1 err");
			     return ERR;
			}
		}
		else
		{
		     dbg("RF_HEAD_C1A err");
		     return ERR;
		}
	}
	return ERR;
}

void rf_over_check(void)
{
	rf_reInitRx(0);
    if( rf_init() == OK)
    {
        /* ��⵽ģ�� �����Զ����ģʽ */
        if(rf_sendCmd(RF_HEAD_C2N,C2N_info) == OK)
        //���������ڿ�Ƭ���Զ���⣬ִ�и�����ɹ�����UART ģʽ�£�ģ�齫��������
        //��ȡ����Ƭ������
        {
            dbg("RF_HEAD_C2N ok");
            rf_reInitRx(1);
        }
        else
        {    /* ���RFģ����� */
            dbg("RF_HEAD_C2N err");
        }

    }
    else
    {
        dbg("rf_init err");
    }
}


void rf_init_check(void)
{
	rf_reInitRx(0);
    if( rf_init() == OK)
    {
        /* ��⵽ģ�� �����Զ����ģʽ */
        if(rf_sendCmd(RF_HEAD_C2N,C2N_info) == OK)
        //���������ڿ�Ƭ���Զ���⣬ִ�и�����ɹ�����UART ģʽ�£�ģ�齫��������
        //��ȡ����Ƭ������
        {
            dbg("RF_HEAD_C2N ok");
            rfUsr_init();
            rf_reInitRx(1);
        }
        else
        {    /* ���RFģ����� */
            dbg("RF_HEAD_C2N err");
        }
    }
    else
    {
        dbg("rf_init err");
    }
}

/*****************************************************************************
 �� �� ��  : get_rf_uid
 ��������  : ������uid
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
uint32 get_rf_uid(void)
{
    return rfMux.devInfo.uid.u;
}

/*****************************************************************************
 �� �� ��  : read_rf_dat
 ��������  : ��rfid��ŵ�ֵ����
 �������  : uint8 blank
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��7��26��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void read_rf_dat(uint8 blank)
{
    rfC2H_t info_w;
    rfC2E_t info;
    if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))
    {
        info.mode = 0x60;                           //0x60������ԿA
        memcpy(info.uid,rfMux.devInfo.uid.uch,4);   //�����кţ�4 �ֽڣ�
        info.keyid = 1;                             //��Կ���ţ�1 �ֽڣ��� ȡֵ��Χ0��7
        info.blkid = blank;
        /*����ţ�1 �ֽڣ���
            S50��0��63��
            S70��0��255��
            PLUS CPU 2K��0��127��
            PLUS CPU 4K��0��255��
        */
        if(OK == rf_sendCmd(RF_HEAD_C2E,(uint8 *)&info)) //��Կ��֤
        {
            info_w.blkid = blank;
            if(OK == rf_sendCmd(RF_HEAD_C2G,(uint8 *)&info_w.blkid))
            {
                dbg("info:");
                dbg_hex(rfMux.rPkt.info,16);
            }
            else
            {
                dbg("read err!");
            }
        }
        else
        {
            dbg("password err ");
        }
    }
    else
    {
         dbg("no card !");
    }
}



/*****************************************************************************
 �� �� ��  : rf_reportMain
 ��������  : �ϱ���������
 �������  : u32 id
             u32 rfid
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2019��11��19��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void rf_reportMain(u32 id, u32 rfid)
{
	u32 type = msgType(id,MSG_SRC_RFID,MSG_DT_CH,4);
	if(msg_sendVal(hMsgSz[MSGQ_MAIN],type,rfid) == OS_FALSE)
	{
		chk(1);
	}
}


void com4_rxDeal()
{
	char ch;
	while(1)
	{
        if(OK == com_getch(rfid_com,(char *)&ch))
        {
            rfMux.rxBuf[rfMux.rIdx++] = ch;
            if ((rfMux.rxBuf[rfMux.rIdx-1] == 0x03) && (rfMux.rIdx == rfMux.rPkt.frameLen))
            {
                //�յ��������ݣ�������������
                rfMux.rIdx -= 2;    // ָ��bcc
                if(rf_bccCalc(rfMux.rxBuf,rfMux.rIdx) == rfMux.rxBuf[rfMux.rIdx])
                {
                     rfMux.frameOK =1; //������һ֡����
                    if(rfMux.mode == 1)
                    {   /* �����ϱ�����⵽ˢ�� */
                        if(msg_sendVal(hMsgSz[MSGQ_RFID],msgType(EVENT_RFID_CHKCARD, 0, 0, 0),0) == OS_FALSE)
                        {
                            chk(1);
                        }
                    }
                }
                else
                {
                    dbg("bcc err!");
                }
            }
        }
        else
        {
            break;
        }

	}
}

/*-----------------------------------------------------------*/
void RfidTimerFunction( void *pvParameters )
{
    tMsg_t msg;
    msg.type = msgType(EVENT_RFID_TIMER,MSG_SRC_RFID,0,0);
    if(msg_send(hMsgSz[MSGQ_RFID],&msg) == OS_FALSE)
    {
            chk(1);
    }
}



void TaskRfid(void *pvParameters)
{
	task_sleep(1000);
	dbg("CC:%s %s",__DATE__,__TIME__);
	I2C_EE_Config();
	rf_init_check();  //RFID����ʼ����
	RfidTimeHandle = TimerCreate(200, RfidTimerFunction);
	for( ;; )
	{
		tMsg_t msg;
		if(msg_recv(hMsgSz[MSGQ_RFID],&msg) != OS_FALSE)
		{
			switch(msg.id)
			{
				case EVENT_RFID_TIMER:
				{
                    if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))
                    {
                       TimerStart(RfidTimeHandle,250);
                    }
                    else
                    {
                        rfMux.status = 0;
                        // �����ߣ��ٴν����Զ�ģʽ
                        if(OK == rf_sendCmd(RF_HEAD_C2N,C2N_info))
                        {
                            rf_reInitRx(1);
                            rf_reportMain(RFID_LEAVE,rfMux.devInfo.uid.u);
                        }
                    }
					break;
				}
				case EVENT_RFID_CHKCARD:
				{
                    if(rfMux.rPkt.infoLen == 0x19)
                    {
                        memcpy(&(rfMux.devInfo),rfMux.rxBuf+9,20);
                        dbg("uid: 0x%X",rfMux.devInfo.uid.u);
                        rfMux.status = 1;
						/* ��ʱ��ѯ���Ƿ��뿪 */
						TimerStart(RfidTimeHandle,250);
                        rf_reportMain(RFID_GET,rfMux.devInfo.uid.u);
                    }
					break;
				}
				case EVENT_RFID_RDBLK:
				{	/* �������ݿ� */

					break;
				}
				case EVENT_RFID_WRBLK:
				{	/* д�����ݿ� */

					break;
				}
				default:
				{
					dbg("msg rcv:\r\n");
					dbg_hex((char *)&msg,sizeof(tMsg_t));
					break;
				}
			}
			mem_free(&msg);
		}
	}
}

