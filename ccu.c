#include "ccu.h"
#include "dbg.h"
#include "TaskRfid.h"
#include "osif.h"

ccubuf_t ccubuf;

uint8 rxbuf[32];
void  ccu_cmdDeal(void);


/*****************************************************************************
 函 数 名  : com3_tx_rsp
 功能描述  : com3应答函数
 输入参数  : uint8 sta  0-ok，1-err
             uint8 len
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年7月8日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void com3_tx_rsp(ccubuf_t ptx,uint8 sta,uint8 len)
{
    ptx.resOrSta = sta; //成功
    ptx.frameLen = len;
    ptx.rxBuf[ptx.frameLen-2] = rf_bccCalc(ptx.rxBuf, ptx.frameLen-2);
    ptx.rxBuf[ptx.frameLen-1] = 0x03;
    com_send(ccu_com,ptx.rxBuf,ptx.frameLen); //中断发送
}

/*****************************************************************************
 函 数 名  : com3_rxDeal
 功能描述  : ccu接收数据帧
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年11月7日
    作    者   : zgj
    修改内容   : 新生成函数

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
                    index -=2;// 指向bcc
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
 函 数 名  : ccu_cmdDeal
 功能描述  : ccu命令处理
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年11月7日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void ccu_cmdDeal(void)
{
    switch(ccubuf.cmdType)
    {
       case 0x01: //卡片相关操作命令
       {
            switch (ccubuf.cmd)
            {
               case 0xA1: //读卡号命令，无需验证密钥
               {

					break;
               }
               case 0xA2: //原来为注册卡命令，新版软件取消了此命令
               {

                    break;
               }
               case 0xA3: //读指定数据块命令，验证KEYA
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
               case 0xA4: //写指定数据块命令，验证KEYA
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
               case 0xA5: //设置IC卡密钥命令,验证KEYA
               {

                    break;
               }
               default: //其它
               {

                    break;
               }
            }
            break;
       }
       case 0x02: //用户权限查询与设置
       {
           switch (ccubuf.cmd)
           {
              case 0xA1: //用户RFID使用数查询
              {
                  u32 type =msgType(EVENT_RFID_USER_REQ,MSG_SRC_CCU,0,0);
                  dbg("type:0x%.8x",type);
                  msg_sendVal(hMsgSz[MSGQ_RFID],type,0);
                  break;
              }
              case 0xA2: //添加用户RFID
              {
                  u32 type =msgType(EVENT_RFID_USER_ADD,MSG_SRC_CCU,MSG_DT_CH,4);
                  u32 uid;
                  memcpy(&uid,ccubuf.info,4);
                  dbg("type:0x%.8x uid:0x%x",type,uid);
                  msg_sendVal(hMsgSz[MSGQ_RFID],type,uid);
                  break;
              }
              case 0xA3: //删除用户RFID
              {
                  u32 type =msgType(EVENT_RFID_USER_DEL,MSG_SRC_CCU,MSG_DT_CH,4);
                  u32 uid;
                  memcpy(&uid,ccubuf.info,4);
                  dbg("type:0x%.8x uid:0x%x",type,uid);
                  msg_sendVal(hMsgSz[MSGQ_RFID],type,uid);
                  break;
              }
             default: //其它
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
               case 0xA1: //钱包初始化
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
              default: //其它
              {

                   break;
              }
           }
            break;
       }
       default: //其它
       {

            break;
       }

    }

}


