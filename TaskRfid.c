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

#include "ccu.h"

ccubuf_t rfccubuf;

TimerHandle_t   RfidTimeHandle	= NULL;
void RfidTimerFunction( void *pvParameters );

/*
 密钥类型（1 字节）：   0x60——密钥A
                        0x61——密钥B
 密钥区号（1 字节）：   取值范围0～15
 密钥（6 字节或16 字节）
 RF_HEAD_C1E1= 0x0845010E		// 装载IC卡密钥，6位密钥
向密钥01 区装载密钥A：0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
*/

const uint8 C1E_info[]={0x60,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	// for key1

/*
 RF_HEAD_C2N = 0x074E020D        // 自动检测

 自动检测模式ADMode（1 字节）：0x03  数据输出后不继续检测,产生中断,串口主动发送

 天线驱动方式TxMode（1 字节）： 0：TX1、TX2 交替驱动
                                1：仅TX1 驱动
                                2：仅TX2 驱动
                                3：TX1、TX2 同时驱动

 请求代码ReqCode（1 字节）：    0x26～IDLE
                                0x52～ALL

 验证模式AuthMode（1 字节）：   0x45-‘E’～用E2 密钥验证
                                0x46-‘F’～用直接密钥验证
                                0 ～不验证

 密钥AB KeyType（1 字节）：     0x60～密钥A
                                0x61～密钥B

 密钥Key：                      若验证模式为‘E’，则为密钥区号（1 字节）
                                若验证模式为‘F’，则为密钥（6 或16 字节）

 卡块号Block（1 字节）：        S50（0～63）
                                S70（0～255）
                                PLUS CPU 2K（0～127）
                                PLUS CPU 4K（0～255）

用E2密钥验证密钥区号01，读出第8块数据内容
*/
const uint8 C2N_info[]={0x03,0x03,0x26,0x45,0x60,0x01,0x08};

/*
 RF_HEAD_C2M= 0x024D0208		// 该命令用于激活卡片，是请求、防碰撞和选择三条命令的组合
IDLE方式激活
*/
const uint8 C2M_info[]={0x00,0x26};

//通过该读取自动检测数据命令，可以决定读取数据后是否继续检测。
const uint8  C2O_info[] ={0x00};




rfMux_t rfMux;

T_RFUSER grfUser; /*用户信息*/

void rf_reInitRx(uint8 mode);
int16 rf_sendCmd(uint32 cmdHead, const uint8 * info);
uint8 rf_init(void);
uint8 rf_wait( void );

u8 rf_goto_auto(void);
u8 rf_key_check(u8 blk);

void rf_reportMain(u32 id, u32 rfid);

/*钱包相关变量及函数 */

typedef struct
{
    u8  mode;
   	u8  blkid;
    u8  money[4];
}rfC2P_t;    //钱包数据结构

u8   blkdef  =0x04;
u8 money_dec(u8 blk,u32 money);
u8 money_add(u8 blk,u32 money);
u8 money_init(u8 blk, u32 money);
u8 money_balance(u8 blk);



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


/*增加一个到后面*/
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
		    /*找到相同数据*/
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

/*删除卡*/
int rfUsr_pop(u8 *buf)
{
	int loop=grfUser.cnt;
	while(loop--)
	{
		if(memcmp(buf, (u8 *)&grfUser.serial[loop][0], 4) == NULL)
		{
		    /*找到相同数据*/
            uint8 j;
            for(j=loop; j<grfUser.cnt;j++) //在i数组后面的元素往前移
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
		grfUser.serial[loop][3],grfUser.serial[loop][2],grfUser.serial[loop][1],grfUser.serial[loop][0]);
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
		//dbg_hex(rfSend.info,len);
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
u8 devinfo[20];

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
        //获取模块的版本号
         if(rf_sendCmd(RF_HEAD_C1A,NULL) == OK)
         {
             memcpy(devinfo,rfMux.rPkt.info,20);
             dbg("%s",devinfo);
    	    //向密钥01 区装载密钥A：0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
    	    //此命令是向模块内装载密码，并非改变Mifare1 卡内扇区的密码
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


/*进入自动检测模式 */
u8 rf_goto_auto(void)
{
    /*进入自动检测模式 */
    if(rf_sendCmd(RF_HEAD_C2N,C2N_info) != OK)
    //该命令用于卡片的自动检测，执行该命令成功后，在UART 模式下，模块将主动发送
    //读取到卡片的数据
    {
        return ERR;
    }
    rf_reInitRx(1);
    return OK;
}

/*rf 密钥验证 */
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
        if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))//检测rf card?
        {
            msg_sendVal(hMsgSz[MSGQ_RFID],msgType(EVENT_RFID_CHKCARD, 0, 0, 0),0);
        }
        else
        {
            /* 进入自动检测模式 */
            if(rf_goto_auto() == OK)
            {
                dbg("RF_HEAD_C2N ok");
            }
            else
            {    /* 检测RF模块错误 */
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
 函 数 名  : money_init
 功能描述  : 钱包格式初始化
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2020年6月21日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
u8 money_init(u8 blk, u32 money)
{
    if(OK == rf_key_check(blk)) //验证 ok
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
 函 数 名  : money_add
 功能描述  : 钱包充值操作
 输入参数  : u8 blk
             u32 money
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2020年6月22日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
u8 money_add(u8 blk,u32 money)
{
    if(OK == rf_key_check(blk)) //验证 ok
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
        if(OK == rf_sendCmd(RF_HEAD_C2J,&C2pInfo->mode)) //充钱
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
 函 数 名  : money_balance
 功能描述  : 钱包余额获取
 输入参数  : u8 blk
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2020年6月22日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
u8 money_balance(u8 blk)
{
    if(OK == rf_key_check(blk)) //验证 ok
    {
        if(OK == rf_sendCmd(RF_HEAD_C2Q,&blk)) //获取余额
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
 函 数 名  : money_dec
 功能描述  : 钱包扣款操作
 输入参数  : u8 blk
             u32 money
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2020年6月22日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
u8 money_dec(u8 blk,u32 money)
{
    if(OK == rf_key_check(blk)) //验证 ok
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
        if(OK == rf_sendCmd(RF_HEAD_C2Q,&C2pInfo->blkid)) //获取值块
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
                if(OK == rf_sendCmd(RF_HEAD_C2J,&C2pInfo->mode)) //扣钱
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

	dbg("CC:%s %s",__DATE__,__TIME__);
	task_sleep(1000);
	RfidTimeHandle = TimerCreate(200, RfidTimerFunction);
	I2C_EE_Config();
	rf_init_check();  //RFID检测初始化。
	dbg("rf FreeStack:%d",OSTaskGetFreeStackSpace(htaskget(3)));
	for( ;; )
	{
		tMsg_t msg;
		if(msg_recv(hMsgSz[MSGQ_RFID],&msg) != OS_FALSE)
		{
			switch(msg.id)
			{
				case EVENT_RFID_TIMER: //定时检测
				{
                    if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))
                    {
                       TimerStart(RfidTimeHandle,1000);
                    }
                    else
                    {
                        // 卡移走，再次进入自动模式
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
                        u16 mifareATQ = rfMux.rPkt.info[2];             //高8位
                        mifareATQ = (mifareATQ<<8)+ rfMux.rPkt.info[1]; //低8位
                        u8 mifareSAK= rfMux.rPkt.info[3];
                        dbg("ATQ:0x%.4x,SAK:0x%.2x,uid: 0x%.8X",mifareATQ,mifareSAK,rfMux.devInfo.uid.u);
						/* 定时查询卡是否离开 */
						rf_reportMain(RFID_GET,rfMux.devInfo.uid.u);
						//money_dec(blkdef,1);
						TimerStart(RfidTimeHandle,1000);
                    }
                    else if(rfMux.rPkt.infoLen == 0x08)
                    {
                        memcpy(&(rfMux.devInfo),rfMux.rxBuf+8,4);
                        u16 mifareATQ = rfMux.rPkt.info[1];             //高8位
                        mifareATQ = (mifareATQ<<8)+ rfMux.rPkt.info[0]; //低8位
                        u8 mifareSAK= rfMux.rPkt.info[2];
                        dbg("ATQ:0x%.4x,SAK:0x%.2x,uid: 0x%.8X",mifareATQ,mifareSAK,rfMux.devInfo.uid.u);
						/* 定时查询卡是否离开 */
						rf_reportMain(RFID_GET,rfMux.devInfo.uid.u);
						TimerStart(RfidTimeHandle,1000);
                    }
					break;
				}
				case EVENT_RFID_RDBLK:
				{	/* 读卡数据块 */
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
				{	/* 写卡数据块 */
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

