/******************************************************************************

                  版权所有 (C), 2015-2050, 厦门优胜卫厨有限公司

 ******************************************************************************
  文 件 名   : TaskRfid.h
  版 本 号   : 初稿
  作    者   : zgj
  生成日期   : 2019年11月17日
  最近修改   :
  功能描述   : RFID头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年11月17日
    作    者   : zgj
    修改内容   : 创建文件

******************************************************************************/
#ifndef _TASKRFID_H_
#define _TASKRFID_H_

#define SLAVE_TYPE     0 //从机模式，类似公交地铁刷卡扣款模式

#define MASTER_TYPE    1 //主机模式，管理员模式

#define RFID_TYPE     MASTER_TYPE

#include "config.h"

#include "com.h"

#define rfid_com  COM4
#define rfid_rxDeal()  com4_rxDeal()





#define RFID_BUF_MAX	32


#define RFUSER_MAX 	    5 /*限制5个用户*/
#define ADDR_RFUSER     0x0000

typedef struct
{
	uint8 cnt;                      /*用户数	指示后面启用用户个数：０～５*/
	uint8 crc;                      /*CRC	后面启用用户数据的CRC校验值*/
	uint8 serial[RFUSER_MAX][4];     /*用户n	存储用户RFID卡*/
}T_RFUSER;

/*
该命令用模块内部已存入的密钥与卡的密钥进行验证，所以使用该命令前，应事先用“装
载IC 卡密钥”命令把密钥成功载入模块内，另外，需要验证的卡的扇区号不必与模块内密
钥区号相等
*/
typedef struct
{
	u8 mode;        //0x60——密钥A  0x61——密钥B
	u8 uid[4];      //卡序列号（4 字节）
	u8 keyid;       //密钥区号（1 字节）： 取值范围0～7
	u8 blkid;       //卡块号（1 字节）：S50（0～63）S70（0～255）PLUS CPU 2K（0～127）PLUS CPU 4K（0～255）
} rfC2E_t;        // E2密钥验证信息结构

typedef struct
{
	u8 blkid;
	u8 dat[16];
} rfC2H_t;    // 写数据信息结构



typedef struct
{
union {
        struct
        {
            uint8  frameLen;
            uint8  cmdType;
            uint8  cmdOrSta;
            uint8  infoLen;
        };
        uint32  cmdHead;
      };
    uint8  info[RFID_BUF_MAX-4];   //28 byte
} rfPkt_t;


typedef struct {
	UN32	uid;
	uint8	dat[16];
} rfInfo_t;


typedef struct
{
	union
	{
		uint8 rxBuf[RFID_BUF_MAX];
		rfPkt_t rPkt;
	};
	uint8	rIdx;		// 接收位置索引
	uint8	frameOK;	// 帧接收完整标记: 1--完整，0--没有
	uint8	mode;		// 0--主动检测  1--自动检测
	uint8	status;		// 0--没有检测到卡，1--检测到卡
	rfInfo_t	devInfo;	// 卡信息
} rfMux_t;



#define RF_HEAD_C1A		0x00410106		// 读取设备信息
#define RF_HEAD_C1B		0x00420106		// 配置IC卡接口
#define RF_HEAD_C1C		0x00430106		// 关闭IC卡接口
#define RF_HEAD_C1D		0x01440107		// 设置IC卡接口协议
#define RF_HEAD_C1E1	0x0845010E		// 装载IC卡密钥，6位密钥
#define RF_HEAD_C1E2	0x12450118		// 装载IC卡密钥，16位密钥
#define RF_HEAD_C1F		0x02460108		// 设置IC卡接口的寄存器值
#define RF_HEAD_C1G		0x01470107		// 获取IC卡接口的寄存器值
#define RF_HEAD_C1H		0x01480107		// 设置波特率
#define RF_HEAD_C1I		0x01490107		// 设置天线驱动方式
#define RF_HEAD_C1K		0x014B0107		// 设置新旧帧格式
#define RF_HEAD_C1U		0x02550108		// 设置设备工作模式
#define RF_HEAD_C1V		0x00560106		// 获取设备工作模式
#define RF_HEAD_C1a		0x11610117		// 装载用户密钥, info: 扇区+密钥
#define RF_HEAD_C1b		0x02620108		// 读eeprom, info: addr+len
#define RF_HEAD_C1c		0x00630106		// 写eeprom，info: addr+len+data

#define RF_HEAD_C2A		0x01410207		// 请求
#define RF_HEAD_C2B1	0x02420208		// 防碰撞1
#define RF_HEAD_C2B2	0x0642020C		// 防碰撞2
#define RF_HEAD_C2C		0x0543020B		// 卡选择
#define RF_HEAD_C2D		0x00440206		// 卡挂起
#define RF_HEAD_C2E		0x0745020D		// 密钥验证
#define RF_HEAD_C2FA	0x0C460212		// 直接密钥A验证
#define RF_HEAD_C2FB	0x0C460212		// 直接密钥B验证
#define RF_HEAD_C2G		0x01470207		// Mifare卡读
#define RF_HEAD_C2H		0x11480217		// Mifare卡写
#define RF_HEAD_C2I		0x0549020B		// UltraLight卡读
#define RF_HEAD_C2J		0x074A020D		// Mifare值操作
#define RF_HEAD_C2L		0x014C0207		// 卡复位
#define RF_HEAD_C2M		0x024D0208		// 卡激活
#define RF_HEAD_C2N		0x074E020D		// 自动检测，使用密钥E2或不验证
#define RF_HEAD_C2O		0x014F0207		// 读自动检测数据
#define RF_HEAD_C2P		0x0550020B		// 设置值块的值
#define RF_HEAD_C2Q		0x01510207		// 获取值块的值
#define RF_HEAD_C2X		0x00580206		// 数据交互命令, info: data+2B



void rf_init_check(void);

extern int rfUsr_isRfok(u8 *buf);
extern int rfUsr_append(u8 *buf);
extern void rfUsr_showRfSerial(void);
extern void rfUsr_setDefault(void);
extern void rfUsr_init(void);
extern int rfUsr_pop(u8 *buf);

extern uint8 rf_bccCalc(uint8 * buf,uint8 len);

extern void com4_rxDeal(void);

extern void TaskRfid(void *pvParameters);


extern u32 get_rf_uid(void);
extern void get_dev_info(void);


#endif

