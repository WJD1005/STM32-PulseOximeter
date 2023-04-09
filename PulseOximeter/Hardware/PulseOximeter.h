#ifndef __PULSEOXIMETER_H
#define __PULSEOXIMETER_H

//由MAX30102_Config()配置时直接对MAX30102进行采样率设置，目前配置为100Hz
//该宏只用于修改算法中的参数，不能修改实际采样率
#define SAMPLE_PER_SECOND	100

//FFT参数
#define FFT_NUM			512		//FFT数据个数
#define START_INDEX		4		//低频过滤阈值，寻峰函数Find_MaxIndex()索引起点，过滤FFT后的低频峰（直流成分引起的）

//血液信息结构体
typedef struct
{
	int PR;		//脉率
	float SpO2;	//血氧饱和度
}_BloodData;

//数据获取
extern _BloodData BloodData;

void POM_Init(void);
void POM_DataUpdate(void);
void POM_DataConvert(void);
void POM_GetBloodData(void);
void POM_WaitInput(void);

#endif
