#ifndef _SYN6658_H_
#define _SYN6658_H_

#include "config.h"

#include "com.h"

#define syn6658_com  COM2


#define SYN_BUF_MAX  128

#define Init_Ok_val   0x4A   //��ʼ���ɹ��ش�
#define Frame_Ok_val  0x41   //�յ���ȷ������֡�ش�
#define Frame_Err_val  0x45  //�յ�����ʶ������֡�ش�
#define Play_St_val  0x4E //оƬ����״̬�ش�
#define Idle_St_val  0x4F //оƬ����״̬�ش�

#define stopcmd         0x02    //ֹͣ�ϳ�����
#define pausecmd        0x03    //��ͣ�ϳ�����
#define querycmd        0x21   //״̬��ѯ
#define standbycmd      0x22   //��������
#define awakencmd       0xFF   //��������
#define recovercmd      0x04   //�ָ��ϳ�����



typedef  union
{
      struct
      {
        uint8  frameHead;
        uint8  lenH;
        uint8  lenL;
        uint8  cmdType;
        uint8  cmdval;
        uint8  info[SYN_BUF_MAX-5];   //123 byte
      };
      uint8 Buf[SYN_BUF_MAX];
} SynMux_t;


extern void Syn6658_Play(char *buf,uint8 len);
extern void Syn6658_Cmd(uint8 cmd);
extern void syn6658_check(void);
extern uint8 get_rspcmd(uint8 dat);
#endif

