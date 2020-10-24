#include "Syn6658.h"
#include "string.h"
#include  "dbg.h"
#include "delay.h"


char code_text[] = {"��ӭʹ�ö�д��"};
uint8 length;

uint8 rot_val;//��ת����





SynMux_t  SynMux;

uint8 Syn6658_Init(void);

/*****************************************************************************
 �� �� ��  : Syn6658_Play
 ��������  : �����ϳɲ�������
 �������  : char *buf --���ϳɵ��ı�
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��9��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
      com_send(COM2,SynMux.Buf, SynMux.lenL+3); //�жϷ���
}

/*****************************************************************************
 �� �� ��  : Syn6658_Cmd
 ��������  : ��������
 �������  : uint8 cmd
             stopcmd         0x02    //ֹͣ�ϳ�����
             pausecmd        0x03    //��ͣ�ϳ�����
             querycmd        0x21   //״̬��ѯ
             standbycmd      0x22   //��������
             awakencmd       0xFF   //��������
             recovercmd      0x04   //�ָ��ϳ�����
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��9��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Syn6658_Cmd(uint8 cmd)
{
     SynMux.lenL =0x01;
     SynMux.cmdType = cmd;
    com_send(COM2,SynMux.Buf, SynMux.lenL+3); //�жϷ���
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

