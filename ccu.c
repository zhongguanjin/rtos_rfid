#include "ccu.h"
#include "dbg.h"
#include "TaskRfid.h"
#include "osif.h"

ccubuf_t ccubuf;

uint8 rxbuf[32];
void  ccu_cmdDeal(void);


/*****************************************************************************
 �� �� ��  : com3_tx_rsp
 ��������  : com3Ӧ����
 �������  : uint8 sta  0-ok��1-err
             uint8 len
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��7��8��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void com3_tx_rsp(ccubuf_t ptx,uint8 sta,uint8 len)
{
    ptx.resOrSta = sta; //�ɹ�
    ptx.frameLen = len;
    ptx.rxBuf[ptx.frameLen-2] = rf_bccCalc(ptx.rxBuf, ptx.frameLen-2);
    ptx.rxBuf[ptx.frameLen-1] = 0x03;
    com_send(ccu_com,ptx.rxBuf,ptx.frameLen); //�жϷ���
}

/*****************************************************************************
 �� �� ��  : com3_rxDeal
 ��������  : ccu��������֡
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��11��7��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void com3_rxDeal(void)
{
	char ch;
	static uint8 index = 0;
	if(com_rxLeft(ccu_com) != 0)
	{
		while(1)
		{
			if(OK == com_getch(ccu_com,&ch))
			{
                rxbuf[index++]= ch;
                if((rxbuf[index-1] == 0x03)&&(index == rxbuf[0]))
                {
                    index -=2;// ָ��bcc
                    if(rf_bccCalc(rxbuf, index) == rxbuf[index]) //bcc ok?
                    {
                        memcpy(&(ccubuf.rxBuf),rxbuf,index);
                        index = 0;
                        ccu_cmdDeal();
                    }
                    else
                    {
                        memset(&rxbuf,0,32);
                        index = 0;
                    }
                }

			}
			else
			{
				break;
			}
		}
	}
}

/*****************************************************************************
 �� �� ��  : ccu_cmdDeal
 ��������  : ccu�����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��11��7��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void ccu_cmdDeal(void)
{
    switch(ccubuf.cmdType)
    {
       case 0x01: //��Ƭ��ز�������
       {
            switch (ccubuf.cmd)
            {
               case 0xA1: //���������������֤��Կ
               {

					break;
               }
               case 0xA2: //ԭ��Ϊע�Ῠ����°����ȡ���˴�����
               {

                    break;
               }
               case 0xA3: //��ָ�����ݿ������֤KEYA
               {
                   if((ccubuf.info[0]%4) != 3)
                   {
                       u32 type =msgType(EVENT_RFID_RDBLK,MSG_SRC_CCU,MSG_DT_VAL,1);
                       dbg("type:0x%.8x,blk:%d",type,ccubuf.info[0]);
                       msg_sendVal(hMsgSz[MSGQ_RFID],type,ccubuf.info[0]);
                   }
                   else
                   {
                       dbg("err,pwd ctr blk");
                       com3_tx_rsp(ccubuf,1,ccubuf.frameLen-1);
                   }
                   break;
               }
               case 0xA4: //дָ�����ݿ������֤KEYA
               {
                   if((ccubuf.info[0]%4) != 3)
                   {
                       u8 * p = mem_malloc(17);
                       memcpy(p,ccubuf.info,17);
                       dbg_hex(p, 17);
                       u32 type =msgType(EVENT_RFID_WRBLK,MSG_SRC_CCU,MSG_DT_PTR_M,17);
                       dbg("type:0x%.8x,blk:%d",type,ccubuf.info[0]);
                       msg_sendVal(hMsgSz[MSGQ_RFID],type,(u32)p);
                       mem_free(p);
                   }
                   else
                   {
                       dbg("err,pwd ctr blk");
                       com3_tx_rsp(ccubuf,1,ccubuf.frameLen-17);
                   }
                    break;
               }
               case 0xA5: //����IC����Կ����,��֤KEYA
               {

                    break;
               }
               default: //����
               {

                    break;
               }
            }
            break;
       }
       case 0x02: //�û�Ȩ�޲�ѯ������
       {
           switch (ccubuf.cmd)
           {
              case 0xA1: //�û�RFIDʹ������ѯ
              {
                  u32 type =msgType(EVENT_RFID_USER_REQ,MSG_SRC_CCU,0,0);
                  dbg("type:0x%.8x",type);
                  msg_sendVal(hMsgSz[MSGQ_RFID],type,0);
                  break;
              }
              case 0xA2: //����û�RFID
              {
                  u32 type =msgType(EVENT_RFID_USER_ADD,MSG_SRC_CCU,MSG_DT_CH,4);
                  u32 uid;
                  memcpy(&uid,ccubuf.info,4);
                  dbg("type:0x%.8x uid:0x%x",type,uid);
                  msg_sendVal(hMsgSz[MSGQ_RFID],type,uid);
                  break;
              }
              case 0xA3: //ɾ���û�RFID
              {
                  u32 type =msgType(EVENT_RFID_USER_DEL,MSG_SRC_CCU,MSG_DT_CH,4);
                  u32 uid;
                  memcpy(&uid,ccubuf.info,4);
                  dbg("type:0x%.8x uid:0x%x",type,uid);
                  msg_sendVal(hMsgSz[MSGQ_RFID],type,uid);
                  break;
              }
             default: //����
             {

                  break;
             }
          }
           break;
       }
       case 0x03: //
       {
            switch (ccubuf.cmd)
            {
               case 0xA1: //Ǯ����ʼ��
               {
                   if((ccubuf.info[0]%4) != 3)
                   {
                       u8 * p = mem_malloc(5);
                       memcpy(p,ccubuf.info,5);
                       dbg_hex(p, 5);
                       u32 type =msgType(EVENT_RFID_PURSE_INIT,MSG_SRC_CCU,MSG_DT_PTR_M,5);
                       dbg("type:0x%.8x,blk:%d",type,ccubuf.info[0]);
                       msg_sendVal(hMsgSz[MSGQ_RFID],type,(u32)p);
                       mem_free(p);
                   }
                   else
                   {
                       dbg("err,pwd ctr blk");
                       com3_tx_rsp(ccubuf,1,ccubuf.frameLen-5);
                   }
                   break;
               }
               case 0xA2: //
               {
                   if((ccubuf.info[0]%4) != 3)
                   {
                       u8 * p = mem_malloc(5);
                       memcpy(p,ccubuf.info,5);
                       dbg_hex(p, 5);
                       u32 type =msgType(EVENT_RFID_PURSE_CUT,MSG_SRC_CCU,MSG_DT_PTR_M,5);
                       dbg("type:0x%.8x,blk:%d",type,ccubuf.info[0]);
                       msg_sendVal(hMsgSz[MSGQ_RFID],type,(u32)p);
                       mem_free(p);
                   }
                   else
                   {
                       dbg("err,pwd ctr blk");
                       com3_tx_rsp(ccubuf,1,ccubuf.frameLen-5);
                   }
                   break;
               }
               case 0xA3: //
               {
                   if((ccubuf.info[0]%4) != 3)
                   {
                       u8 * p = mem_malloc(5);
                       memcpy(p,ccubuf.info,5);
                       dbg_hex(p, 5);
                       u32 type =msgType(EVENT_RFID_PURSE_PAY,MSG_SRC_CCU,MSG_DT_PTR_M,5);
                       dbg("type:0x%.8x,blk:%d",type,ccubuf.info[0]);
                       msg_sendVal(hMsgSz[MSGQ_RFID],type,(u32)p);
                       mem_free(p);
                   }
                   else
                   {
                       dbg("err,pwd ctr blk");
                       com3_tx_rsp(ccubuf,1,ccubuf.frameLen-5);
                   }
                   break;
               }
               case 0xA4:
               {
                   if((ccubuf.info[0]%4) != 3)
                   {
                       u32 type =msgType(EVENT_RFID_PURSE_BLANCE,MSG_SRC_CCU,0,1);
                       dbg("type:0x%.8x,blk:%d",type,ccubuf.info[0]);
                       msg_sendVal(hMsgSz[MSGQ_RFID],type,ccubuf.info[0]);
                   }
                   else
                   {
                       dbg("err,pwd ctr blk");
                       com3_tx_rsp(ccubuf,1,ccubuf.frameLen-1);
                   }

                   break;
               }
              default: //����
              {

                   break;
              }
           }
            break;
       }
       default: //����
       {

            break;
       }

    }

}


