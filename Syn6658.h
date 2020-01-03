#ifndef _SYN6658_H_
#define _SYN6658_H_

#include "config.h"

#include "com.h"

#define syn6658_com  COM2


#define SYN_BUF_MAX  128

#define Init_Ok_val   0x4A   //初始化成功回传
#define Frame_Ok_val  0x41   //收到正确的命令帧回传
#define Frame_Err_val  0x45  //收到不能识别命令帧回传
#define Play_St_val  0x4E //芯片播音状态回传
#define Idle_St_val  0x4F //芯片空闲状态回传

#define stopcmd         0x02    //停止合成请求
#define pausecmd        0x03    //暂停合成请求
#define querycmd        0x21   //状态查询
#define standbycmd      0x22   //待机请求
#define awakencmd       0xFF   //唤醒请求
#define recovercmd      0x04   //恢复合成请求



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

