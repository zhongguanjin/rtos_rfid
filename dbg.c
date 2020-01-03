#include "dbg.h"
#include "delay.h"
#include "mycfg.h"
#include "com.h"
#include "console.h"




char *itoa_my(long value,char *string,int radix);
void printhex (const unsigned int x);

/* ���� �� -radix =10 ��ʾ 10 ���ƣ��������Ϊ 0
 * -value Ҫת����������
 * -string ת������ַ���
 * -radix = 10
 * ��� ����
 * ���� ����
*/
char *itoa_my(long value,char *string,int radix)
{
    char zm[37]="0123456789abcdefghijklmnopqrstuvwxyz";
    char aa[80]={0};
    char *cp=string;
    int i=0,j=0;
    if(radix<2||radix>36)//�����˶Դ���ļ��
    {
        return string;
    }
    if(value<0)
    {
        return string;
    }
    else if(value ==0)
    {
        *cp=0x30;
        cp++;
        *cp=0x30;
        cp++;
        *cp='\0';
        return string;
    }
    if((value<=0x0f)&&(value>0))
    {
        aa[i++]=zm[value%radix];
        aa[i++]=zm[0];
    }
    if (value>0x0f)
    {
        long temp =value;
        while(temp>0)
        {

            aa[i++]=zm[temp%radix];
            temp/=radix;
        }
    }
    for(j=i-1;j>=0;j--)
    {
        *cp++=aa[j];
    }
    *cp='\0';
    return string;
}

//16�������
void printhex (const unsigned int x)
{
    char *s;
    char buf[16];
    itoa_my( x, buf,  16);
    for(s=buf;*s;s++)
    {
        USART_SendData(console_uart,*s);
        while (USART_GetFlagStatus(console_uart, USART_FLAG_TXE) == RESET);
    }

}

 void dbg_hex(uint8 *buf,uint16 len)
 {
     uint16 i =0;
     //CTR_485(ON);
     //delay_ms(2);
     for( i=0;i<len;i++)
     {
         printhex(*buf++);
         USART_SendData(console_uart,' ');
         while (USART_GetFlagStatus(console_uart, USART_FLAG_TXE) == RESET);
     }
     printf("\r\n");
     //delay_ms(1);
     //CTR_485(OFF);
 }

 //�ض���printf����
 int fputc(int ch, _Filet *f)
 {
     //CTR_485(ON);
     //delay_ms(2);
     USART_SendData(console_uart, ch);
     while (USART_GetFlagStatus(console_uart, USART_FLAG_TXE) == RESET);
     //delay_ms(1);
     //CTR_485(OFF);
     return ch;
 }

 /*****************************************************************************
  �� �� ��  : CRC8_SUM
  ��������  : CRCУ�麯��
  �������  : void *p
              uint8 len
  �������  : crc8        --check sum
  �� �� ֵ  :
  ���ú���  :
  ��������  :

  �޸���ʷ      :
   1.��    ��   : 2017��6��30�� ������
     ��    ��   : zgj
     �޸�����   : �����ɺ���

 *****************************************************************************/
 uint8 CRC8_SUM(void *p,uint8 len)
 {
     uint8 crc8 = 0;
     uint8 *temp =p;
     for(uint8 i=0;i<len;i++)
     {
         crc8 ^=*temp;
         temp++;
     }
     return crc8;
 }





