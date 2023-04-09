#include "stm32f10x.h"                  // Device header
#include "IIC.h"
#include "Delay.h"

//IIC通信速度未优化

GPIO_InitTypeDef GPIO_InitStructure;

/**
  * @brief	软件IIC初始化，SCL->PA9，SDA->PA10
  * @param	无
  * @retval	无
  */
void IIC_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA时钟
	
	//PA9、PA10配置为推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	IIC_SCL(1);
	IIC_SDA(1);
}

/**
  * @brief	SDA设置为推挽输出模式，发送用
  * @param	无
  * @retval	无
  */
void IIC_SDAOutput(void)
{
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief	SDA设置为上拉输入模式，接收用
  * @param	无
  * @retval	无
  */
void IIC_SDAInput(void)
{
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief	IIC开始
  * @param	无
  * @retval	无
  */
void IIC_Start(void)
{
	IIC_SDAOutput();
	
	IIC_SDA(1);
	IIC_SCL(1);
	Delay_us(4);
	IIC_SDA(0);
	Delay_us(4);
	IIC_SCL(0);
}

/**
  * @brief	IIC结束
  * @param	无
  * @retval	无
  */
void IIC_Stop(void)
{
	IIC_SDAOutput();
	
	IIC_SCL(0);
	IIC_SDA(0);
	Delay_us(4);
	IIC_SCL(1);
	IIC_SDA(1);
	Delay_us(4);
}

/**
  * @brief	IIC发送一字节
  * @param	Byte: 一字节数据
  * @retval	无
  */
void IIC_SendByte(uint8_t Byte)
{
	uint8_t i;
	
	IIC_SDAOutput();
	
	for(i=0;i<8;i++)
	{
		IIC_SDA(Byte & (0x80 >> i));
		Delay_us(2);
		IIC_SCL(1);
		Delay_us(2);
		IIC_SCL(0);
		Delay_us(2);
	}
}

/**
  * @brief	IIC接收一字节
  * @param	无
  * @retval	接收的一字节数据
  */
uint8_t IIC_ReceiveByte(void)
{
	uint8_t i, Byte = 0x00;
	
	IIC_SDAInput();
	
	for(i=0;i<8;i++)
	{
		IIC_SCL(1);
		Delay_us(2);
		if(IIC_ReadSDA)
		{
			Byte |= (0x80 >> i);
		}
		IIC_SCL(0);
		Delay_us(2);
	}
	return Byte;
}

/**
  * @brief	IIC发送应答位
  * @param	Ack: 应答位，0为应答，1为非应答
  * @retval	无
  */
void IIC_SendAck(uint8_t Ack)
{
	IIC_SDAOutput();
	
	IIC_SDA(Ack);
	Delay_us(2);
	IIC_SCL(1);
	Delay_us(2);
	IIC_SCL(0);
}

/**
  * @brief	IIC接收应答位
  * @param	无
  * @retval	接收应答位，0为应答，1为非应答
  */
uint8_t IIC_ReceiveAck(void)
{
	uint8_t Ack;
	
	IIC_SDAInput();
	
	IIC_SDA(1);
	Delay_us(1);
	IIC_SCL(1);
	Delay_us(1);
	Ack = IIC_ReadSDA;
	IIC_SCL(0);
	return Ack;
}
