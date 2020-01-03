#include "eeprom.h"

 /**
  * @brief  IIC EEPROM  ����,������������
  * @param  ��
  * @retval ��
  */
void I2C_EE_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef  I2C_InitStructure;
	// ��IIC GPIO��ʱ��
	EEPROM_I2C_GPIO_APBxClkCmd(EEPROM_I2C_SCL_GPIO_CLK|EEPROM_I2C_SDA_GPIO_CLK, ENABLE);

	// ��IIC �����ʱ��
	EEPROM_I2C_APBxClkCmd(EEPROM_I2C_CLK, ENABLE);

	// ��IIC SCL��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = EEPROM_I2C_SCL_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(EEPROM_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);
  // ��IIC SDA��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = EEPROM_I2C_SDA_GPIO_PIN;
	GPIO_Init(EEPROM_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);

	// ����IIC�Ĺ�������
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable  ;//ʹ��Ӧ��
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit ;//ʹ��7λ��ַģʽ
	I2C_InitStructure.I2C_ClockSpeed = EEPROM_I2_BAUDRATE; //����SCLʱ��Ƶ��
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2 ;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C ;
	I2C_InitStructure.I2C_OwnAddress1 = STM32_I2C_OWN_ADDR; //����STM32 IIC�����豸��ַ��ֻҪ��������Ψһ����

	I2C_Init(EEPROM_I2C,&I2C_InitStructure);

	// ʹ�ܴ���
	I2C_Cmd (EEPROM_I2C, ENABLE);
}


//��EEPROMд��һ���ֽ�

void EEPROM_Byte_Write(u16 addr,u8 data)
{

	//������ʼ�ź�
	I2C_GenerateSTART(EEPROM_I2C,ENABLE);

	while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

	//EV5�¼�����⵽�������豸��ַ
	I2C_Send7bitAddress(EEPROM_I2C,EEPROM_ADDR,I2C_Direction_Transmitter);

  while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) == ERROR);

	//EV6�¼�����⵽������Ҫ�����Ĵ洢��Ԫ��ַ
	//I2C_SendData (EEPROM_I2C,addr);
	I2C_SendData (EEPROM_I2C,(addr&0xff00)>>8); //

	while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_BYTE_TRANSMITTING ) == ERROR);

   I2C_SendData (EEPROM_I2C,addr&0x00ff); //
	while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_BYTE_TRANSMITTING ) == ERROR);
  //EV8�¼�����⵽������Ҫ�洢������
	I2C_SendData (EEPROM_I2C,data);

	while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_BYTE_TRANSMITTED ) == ERROR);

	//���ݴ������
	I2C_GenerateSTOP(EEPROM_I2C,ENABLE);

}



//��EEPROMд�����ֽڣ�ҳд�룩��ÿ��д�벻�ܳ���8���ֽ�

void EEPROM_Page_Write(u16 addr,u8 *data,u8 len)
{
	//������ʼ�ź�
	I2C_GenerateSTART(EEPROM_I2C,ENABLE);

	while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

	//EV5�¼�����⵽�������豸��ַ
	I2C_Send7bitAddress(EEPROM_I2C,EEPROM_ADDR,I2C_Direction_Transmitter);

  while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) == ERROR);

	//EV6�¼�����⵽������Ҫ�����Ĵ洢��Ԫ��ַ
	I2C_SendData (EEPROM_I2C,(addr&0xff00)>>8); //?????
	while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_BYTE_TRANSMITTING ) == ERROR);
   I2C_SendData (EEPROM_I2C,addr&0x00ff); //?????
	while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_BYTE_TRANSMITTING ) == ERROR);


	while(len)
	{
		//EV8�¼�����⵽������Ҫ�洢������
		I2C_SendData (EEPROM_I2C,*data);

		while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_BYTE_TRANSMITTED ) == ERROR);

		data++;
		len--;

	}
	//���ݴ������
	I2C_GenerateSTOP(EEPROM_I2C,ENABLE);

}


void EEPROM_Write(u16 addr,u8 *data,u8 len)
{
    u8 i;
    for( i=0;i<len;i++)
    {
        EEPROM_Byte_Write(addr,*data);
        EEPROM_WaitForWriteEnd();
        addr++;
        data++;
    }
}
//��EEPROM��ȡ����

void EEPROM_Read(u16 addr,u8 *data,u8 len)
{
	//������ʼ�ź�
	I2C_GenerateSTART(EEPROM_I2C,ENABLE);

	while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

	//EV5�¼�����⵽�������豸��ַ
	I2C_Send7bitAddress(EEPROM_I2C,EEPROM_ADDR,I2C_Direction_Transmitter);

  while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) == ERROR);

	//EV6�¼�����⵽������Ҫ�����Ĵ洢��Ԫ��ַ
	I2C_SendData (EEPROM_I2C,(addr&0xff00)>>8); //?????
	while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_BYTE_TRANSMITTING ) == ERROR);
   I2C_SendData (EEPROM_I2C,addr&0x00ff); //?????
	while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_BYTE_TRANSMITTING ) == ERROR);


	//�ڶ�����ʼ�ź�
	//������ʼ�ź�
	I2C_GenerateSTART(EEPROM_I2C,ENABLE);

	while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

	//EV5�¼�����⵽�������豸��ַ
	I2C_Send7bitAddress(EEPROM_I2C,EEPROM_ADDR,I2C_Direction_Receiver);

  while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ) == ERROR);


	while(len)
	{
		if(len == 1)
		{
			//���Ϊ���һ���ֽ�
			I2C_AcknowledgeConfig (EEPROM_I2C,DISABLE);
		}

		//EV7�¼�����⵽
		while(I2C_CheckEvent(EEPROM_I2C,I2C_EVENT_MASTER_BYTE_RECEIVED ) == ERROR);

		//EV7�¼�����⵽�������ݼĴ������µ���Ч����
		*data = I2C_ReceiveData(EEPROM_I2C);

		data++;

		len--;

	}


	//���ݴ������
	I2C_GenerateSTOP(EEPROM_I2C,ENABLE);

	//��������ACKʹ�ܣ��Ա��´�ͨѶ
	I2C_AcknowledgeConfig (EEPROM_I2C,ENABLE);

}


//�ȴ�EEPROM�ڲ�ʱ�����
void EEPROM_WaitForWriteEnd(void)
{

	do
	{
		//������ʼ�ź�
		I2C_GenerateSTART(EEPROM_I2C,ENABLE);

		while(I2C_GetFlagStatus (EEPROM_I2C,I2C_FLAG_SB) == RESET);

		//EV5�¼�����⵽�������豸��ַ
		I2C_Send7bitAddress(EEPROM_I2C,EEPROM_ADDR,I2C_Direction_Transmitter);
	}
	while(I2C_GetFlagStatus (EEPROM_I2C,I2C_FLAG_ADDR) == RESET );

	//EEPROM�ڲ�ʱ����ɴ������
	I2C_GenerateSTOP(EEPROM_I2C,ENABLE);
}

