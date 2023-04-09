#include "stm32f10x.h"                  // Device header
#include "MAX30102.h"
#include "IIC.h"

//MAX30102的FIFO缓存器的数据，由MAX30102_ReadFIFO()读取
uint16_t FIFO_RED;	//红光LED
uint16_t FIFO_IR;	//红外LED

/**
  * @brief	MAX30102初始化
  * @param	无
  * @retval	无
  */
void MAX30102_Init(void)
{
	//初始化INT（PA1）为浮空输入
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//初始化IIC（PA9、PA10）
	IIC_Init();
	
	//重置
	MAX30102_Reset();
	//配置
	MAX30102_Config();
}

/**
  * @brief	MAX30102写一字节
  * @param	WordAddr: 写入地址
  * @param	Data: 写入数据
  * @retval	无
  */
void MAX30102_WriteByte(uint8_t WordAddr, uint8_t Data)
{
	IIC_Start();
	IIC_SendByte(MAX30102_Address);
	IIC_ReceiveAck();
	IIC_SendByte(WordAddr);	//写地址
	IIC_ReceiveAck();
	IIC_SendByte(Data);
	IIC_ReceiveAck();
	IIC_Stop();
}

/**
  * @brief	MAX30102读一字节（未配置应答，调用需要根据需要决定收到后是否应答）
  * @param	WordAddr: 读取地址
  * @retval	读取的数据
  */
uint8_t MAX30102_ReadByte(uint8_t WordAddr)
{
	uint8_t Data;
	IIC_Start();
	IIC_SendByte(MAX30102_Address);
	IIC_ReceiveAck();
	IIC_SendByte(WordAddr);
	IIC_ReceiveAck();
	IIC_Start();
	IIC_SendByte(WordAddr + 1);
	IIC_ReceiveAck();
	Data = IIC_ReceiveByte();
	IIC_Stop();
	return Data;
}

/**
  * @brief	MAX30102写入数组
  * @param	WordAddr: 写入地址
  * @param	*Data: 数组地址
  * @param	Num: 元素数量，范围：0~255
  * @retval	无
  */
void MAX30102_WriteArray(uint8_t WordAddr, uint8_t *Data, uint8_t Num)
{
	uint8_t i;
	IIC_Start();
	IIC_SendByte(MAX30102_Address);
	IIC_ReceiveAck();
	IIC_SendByte(WordAddr);
	IIC_ReceiveAck();
	IIC_Start();
	IIC_SendByte(MAX30102_Address + 1);
	IIC_ReceiveAck();
	for(i=0;i<Num;i++)
	{
		IIC_SendByte(*Data++);
		IIC_ReceiveAck();
	}
	IIC_Stop();
}

/**
  * @brief	MAX30102读取数组
  * @param	WordAddr: 读取地址
  * @param	*Data: 数组地址
  * @param	Num: 元素数量，范围：0~255
  * @retval	无
  */
void MAX30102_ReadArray(uint8_t WordAddr, uint8_t *Data, uint8_t Num)
{
	uint8_t i;
	IIC_Start();
	IIC_SendByte(MAX30102_Address);
	IIC_ReceiveAck();
	IIC_SendByte(WordAddr);
	IIC_ReceiveAck();
	IIC_Start();
	IIC_SendByte(MAX30102_Address + 1);
	IIC_ReceiveAck();
	for(i=0;i<Num;i++)
	{
		*Data++ = IIC_ReceiveByte();
		if(i == Num - 1)
		{
			IIC_SendAck(1);	//最后一次读取不应答
		}
		else
		{
			IIC_SendAck(0);	//前面要应答
		}
	}
	IIC_Stop();
}

/**
  * @brief	MAX30102重置
  * @param	无
  * @retval	无
  */
void MAX30102_Reset(void)
{
	MAX30102_WriteByte(REG_MODE_CONFIG, 0x40);
}

/**
  * @brief	MAX30102配置
  * @param	无
  * @retval	无
  */
void MAX30102_Config(void)
{
	MAX30102_WriteByte(REG_INTR_ENABLE_1, 0xc0);//// INTR setting
	MAX30102_WriteByte(REG_INTR_ENABLE_2, 0x00);//
	MAX30102_WriteByte(REG_FIFO_WR_PTR, 0x00);//FIFO_WR_PTR[4:0]
	MAX30102_WriteByte(REG_OVF_COUNTER, 0x00);//OVF_COUNTER[4:0]
	MAX30102_WriteByte(REG_FIFO_RD_PTR, 0x00);//FIFO_RD_PTR[4:0]
	MAX30102_WriteByte(REG_FIFO_CONFIG, 0x0f);//sample avg = 1, fifo rollover=false, fifo almost full = 17
	MAX30102_WriteByte(REG_MODE_CONFIG, 0x03);//0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
	MAX30102_WriteByte(REG_SPO2_CONFIG, 0x27);	// SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)  
	MAX30102_WriteByte(REG_LED1_PA, 0x32);//Choose value for ~ 10mA for LED1
	MAX30102_WriteByte(REG_LED2_PA, 0x32);// Choose value for ~ 10mA for LED2
	MAX30102_WriteByte(REG_PILOT_PA, 0x7f);// Choose value for ~ 25mA for Pilot LED
}

/**
  * @brief	MAX30102读取FIFO缓存器中的数据，读到全局变量中
  * @param	无
  * @retval	无
  */
void MAX30102_ReadFIFO(void)
{
	uint16_t Temp;
	uint8_t Data[6];
	FIFO_RED = 0;
	FIFO_IR = 0;
  
	//读状态寄存器并清除
	MAX30102_ReadByte(REG_INTR_STATUS_1);
	MAX30102_ReadByte(REG_INTR_STATUS_2);
	//初始化数组
	Data[0] = REG_FIFO_DATA;
	//读取FIFO中的6个数据
	MAX30102_ReadArray(REG_FIFO_DATA, Data, 6);
	//对RED数据进行组合
	Temp = Data[0];
	Temp <<= 14;
	FIFO_RED += Temp;
	Temp = Data[1];
	Temp <<= 6;
	FIFO_RED += Temp;
	Temp = Data[2];
	Temp >>= 2;
	FIFO_RED += Temp;
	//对IR数据进行组合
	Temp = Data[3];
	Temp <<= 14;
	FIFO_IR += Temp;
	Temp = Data[4];
	Temp <<= 6;
	FIFO_IR += Temp;
	Temp = Data[5];
	Temp >>= 2;
	FIFO_IR += Temp;
	//无效数据归零
	if(FIFO_IR <= 10000)
	{
		FIFO_IR = 0;
	}
	if(FIFO_RED <= 10000)
	{
		FIFO_RED = 0;
	}
}
