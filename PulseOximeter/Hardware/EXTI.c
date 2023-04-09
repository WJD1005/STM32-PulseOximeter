#include "stm32f10x.h"                  // Device header
#include "MAX30102.h"
#include "PulseOximeter.h"

/**
  * @brief	初始化INT（PA1）外部中断，用于读取MAX30102数据
  * @param	无
  * @retval	无
  */
void INT_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);	//开启AFIO时钟
	
	//初始化PA1为浮空输入
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);	//配置AFIO使PA1接入EXTI
	
	//配置EXTI
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line5;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;	//下降沿触发，因为MAX30102可读取时中断线为低电平
	EXTI_Init(&EXTI_InitStructure);
	
	//配置NVIC
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//分组2，2位抢占优先级和2位响应优先级
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
}

void EXTI1_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line1) == SET)
	{
		//读取FIFO
		MAX30102_ReadFIFO();
		if(FFT_Index < FFT_NUM)
		{
			//输入FFT数据
			sRED[FFT_Index].Real = FIFO_RED;
			sRED[FFT_Index].Imag = 0;
			sIR[FFT_Index].Real = FIFO_IR;
			sIR[FFT_Index].Imag = 0;
			FFT_Index++;
		}
	}
}

