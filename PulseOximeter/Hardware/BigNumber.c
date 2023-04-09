#include "stm32f10x.h"                  // Device header
#include "TFTLCD.h"
#include "BigNumberFont.h"

/**
  * @brief	LCD显示一个大数字函数
  * @param	x: 字符左上角顶点列地址
  * @param	y: 字符左上角顶点页地址
  * @param	Num: 数字
  * @param	Size: 字符大小（64）
  * @param	Mode: 0，非叠加模式；1，叠加模式
  * @retval	无
  */
void LCD_ShowSingleBigNum(uint16_t x, uint16_t y, uint8_t Num, uint8_t Size, uint8_t Mode)
{
	uint8_t Temp, j;
	uint16_t i;
	uint16_t cSize = (Size / 8 + ((Size % 8) ? 1 : 0)) * (Size / 2);
	uint16_t y0 = y;
	
	for(i=0;i<cSize;i++)		//取字节
	{
		if(Size == 64)			//6432字库
		{
			Temp = Number_6432[Num][i];
		}
		else
		{
			return;
		}
		
		for(j=0;j<8;j++)		//字节遍历
		{
			if(Mode == 0)		//非叠加
			{
				if(Temp & (0x80 >> j))
				{
					LCD_Fast_DrawPoint(x, y, POINT_COLOR);
				}
				else
				{
					LCD_Fast_DrawPoint(x, y, BACK_COLOR);
				}
			}
			else if(Mode == 1)	//叠加
			{
				if(Temp & (0x80 >> j))
				{
					LCD_Fast_DrawPoint(x, y, POINT_COLOR);
				}
			}
			y++;
			if(y >= LCD_Dev.Height)		//y超出范围
			{
				return;
			}
			if((y - y0) == Size)		//列画完
			{
				y = y0;
				x++;
				if(x >= LCD_Dev.Width)	//x超出范围
				{
					return;
				}
				break;					//到了字模设定大小就完了，直接跳出剩余的补0
			}
		}
	}
}

/**
  * @brief	乘方函数
  * @param	n: 底数
  * @param	m: 指数
  * @retval	n^m
  */
uint32_t BN_Pow(uint8_t n, uint8_t m)
{
	uint32_t Result = 1;
	
	for(;m>0;m--)
	{
		Result *= n;
	}
	return Result;
}

/**
  * @brief	LCD显示一个空格
  * @param	x: 字符左上角顶点列地址
  * @param	y: 字符左上角顶点页地址
  * @param	Size: 字符大小（64）
  * @param	Mode: 0，非叠加模式；1，叠加模式
  * @retval	无
  */
void LCD_ShowBigSpace(uint16_t x, uint16_t y, uint8_t Size, uint8_t Mode)
{
	//叠加不用画空格
	if(Mode == 1)
	{
		return;
	}
	//不叠加画空格
	LCD_Fill(x, y, Size / 2, Size, BACK_COLOR);
	//修改回全屏
	LCD_SetWindow(0, 0, LCD_Dev.Width, LCD_Dev.Height);
}

/**
  * @brief	LCD显示大数字非负整数
  * @param	x: 显示区域左上角顶点列坐标
  * @param	y: 显示区域左上角顶点页坐标
  * @param	Num: 显示的数字，范围：0~4294967295
  * @param	Len: 显示位数
  * @param	Size: 字符大小
  * @param	Mode: Mode[4]:0，不足高位不显示，靠右对齐；1，不足高位补0显示
				  Mode[0]:0，非叠加；1，叠加
  * @retval	无
  */
void LCD_ShowBigNum(uint16_t x, uint16_t y, uint32_t Num, uint8_t Len, uint8_t Size, uint8_t Mode)
{
	uint8_t i, Temp, Flag = 0;
	
	if(Mode & 0x10)		//不足高位补0显示
	{
		for(i=1;i<=Len;i++)
		{
			Temp = Num / BN_Pow(10, Len - i) % 10;		//拆位
			LCD_ShowSingleBigNum(x + ((i - 1) * Size / 2), y, Temp, Size, Mode & 0x01);
		}
	}
	else				//不足高位不显示，靠右对齐
	{
		for(i=1;i<=Len;i++)
		{
			Temp = Num / BN_Pow(10, Len - i) % 10;		//拆位
			
			if(Flag == 0 && i < Len)	//未到有效数字（i<Len是因为0要显示最后一位）
			{
				if(Temp == 0)			//Temp不是有效数字
				{
					LCD_ShowBigSpace(x + ((i - 1) * Size / 2), y, Size, Mode & 0x01);
					continue;
				}
				else
				{
					Flag = 1;			//Temp是有效数字，Flag置一
				}
			}
			LCD_ShowSingleBigNum(x + ((i - 1) * Size / 2), y, Temp, Size, Mode & 0x01);
		}
	}
}

/**
  * @brief	LCD显示-占位
  * @param	x: 字符左上角顶点列地址
  * @param	y: 字符左上角顶点页地址
  * @param	Size: 字符大小（64）
  * @param	Mode: 0，非叠加模式；1，叠加模式
  * @retval	无
  */
void LCD_ShowBigEmpty(uint16_t x, uint16_t y, uint8_t Size, uint8_t Mode)
{
	LCD_ShowSingleBigNum(x, y, 10, Size, Mode);
}
