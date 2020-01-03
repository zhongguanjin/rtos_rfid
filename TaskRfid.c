/******************************************************************************

                  版权所有 (C), 2015-2050, 厦门优胜卫厨有限公司

 ******************************************************************************
  文 件 名   : TaskRfid.c
  版 本 号   : 初稿
  作    者   : zgj
  生成日期   : 2019年11月17日
  最近修改   :
  功能描述   : RFID源文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年11月17日
    作    者   : zgj
    修改内容   : 创建文件

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

T_RFUSER grfUser; /*用户信息*/

void rf_reInitRx(uint8 mode);
int16 rf_sendCmd(uint32 cmdHead, const uint8 * info);
uint8 rf_init(void);
uint8 rf_wait( void );

void rf_over_check(void);


/*判断卡是否有效，返回1有效，0无效*/
int rfUsr_isRfok(u8 *buf)
{
	int loop=grfUser.cnt;
	while(loop--)
	{
		if(memcmp(buf, (u8 *)&grfUser.serial[loop][0], 4) == NULL)
		{/*找到相同数据*/
			return 1;
		}
	}
	return 0;
}

/*返回1，满*/
int rfUsr_isRfFull(void)
{
	if(grfUser.cnt>=RFUSER_MAX)
	{
	    dbg("user full");
		return 1;
	}
	return 0;
}
/*返回1，空*/
int rfUsr_isRFEmpty(void)
{
	if(grfUser.cnt==0)
	{
	    dbg("user empty");
		return 1;
	}
	return 0;
}


/*增加一个到后面*/
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

/*删除卡*/
int rfUsr_pop(u8 *buf)
{
	int loop=grfUser.cnt;
	while(loop--)
	{
		if(memcmp(buf, (u8 *)&grfUser.serial[loop][0], 4) == NULL)
		{
		    /*找到相同数据*/
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
 函 数 名  : rfUsr_showRfSerial
 功能描述  : 查看所有用户卡使用数据
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年11月17日
    作    者   : zgj
    修改内容   : 新生成函数

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
 函 数 名  : rfUsr_setDefault
 功能描述  : 用户RF使用清除
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年11月17日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void rfUsr_setDefault(void)
{
	memset((u8 *)(&grfUser),0, sizeof(T_RFUSER));
	grfUser.crc = 0xff;
	EEPROM_Write(ADDR_RFUSER, (u8 *)(&grfUser),sizeof(T_RFUSER));
	return;
}
/*****************************************************************************
 函 数 名  : rfUsr_init
 功能描述  : 用户RFID卡初始化
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年11月17日
    作    者   : zgj
    修改内容   : 新生成函数

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
 函 数 名  : rf_bccCalc
 功能描述  : 校验码函数
 输入参数  : uint8 * buf
             uint8 len
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年7月11日 星期二
    作    者   : zgj
    修改内容   : 新生成函数

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
 函 数 名  : rf_sendCmd
 功能描述  : RF发送命令帧函数
 输入参数  : u32 cmdHead
             const u8 * info
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年7月11日 星期二
    作    者   : zgj
    修改内容   : 新生成函数

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
	//帧长必须不小于6 字节，最大不能超过70 字节，且帧长必须等于信息长度加6；
	rfSend.frameLen = rfSend.infoLen + 6;
	//校验和，从FrameLen 开始到Info 的最后一字节异或取反
	rfSend.info[len++] = rf_bccCalc((uint8 *)&rfSend,(rfSend.infoLen+4));
	rfSend.info[len++] = 0x03;
	len += 4;
	rf_reInitRx(0);  //主动交互模式
	com_send(rfid_com,(uint8 *)&rfSend,len); //发送命令帧数据
	//for recv
    if(rf_wait() == OK)
    {
        if(rfMux.rxBuf[2] == 0)  //状态成功
        {	// 成功
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
 函 数 名  : rf_init
 功能描述  : RFID初始化函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年7月13日 星期四
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
uint8 rf_init(void)
{
    uint8 times = 1;
	memset(&rfMux,0,sizeof(rfMux));
	while(times-- != 0)
	{	// 发送两空格，初始化串口通信
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
        /* 检测到模块 进入自动检测模式 */
        if(rf_sendCmd(RF_HEAD_C2N,C2N_info) == OK)
        //该命令用于卡片的自动检测，执行该命令成功后，在UART 模式下，模块将主动发送
        //读取到卡片的数据
        {
            dbg("RF_HEAD_C2N ok");
            rf_reInitRx(1);
        }
        else
        {    /* 检测RF模块错误 */
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
        /* 检测到模块 进入自动检测模式 */
        if(rf_sendCmd(RF_HEAD_C2N,C2N_info) == OK)
        //该命令用于卡片的自动检测，执行该命令成功后，在UART 模式下，模块将主动发送
        //读取到卡片的数据
        {
            dbg("RF_HEAD_C2N ok");
            rfUsr_init();
            rf_reInitRx(1);
        }
        else
        {    /* 检测RF模块错误 */
            dbg("RF_HEAD_C2N err");
        }
    }
    else
    {
        dbg("rf_init err");
    }
}

/*****************************************************************************
 函 数 名  : get_rf_uid
 功能描述  : 读卡号uid
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
uint32 get_rf_uid(void)
{
    return rfMux.devInfo.uid.u;
}

/*****************************************************************************
 函 数 名  : read_rf_dat
 功能描述  : 读rfid块号的值函数
 输入参数  : uint8 blank
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年7月26日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void read_rf_dat(uint8 blank)
{
    rfC2H_t info_w;
    rfC2E_t info;
    if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))
    {
        info.mode = 0x60;                           //0x60——密钥A
        memcpy(info.uid,rfMux.devInfo.uid.uch,4);   //卡序列号（4 字节）
        info.keyid = 1;                             //密钥区号（1 字节）： 取值范围0～7
        info.blkid = blank;
        /*卡块号（1 字节）：
            S50（0～63）
            S70（0～255）
            PLUS CPU 2K（0～127）
            PLUS CPU 4K（0～255）
        */
        if(OK == rf_sendCmd(RF_HEAD_C2E,(uint8 *)&info)) //密钥验证
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
 函 数 名  : rf_reportMain
 功能描述  : 上报主任务处理
 输入参数  : u32 id
             u32 rfid
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年11月19日
    作    者   : zgj
    修改内容   : 新生成函数

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
                //收到完整数据，发给主任务处理
                rfMux.rIdx -= 2;    // 指向bcc
                if(rf_bccCalc(rfMux.rxBuf,rfMux.rIdx) == rfMux.rxBuf[rfMux.rIdx])
                {
                     rfMux.frameOK =1; //接收完一帧数据
                    if(rfMux.mode == 1)
                    {   /* 主动上报，检测到刷卡 */
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
	rf_init_check();  //RFID检测初始化。
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
                        // 卡移走，再次进入自动模式
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
						/* 定时查询卡是否离开 */
						TimerStart(RfidTimeHandle,250);
                        rf_reportMain(RFID_GET,rfMux.devInfo.uid.u);
                    }
					break;
				}
				case EVENT_RFID_RDBLK:
				{	/* 读卡数据块 */

					break;
				}
				case EVENT_RFID_WRBLK:
				{	/* 写卡数据块 */

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

