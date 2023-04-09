#include "stm32f10x.h"                  // Device header
#include "TFTLCD.h"
#include "BigNumber.h"
#include "PulseOximeter.h"

int main(void)
{
	LCD_Init(0);
	BACK_COLOR = BLACK;
	
	//心率
	POINT_COLOR = GREEN;
	LCD_ShowString(58, 116, 60, 24, "PRbpm", 24, 0);
	LCD_ShowBigEmpty(22, 140, 64, 0);
	LCD_ShowBigEmpty(54, 140, 64, 0);
	LCD_ShowBigEmpty(86, 140, 64, 0);
	//血氧
	POINT_COLOR = LBBLUE;
	LCD_ShowString(146, 116, 48, 24, "%SpO", 24, 0);
	LCD_ShowChar(194, 124, '2', 16, 0);
	LCD_ShowBigEmpty(138, 140, 64, 0);
	LCD_ShowBigEmpty(170, 140, 64, 0);
	
	
	POM_Init();
	POM_WaitInput();

	while(1)
	{
		POM_GetBloodData();
		POINT_COLOR = GREEN;
		LCD_ShowBigNum(22, 140, BloodData.PR, 3, 64, 0x00);
		POINT_COLOR = LBBLUE;
		LCD_ShowBigNum(138, 140, BloodData.SpO2, 2, 64, 0x00);
	}
}
