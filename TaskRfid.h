/******************************************************************************

                  ��Ȩ���� (C), 2015-2050, ������ʤ�������޹�˾

 ******************************************************************************
  �� �� ��   : TaskRfid.h
  �� �� ��   : ����
  ��    ��   : zgj
  ��������   : 2019��11��17��
  ����޸�   :
  ��������   : RFIDͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��11��17��
    ��    ��   : zgj
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef _TASKRFID_H_
#define _TASKRFID_H_

#define SLAVE_TYPE     0 //�ӻ�ģʽ�����ƹ�������ˢ���ۿ�ģʽ

#define MASTER_TYPE    1 //����ģʽ������Աģʽ

#define RFID_TYPE     MASTER_TYPE

#include "config.h"

#include "com.h"

#define rfid_com  COM4
#define rfid_rxDeal()  com4_rxDeal()





#define RFID_BUF_MAX	32


#define RFUSER_MAX 	    5 /*����5���û�*/
#define ADDR_RFUSER     0x0000

typedef struct
{
	uint8 cnt;                      /*�û���	ָʾ���������û�������������*/
	uint8 crc;                      /*CRC	���������û����ݵ�CRCУ��ֵ*/
	uint8 serial[RFUSER_MAX][4];     /*�û�n	�洢�û�RFID��*/
}T_RFUSER;

/*
��������ģ���ڲ��Ѵ������Կ�뿨����Կ������֤������ʹ�ø�����ǰ��Ӧ�����á�װ
��IC ����Կ���������Կ�ɹ�����ģ���ڣ����⣬��Ҫ��֤�Ŀ��������Ų�����ģ������
Կ�������
*/
typedef struct
{
	u8 mode;        //0x60������ԿA  0x61������ԿB
	u8 uid[4];      //�����кţ�4 �ֽڣ�
	u8 keyid;       //��Կ���ţ�1 �ֽڣ��� ȡֵ��Χ0��7
	u8 blkid;       //����ţ�1 �ֽڣ���S50��0��63��S70��0��255��PLUS CPU 2K��0��127��PLUS CPU 4K��0��255��
} rfC2E_t;        // E2��Կ��֤��Ϣ�ṹ

typedef struct
{
	u8 blkid;
	u8 dat[16];
} rfC2H_t;    // д������Ϣ�ṹ



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
	uint8	rIdx;		// ����λ������
	uint8	frameOK;	// ֡�����������: 1--������0--û��
	uint8	mode;		// 0--�������  1--�Զ����
	uint8	status;		// 0--û�м�⵽����1--��⵽��
	rfInfo_t	devInfo;	// ����Ϣ
} rfMux_t;



#define RF_HEAD_C1A		0x00410106		// ��ȡ�豸��Ϣ
#define RF_HEAD_C1B		0x00420106		// ����IC���ӿ�
#define RF_HEAD_C1C		0x00430106		// �ر�IC���ӿ�
#define RF_HEAD_C1D		0x01440107		// ����IC���ӿ�Э��
#define RF_HEAD_C1E1	0x0845010E		// װ��IC����Կ��6λ��Կ
#define RF_HEAD_C1E2	0x12450118		// װ��IC����Կ��16λ��Կ
#define RF_HEAD_C1F		0x02460108		// ����IC���ӿڵļĴ���ֵ
#define RF_HEAD_C1G		0x01470107		// ��ȡIC���ӿڵļĴ���ֵ
#define RF_HEAD_C1H		0x01480107		// ���ò�����
#define RF_HEAD_C1I		0x01490107		// ��������������ʽ
#define RF_HEAD_C1K		0x014B0107		// �����¾�֡��ʽ
#define RF_HEAD_C1U		0x02550108		// �����豸����ģʽ
#define RF_HEAD_C1V		0x00560106		// ��ȡ�豸����ģʽ
#define RF_HEAD_C1a		0x11610117		// װ���û���Կ, info: ����+��Կ
#define RF_HEAD_C1b		0x02620108		// ��eeprom, info: addr+len
#define RF_HEAD_C1c		0x00630106		// дeeprom��info: addr+len+data

#define RF_HEAD_C2A		0x01410207		// ����
#define RF_HEAD_C2B1	0x02420208		// ����ײ1
#define RF_HEAD_C2B2	0x0642020C		// ����ײ2
#define RF_HEAD_C2C		0x0543020B		// ��ѡ��
#define RF_HEAD_C2D		0x00440206		// ������
#define RF_HEAD_C2E		0x0745020D		// ��Կ��֤
#define RF_HEAD_C2FA	0x0C460212		// ֱ����ԿA��֤
#define RF_HEAD_C2FB	0x0C460212		// ֱ����ԿB��֤
#define RF_HEAD_C2G		0x01470207		// Mifare����
#define RF_HEAD_C2H		0x11480217		// Mifare��д
#define RF_HEAD_C2I		0x0549020B		// UltraLight����
#define RF_HEAD_C2J		0x074A020D		// Mifareֵ����
#define RF_HEAD_C2L		0x014C0207		// ����λ
#define RF_HEAD_C2M		0x024D0208		// ������
#define RF_HEAD_C2N		0x074E020D		// �Զ���⣬ʹ����ԿE2����֤
#define RF_HEAD_C2O		0x014F0207		// ���Զ��������
#define RF_HEAD_C2P		0x0550020B		// ����ֵ���ֵ
#define RF_HEAD_C2Q		0x01510207		// ��ȡֵ���ֵ
#define RF_HEAD_C2X		0x00580206		// ���ݽ�������, info: data+2B



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

