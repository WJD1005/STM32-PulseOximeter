#include "stm32f10x.h"                  // Device header
#include "PulseOximeter.h"
#include "MAX30102.h"
#include "Algorithm.h"
#include "TFTLCD.h"
#include "Delay.h"

uint16_t FFT_Index = 0;	//FFT输入输出下标

Complex sRED[FFT_NUM + 16];	//红色LED数据FFT输入输出
Complex sIR[FFT_NUM + 16];	//红外LED数据FFT输入输出

uint64_t Offset = 0;

//储存血液信息
_BloodData BloodData;

/**
  * @brief	心率血氧测量初始化
  * @param	无
  * @retval	无
  */
void POM_Init(void)
{
	MAX30102_Init();
}

/**
  * @brief	血液数据更新，写入到FFT输入
  * @param	无
  * @retval	无
  */
void POM_DataUpdate(void)
{
	//下标归零
	FFT_Index = 0;
	//读取数据输入FFT
	while(FFT_Index < FFT_NUM)
	{
		//MAX30102中断线INT，低电平时才能读取
		//不用外部中断是因为一开始第一次可能不会捕捉到下降沿就已经是低电平状态了，需要另加判断
		//并且其实用了外部中断还是要等待数据采集完毕，意义不大
		while(MAX30102_INTRead() == 0)
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
}

void POM_DataConvert(void)
{
	uint16_t i;
	uint16_t RED_Index;
	float Denom;
	float DC_RED = 0;
	float DC_IR = 0;
	float AC_RED = 0;
	float AC_IR = 0;
	float R;
	
	//直流滤波，去直流成分
	for(i=0;i<FFT_NUM;i++)
	{
		DC_RED += sRED[i].Real;
		DC_IR +=  sIR[i].Real;
	}
	DC_RED /= FFT_NUM;
	DC_IR /= FFT_NUM;
	for(i=0;i<FFT_NUM;i++)
	{
		sRED[i].Real = sRED[i].Real - DC_RED;
		sIR[i].Real = sIR[i].Real - DC_IR;
	}
	//移动平均滤波
	for(i=1;i<FFT_NUM-1;i++)
	{
		Denom = (sRED[i-1].Real + 2 * sRED[i].Real + sRED[i+1].Real);
		sRED[i].Real = Denom / 4.00; 
		
		Denom = (sIR[i-1].Real + 2 * sIR[i].Real + sIR[i+1].Real);
		sIR[i].Real = Denom / 4.00; 			
	}
	//八点平均滤波
	for(i=0;i<FFT_NUM-8;i++) 
	{
		Denom = (sRED[i].Real + sRED[i+1].Real + sRED[i+2].Real + sRED[i+3].Real + sRED[i+4].Real + sRED[i+5].Real + sRED[i+6].Real + sRED[i+7].Real);
		sRED[i].Real = Denom / 8.00; 
		
		Denom = (sIR[i].Real + sIR[i+1].Real + sIR[i+2].Real + sIR[i+3].Real + sIR[i+4].Real + sIR[i+5].Real + sIR[i+6].Real + sIR[i+7].Real);
		sIR[i].Real = Denom / 8.00; 	
	}
	//傅里叶变换
	FFT(sRED, FFT_NUM);
	FFT(sIR, FFT_NUM);
	//开平方
	for(i=0;i<FFT_NUM;i++) 
	{
		sRED[i].Real = Sqrt(sRED[i].Real * sRED[i].Real + sRED[i].Imag * sRED[i].Imag);
		sIR[i].Real = Sqrt(sIR[i].Real * sIR[i].Real + sIR[i].Imag * sIR[i].Imag);
	}
	//计算交流分量
	for (i=1;i<FFT_NUM;i++) 
	{
		AC_RED += sRED[i].Real ;
		AC_IR += sIR[i].Real ;
	}
	//读取峰值点横坐标，即信号频率
	//红色和红外数据的频率基本一致，读信号比较好的即可，范围也不用太大
	RED_Index = Find_MaxIndex(sRED, 30, START_INDEX);
	//计算心率
	BloodData.PR = 60.00 * ((SAMPLE_PER_SECOND * RED_Index) / (float)FFT_NUM);
	//计算血氧
	R = (AC_IR * DC_RED) / (AC_RED * DC_IR);
	BloodData.SpO2 = -45.060 * R * R+ 30.354 * R + 94.845;
}

void POM_GetBloodData(void)
{
	POM_DataUpdate();
	POM_DataConvert();
}

void POM_WaitInput(void)
{
	while(FIFO_RED == 0 && FIFO_RED == 0)	//等待有效输入
	{
		while(MAX30102_INTRead() == 0)
		{
			//读取FIFO
			MAX30102_ReadFIFO();
		}
	}
	Delay_s(2);	//过滤初始不稳定信号
}

