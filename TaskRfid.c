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

#include "ccu.h"

ccubuf_t rfccubuf;

TimerHandle_t   RfidTimeHandle	= NULL;
void RfidTimerFunction( void *pvParameters );

/*
 ��Կ���ͣ�1 �ֽڣ���   0x60������ԿA
                        0x61������ԿB
 ��Կ���ţ�1 �ֽڣ���   ȡֵ��Χ0��15
 ��Կ��6 �ֽڻ�16 �ֽڣ�
 RF_HEAD_C1E1= 0x0845010E		// װ��IC����Կ��6λ��Կ
����Կ01 ��װ����ԿA��0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
*/

const uint8 C1E_info[]={0x60,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	// for key1

/*
 RF_HEAD_C2N = 0x074E020D        // �Զ����

 �Զ����ģʽADMode��1 �ֽڣ���0x03  ��������󲻼������,�����ж�,������������

 ����������ʽTxMode��1 �ֽڣ��� 0��TX1��TX2 ��������
                                1����TX1 ����
                                2����TX2 ����
                                3��TX1��TX2 ͬʱ����

 �������ReqCode��1 �ֽڣ���    0x26��IDLE
                                0x52��ALL

 ��֤ģʽAuthMode��1 �ֽڣ���   0x45-��E������E2 ��Կ��֤
                                0x46-��F������ֱ����Կ��֤
                                0 ������֤

 ��ԿAB KeyType��1 �ֽڣ���     0x60����ԿA
                                0x61����ԿB

 ��ԿKey��                      ����֤ģʽΪ��E������Ϊ��Կ���ţ�1 �ֽڣ�
                                ����֤ģʽΪ��F������Ϊ��Կ��6 ��16 �ֽڣ�

 �����Block��1 �ֽڣ���        S50��0��63��
                                S70��0��255��
                                PLUS CPU 2K��0��127��
                                PLUS CPU 4K��0��255��

��E2��Կ��֤��Կ����01��������8����������
*/
const uint8 C2N_info[]={0x03,0x03,0x26,0x45,0x60,0x01,0x08};

/*
 RF_HEAD_C2M= 0x024D0208		// ���������ڼ��Ƭ�������󡢷���ײ��ѡ��������������
IDLE��ʽ����
*/
const uint8 C2M_info[]={0x00,0x26};

//ͨ���ö�ȡ�Զ��������������Ծ�����ȡ���ݺ��Ƿ������⡣
const uint8  C2O_info[] ={0x00};




rfMux_t rfMux;

T_RFUSER grfUser; /*�û���Ϣ*/

void rf_reInitRx(uint8 mode);
int16 rf_sendCmd(uint32 cmdHead, const uint8 * info);
uint8 rf_init(void);
uint8 rf_wait( void );

u8 rf_goto_auto(void);
u8 rf_key_check(u8 blk);

void rf_reportMain(u32 id, u32 rfid);

/*Ǯ����ر��������� */

typedef struct
{
    u8  mode;
   	u8  blkid;
    u8  money[4];
}rfC2P_t;    //Ǯ�����ݽṹ

u8   blkdef  =0x04;
u8 money_dec(u8 blk,u32 money);
u8 money_add(u8 blk,u32 money);
u8 money_init(u8 blk, u32 money);
u8 money_balance(u8 blk);



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


/*����һ��������*/
int rfUsr_append(u8 *buf)
{
	if(grfUser.cnt>=RFUSER_MAX)
	{
	    dbg("user full");
		return 0;
	}
	int loop=grfUser.cnt;
	while(loop--)
	{
		if(memcmp(buf, (u8 *)&grfUser.serial[loop][0], 4) == NULL)
		{
		    /*�ҵ���ͬ����*/
		    dbg("user exist");
		    return 0;
		}
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
            uint8 j;
            for(j=loop; j<grfUser.cnt;j++) //��i��������Ԫ����ǰ��
            {
                memcpy((u8 *)(&grfUser.serial[j][0]), (u8 *)(&grfUser.serial[j+1][0]), 4);
            }
		    u8 sbuf[4] = {0,0,0,0};
		    memcpy((u8 *)(&grfUser.serial[grfUser.cnt][0]), sbuf, 4);
            grfUser.cnt--;
            grfUser.crc =0xff;
            EEPROM_Write(ADDR_RFUSER, (u8 *)(&grfUser),sizeof(T_RFUSER));
            dbg("del rf serial[%d]",grfUser.cnt);
			return 1;
		}
	}
	dbg("user absent");
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
		grfUser.serial[loop][3],grfUser.serial[loop][2],grfUser.serial[loop][1],grfUser.serial[loop][0]);
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
	if(grfUser.cnt>RFUSER_MAX)
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
		//dbg_hex(rfSend.info,len);
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
u8 devinfo[20];

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
        //��ȡģ��İ汾��
         if(rf_sendCmd(RF_HEAD_C1A,NULL) == OK)
         {
             memcpy(devinfo,rfMux.rPkt.info,20);
             dbg("%s",devinfo);
    	    //����Կ01 ��װ����ԿA��0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
    	    //����������ģ����װ�����룬���Ǹı�Mifare1 ��������������
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
            chk(1);
        }

	}
	return ERR;
}


/*�����Զ����ģʽ */
u8 rf_goto_auto(void)
{
    /*�����Զ����ģʽ */
    if(rf_sendCmd(RF_HEAD_C2N,C2N_info) != OK)
    //���������ڿ�Ƭ���Զ���⣬ִ�и�����ɹ�����UART ģʽ�£�ģ�齫��������
    //��ȡ����Ƭ������
    {
        return ERR;
    }
    rf_reInitRx(1);
    return OK;
}

/*rf ��Կ��֤ */
u8 rf_key_check(u8 blk)
{
    if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))
    {
        rfC2E_t *info;
        info =(rfC2E_t*)mem_malloc(sizeof(rfC2E_t));
        if(info  == NULL)
        {
            chk(1);
            return ERR;
        }
        info->mode = 0x60;

        memcpy(info->uid,rfMux.devInfo.uid.uch,4);
        info->keyid = 1;
        info->blkid = blk;
        if(OK != rf_sendCmd(RF_HEAD_C2E,&info->mode))
        {
            dbg("pwd err");
            mem_free(info);
            return ERR;
        }
        mem_free(info);
    }
    else
    {
        dbg("no card");
        rf_goto_auto();
        return ERR;
    }
    return OK;
}
void rf_init_check(void)
{
	rf_reInitRx(0);
	rfUsr_init();
    if( rf_init() == OK)
    {
        if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))//���rf card?
        {
            msg_sendVal(hMsgSz[MSGQ_RFID],msgType(EVENT_RFID_CHKCARD, 0, 0, 0),0);
        }
        else
        {
            /* �����Զ����ģʽ */
            if(rf_goto_auto() == OK)
            {
                dbg("RF_HEAD_C2N ok");
            }
            else
            {    /* ���RFģ����� */
                dbg("RF_HEAD_C2N err");
            }
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
u32 get_rf_uid(void)
{
    dbg("0x%.8X",rfMux.devInfo.uid.u);
    return rfMux.devInfo.uid.u;
}

void get_dev_info(void)
{
     dbg("%s",devinfo);
}
/*****************************************************************************
 �� �� ��  : money_init
 ��������  : Ǯ����ʽ��ʼ��
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2020��6��21��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
u8 money_init(u8 blk, u32 money)
{
    if(OK == rf_key_check(blk)) //��֤ ok
    {
        rfC2P_t *C2pInfo;
        C2pInfo =(rfC2P_t*)mem_malloc(sizeof(rfC2P_t));
        if(C2pInfo  == NULL)
        {
            chk(1);
            return ERR;
        }
        C2pInfo->blkid =blk;
        memcpy(C2pInfo->money,&money,4);
        if(OK == rf_sendCmd(RF_HEAD_C2P,&C2pInfo->blkid))
        {
            dbg("money init ok!");
			mem_free(C2pInfo);
            return OK;
        }
        mem_free(C2pInfo);
    }
    dbg("money init err!");
    return ERR;
}
/*****************************************************************************
 �� �� ��  : money_add
 ��������  : Ǯ����ֵ����
 �������  : u8 blk
             u32 money
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2020��6��22��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
u8 money_add(u8 blk,u32 money)
{
    if(OK == rf_key_check(blk)) //��֤ ok
    {
        rfC2P_t *C2pInfo;
        C2pInfo =(rfC2P_t*)mem_malloc(sizeof(rfC2P_t));
        if(C2pInfo  == NULL)
        {
            chk(1);
            return ERR;
        }
        C2pInfo->mode = 0xc1;
        C2pInfo->blkid =blk;
        memcpy(C2pInfo->money,&money,4);
        //dbg_hex(&C2pInfo->mode, 6);
        if(OK == rf_sendCmd(RF_HEAD_C2J,&C2pInfo->mode)) //��Ǯ
        {
            u32 type = msgType(EVENT_RFID_PURSE_BLANCE,MSG_SRC_RFID,MSG_DT_VAL,1);
            if(msg_sendVal(hMsgSz[MSGQ_RFID],type,C2pInfo->blkid) == OS_FALSE)
            {
                chk(1);
            }
			mem_free(C2pInfo);
            return OK;
        }
        mem_free(C2pInfo);
    }
    dbg("money add err!");
    return ERR;
}

/*****************************************************************************
 �� �� ��  : money_balance
 ��������  : Ǯ������ȡ
 �������  : u8 blk
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2020��6��22��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
u8 money_balance(u8 blk)
{
    if(OK == rf_key_check(blk)) //��֤ ok
    {
        if(OK == rf_sendCmd(RF_HEAD_C2Q,&blk)) //��ȡ���
        {
            u32 balance;
            memcpy(&balance,rfMux.rPkt.info, 4);
            dbg("balance:%d",balance);
            return OK;
        }
    }
    dbg("get balance err!");
    return ERR;
}

/*****************************************************************************
 �� �� ��  : money_dec
 ��������  : Ǯ���ۿ����
 �������  : u8 blk
             u32 money
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2020��6��22��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
u8 money_dec(u8 blk,u32 money)
{
    if(OK == rf_key_check(blk)) //��֤ ok
    {
        u32 curmoney;
        rfC2P_t *C2pInfo;
        C2pInfo =(rfC2P_t*)mem_malloc(sizeof(rfC2P_t));
        if(C2pInfo  == NULL)
        {
            chk(1);
            return ERR;
        }
        C2pInfo->mode = 0xc0;
        C2pInfo->blkid =blk;
        memcpy(C2pInfo->money,&money,4);
        //dbg_hex(&C2pInfo->mode, 6);
        if(OK == rf_sendCmd(RF_HEAD_C2Q,&C2pInfo->blkid)) //��ȡֵ��
        {
            //dbg_hex(rfMux.rPkt.info, 4);
            memcpy(&curmoney,rfMux.rPkt.info, 4);
            if(curmoney < money)
            {
                rf_reportMain(RFID_NSF,curmoney);
                mem_free(C2pInfo);
                return OK;
            }
            else
            {
                if(OK == rf_sendCmd(RF_HEAD_C2J,&C2pInfo->mode)) //��Ǯ
                {
                    curmoney=curmoney-money;
                    rf_reportMain(RFID_GET,curmoney);
                    mem_free(C2pInfo);
                    return OK;
                }
            }
        }
        mem_free(C2pInfo);
    }
    dbg("money dec err!");
    rf_reportMain(RFID_ERR,0);
    return ERR;
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

	dbg("CC:%s %s",__DATE__,__TIME__);
	task_sleep(1000);
	RfidTimeHandle = TimerCreate(200, RfidTimerFunction);
	I2C_EE_Config();
	rf_init_check();  //RFID����ʼ����
	dbg("rf FreeStack:%d",OSTaskGetFreeStackSpace(htaskget(3)));
	for( ;; )
	{
		tMsg_t msg;
		if(msg_recv(hMsgSz[MSGQ_RFID],&msg) != OS_FALSE)
		{
			switch(msg.id)
			{
				case EVENT_RFID_TIMER: //��ʱ���
				{
                    if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))
                    {
                       TimerStart(RfidTimeHandle,1000);
                    }
                    else
                    {
                        // �����ߣ��ٴν����Զ�ģʽ
                        if(OK == rf_goto_auto())
                        {
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
                        u16 mifareATQ = rfMux.rPkt.info[2];             //��8λ
                        mifareATQ = (mifareATQ<<8)+ rfMux.rPkt.info[1]; //��8λ
                        u8 mifareSAK= rfMux.rPkt.info[3];
                        dbg("ATQ:0x%.4x,SAK:0x%.2x,uid: 0x%.8X",mifareATQ,mifareSAK,rfMux.devInfo.uid.u);
						/* ��ʱ��ѯ���Ƿ��뿪 */
						rf_reportMain(RFID_GET,rfMux.devInfo.uid.u);
						//money_dec(blkdef,1);
						TimerStart(RfidTimeHandle,1000);
                    }
                    else if(rfMux.rPkt.infoLen == 0x08)
                    {
                        memcpy(&(rfMux.devInfo),rfMux.rxBuf+8,4);
                        u16 mifareATQ = rfMux.rPkt.info[1];             //��8λ
                        mifareATQ = (mifareATQ<<8)+ rfMux.rPkt.info[0]; //��8λ
                        u8 mifareSAK= rfMux.rPkt.info[2];
                        dbg("ATQ:0x%.4x,SAK:0x%.2x,uid: 0x%.8X",mifareATQ,mifareSAK,rfMux.devInfo.uid.u);
						/* ��ʱ��ѯ���Ƿ��뿪 */
						rf_reportMain(RFID_GET,rfMux.devInfo.uid.u);
						TimerStart(RfidTimeHandle,1000);
                    }
					break;
				}
				case EVENT_RFID_RDBLK:
				{	/* �������ݿ� */
					if(OK != rf_key_check(msg.uch[0]))
					{
                        rfccubuf.cmdType=0x01;
                        rfccubuf.cmd=0xA3;
                        com3_tx_rsp(rfccubuf,1,0x06);

                        break;
                    }
					if(OK == rf_sendCmd(RF_HEAD_C2G,msg.uch))
					{
						memcpy(rfMux.devInfo.dat,rfMux.rPkt.info,16);
						dbg("read OK!");
						dbg_hex(rfMux.devInfo.dat, 16);
                        memcpy(rfccubuf.info,rfMux.devInfo.dat,16);
                        rfccubuf.cmdType=0x01;
                        rfccubuf.cmd=0xA3;
                        com3_tx_rsp(rfccubuf,0,0x16);

					}
					else
					{
						dbg("read err!");
                        rfccubuf.cmdType=0x01;
                        rfccubuf.cmd=0xA3;
                        com3_tx_rsp(rfccubuf,1,0x06);

					}
					break;
				}
				case EVENT_RFID_WRBLK:
				{	/* д�����ݿ� */
                    dbg_hex((u8 *)(msg.ptr),17);
                    u8 dat[17];
                    memcpy(dat,(u8 *)(msg.ptr),17);
					if(OK != rf_key_check(dat[0]))
					{
                        rfccubuf.cmdType=0x01;
                        rfccubuf.cmd=0xA4;
                        com3_tx_rsp(rfccubuf,1,0x06);

                        break;
                    }
                    if(OK == rf_sendCmd(RF_HEAD_C2H,dat))
                    {
                        dbg("write OK!");
                        rfccubuf.cmdType=0x01;
                        rfccubuf.cmd=0xA4;
                        com3_tx_rsp(rfccubuf,0,0x06);
                    }
                    else
                    {
                        dbg("write err!");
                        rfccubuf.cmdType=0x01;
                        rfccubuf.cmd=0xA4;
                        com3_tx_rsp(rfccubuf,1,0x06);
                    }
                    break;
                }
                case EVENT_RFID_PURSE_INIT:
                {
                    dbg("money:0x%.8x",msg.u);
                    money_init(blkdef, msg.u);
                    break;
                }
                case EVENT_RFID_PURSE_CUT:
                {
                    dbg("money:0x%.8x",msg.u);
                    money_dec(blkdef, msg.u);
                    break;
                }
                case EVENT_RFID_PURSE_BLANCE:
                {
                    money_balance(blkdef);
                    break;
                }
                case EVENT_RFID_PURSE_PAY:
                {
                    dbg("money:0x%.8x",msg.u);
                    money_add(blkdef, msg.u);
                    break;
                }
                case EVENT_RFID_USER_REQ:
                {
                    uint8 loop;
                    for(loop=0;loop<grfUser.cnt;loop++)
                    {
                        //dbg("id:%d, serial:%02X %02X %02X %02X",loop,
                        //grfUser.serial[loop][3],grfUser.serial[loop][2],grfUser.serial[loop][1],grfUser.serial[loop][0]);
                        memcpy(&rfccubuf.info[loop*4+1],(u8 *)(&grfUser.serial[loop][0]),4);
                    }
                    rfccubuf.cmdType=0x02;
                    rfccubuf.cmd=0xA1;
                    rfccubuf.info[0]= grfUser.cnt;
                    com3_tx_rsp(rfccubuf,0,rfccubuf.info[0]*4+7);
                    break;
                }
                case EVENT_RFID_USER_ADD:
                {
                    dbg("uid:0x%.8x",msg.u);
                    if(rfUsr_append(msg.uch)==OK)
                    {
                        dbg("add user ok");
                    }
                    else
                    {
                        dbg("add user err");
                    }
                    break;
                }
                case EVENT_RFID_USER_DEL:
                {
                    dbg("uid:0x%.8x",msg.u);
                    if(rfUsr_pop(msg.uch)==OK)
                    {
                        dbg("del user ok");
                    }
                    else
                    {
                        dbg("del user err");
                    }
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

