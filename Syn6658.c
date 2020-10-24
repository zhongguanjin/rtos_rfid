#include "Syn6658.h"
#include "string.h"
#include  "dbg.h"
#include "delay.h"


char code_text[] = {"欢迎使用读写器"};
uint8 length;

uint8 rot_val;//回转数据





SynMux_t  SynMux;

uint8 Syn6658_Init(void);

/*****************************************************************************
 函 数 名  : Syn6658_Play
 功能描述  : 语音合成播放命令
 输入参数  : char *buf --待合成的文本
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年9月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Syn6658_Play(char *buf,uint8 len)
{
	  uint8 i=0;
      SynMux.cmdType = 0x01;
      SynMux.cmdval = 0x00;
       SynMux.lenL= len+2;
      for(i=0;i<len;i++)
      {
          SynMux.info[i] = buf[i];
      }
      com_send(COM2,SynMux.Buf, SynMux.lenL+3); //中断发送
}

/*****************************************************************************
 函 数 名  : Syn6658_Cmd
 功能描述  : 命令请求
 输入参数  : uint8 cmd
             stopcmd         0x02    //停止合成请求
             pausecmd        0x03    //暂停合成请求
             querycmd        0x21   //状态查询
             standbycmd      0x22   //待机请求
             awakencmd       0xFF   //唤醒请求
             recovercmd      0x04   //恢复合成请求
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年9月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Syn6658_Cmd(uint8 cmd)
{
     SynMux.lenL =0x01;
     SynMux.cmdType = cmd;
    com_send(COM2,SynMux.Buf, SynMux.lenL+3); //中断发送
}



uint8 Syn6658_Init(void)
{
  uint8 times = 5;
  SynMux.frameHead = 0xfd;
  Syn6658_Cmd(awakencmd);
  delay_ms(1000);
  while(times-- != 0)
  {
    if(rot_val==Frame_Ok_val)
    {
        return OK;
    }
  }
  return ERR;
}


void syn6658_check(void)
{
    if(Syn6658_Init() == OK)
    {
        Syn6658_Play(code_text,strlen(code_text));
        dbg("syn6658 init ok");
    }
    else
    {
        dbg("syn6658 init err");
    }
}

uint8 get_rspcmd(uint8 dat)
{
    rot_val=dat;
    return rot_val;
}

void com2_rxDeal(void)
{
	char ch;
	if(com_rxLeft(COM2) != 0)
	{
		while(1)
		{
			if(OK == com_getch(COM2,&ch))
			{
                rot_val=ch;
                //dbg("rot_val:%x",rot_val);
			}
			else
			{
				break;
			}
		}
	}
}

