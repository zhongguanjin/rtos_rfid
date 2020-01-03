#ifndef __EEPROM_H
#define	__EEPROM_H


#include "stm32f10x.h"

//EEPROM �豸��ַ��8λ��
#define EEPROM_ADDR   0xA0

/**
* IIC EERPOM�����Ŷ���
*
*/
#define STM32_I2C_OWN_ADDR     0x5f


// IIC
#define  EEPROM_I2C                   I2C1
#define  EEPROM_I2C_CLK               RCC_APB1Periph_I2C1
#define  EEPROM_I2C_APBxClkCmd       RCC_APB1PeriphClockCmd
#define  EEPROM_I2_BAUDRATE           400000

//IIC GPIO ���ź궨��
#define  EEPROM_I2C_SCL_GPIO_CLK           (RCC_APB2Periph_GPIOB)
#define  EEPROM_I2C_SDA_GPIO_CLK           (RCC_APB2Periph_GPIOB)

#define  EEPROM_I2C_GPIO_APBxClkCmd    RCC_APB2PeriphClockCmd

#define  EEPROM_I2C_SCL_GPIO_PORT         GPIOB
#define  EEPROM_I2C_SCL_GPIO_PIN          GPIO_Pin_6

#define  EEPROM_I2C_SDA_GPIO_PORT       GPIOB
#define  EEPROM_I2C_SDA_GPIO_PIN        GPIO_Pin_7


void I2C_EE_Config(void);
void EEPROM_Byte_Write(u16 addr,u8 data);
void EEPROM_Read(u16 addr,u8 *data,u8 len);
void EEPROM_Page_Write(u16 addr,u8 *data,u8 len);
void EEPROM_WaitForWriteEnd(void);
void EEPROM_Write(u16 addr,u8 *data,u8 len);


#endif /* __BSP_I2C_H */
