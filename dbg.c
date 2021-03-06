#include "dbg.h"
#include "delay.h"
#include "mycfg.h"
#include "com.h"
#include "console.h"





char *itoa_my(long value,char *string,int radix);
void printhex (const unsigned int x);

/* 输入 ： -radix =10 表示 10 进制，其他结果为 0
 * -value 要转换的整形数
 * -string 转换后的字符串
 * -radix = 10
 * 输出 ：无
 * 返回 ：无
*/
char *itoa_my(long value,char *string,int radix)
{
    char zm[37]="0123456789abcdefghijklmnopqrstuvwxyz";
    char aa[80]={0};
    char *cp=string;
    int i=0,j=0;
    if(radix<2||radix>36)//增加了对错误的检测
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

//16进制输出
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

 //重定义printf函数
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
  函 数 名  : CRC8_SUM
  功能描述  : CRC校验函数
  输入参数  : void *p
              uint8 len
  输出参数  : crc8        --check sum
  返 回 值  :
  调用函数  :
  被调函数  :

  修改历史      :
   1.日    期   : 2017年6月30日 星期五
     作    者   : zgj
     修改内容   : 新生成函数

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

#if 0
 //SemHandle_t dbgSem;
 void DMA1_Channel4_IRQHandler (void)
 {
     DMA1->IFCR = DMA1->ISR & 0x0000F000;
     DMA_Cmd(DMA1_Channel4,DISABLE);
     //sem_postIsr(dbgSem);
    // printf("dma ok\r\n");
 }


 void dbg_out(char * buf, u32 len)
 {
     /* DMA1 channel4 configuration */  // DMA_Channel_TypeDef
     u32 tmpreg = DMA1_Channel4->CCR & 0xFFFF800F;

     RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
     DMA_DeInit(DMA1_Channel4);
     tmpreg |=   DMA_DIR_PeripheralDST | DMA_Mode_Normal|
                 DMA_PeripheralInc_Disable | DMA_MemoryInc_Enable |
                 DMA_PeripheralDataSize_Byte | DMA_MemoryDataSize_Byte |
                 DMA_Priority_High | DMA_M2M_Disable;    // | 0x0000000F
     DMA1_Channel4->CCR |= tmpreg;
     DMA1_Channel4->CNDTR = len;
     DMA1_Channel4->CPAR = (u32)&(USART1->DR);   // USART_TypeDef
     DMA1_Channel4->CMAR = (u32)(buf);

     /* Enable DMA Channel4Transfer Complete interrupt */
     DMA_ITConfig(DMA1_Channel4,DMA_IT_TC|DMA_IT_TE, ENABLE);
     DMA_Cmd(DMA1_Channel4,ENABLE);

     // 采用DMA方式发送
     USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);
     // 传输完成则进入DMA2_Channel4中断;
 }

#endif


