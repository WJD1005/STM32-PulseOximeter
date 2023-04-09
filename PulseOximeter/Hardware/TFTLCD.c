#include "stm32f10x.h"                  // Device header
#include "TFTLCD.h"
#include "Delay.h"
#include "Font.h"

_LCD_Dev LCD_Dev;

uint16_t POINT_COLOR = BLACK;
uint16_t BACK_COLOR = WHITE;

/**
  * @brief	LCD写数据函数
  *			比宏定义函数慢但可以被大量调用，适合在初始化函数中大量调用
  * @param	Data: 16位数据
  * @retval	无
  */
void LCD_WR_DATAX(uint16_t Data)
{
	LCD_RS_SET;
	LCD_CS_CLR;
	DATAOUT(Data);
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
}

/**
  * @brief	LCD写命令函数
  * @param	Command: 命令
  * @retval	无
  */
void LCD_WR_Cmd(uint16_t Command)
{
	LCD_RS_CLR;
	LCD_CS_CLR;
	DATAOUT(Command);
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
}

/**
  * @brief	LCD获取寄存器数据函数
  * @param	无
  * @retval	寄存器上的数据
  */
uint16_t LCD_RD_DATA(void)
{
	uint16_t Data;
	
	//快速修改IO为上拉输入
	GPIOB->CRL=0X88888888; //PB0-7  上拉输入
    GPIOB->CRH=0X88888888; //PB8-15 上拉输入
    GPIOB->ODR=0X0000;     //全部输出0
	
	LCD_RS_SET;
	LCD_CS_CLR;
	LCD_RD_CLR;
	Data = DATAIN;
	LCD_RD_SET;
	LCD_CS_SET;
	
	//快速修改IO为上拉输出
	GPIOB->CRL=0X33333333;  //PB0-7  上拉输出
    GPIOB->CRH=0X33333333;  //PB8-15 上拉输出
    GPIOB->ODR=0XFFFF;      //全部输出1
	
	return Data;
}

/**
  * @brief	LCD写寄存器
  * @param	Reg: 寄存器地址（编号）
  * @param	RegValue: 写入寄存器的数据
  * @retval	无
  */
void LCD_WriteReg(uint16_t Reg, uint16_t RegValue)
{
	LCD_WR_Cmd(Reg);		//发送命令确定寄存器
	LCD_WR_DATA(RegValue);	//写入数据
}

/**
  * @brief	LCD读寄存器
  * @param	Reg: 寄存器地址（编号）
  * @retval	寄存器数据
  */
uint16_t LCD_ReadReg(uint16_t Reg)
{
	LCD_WR_Cmd(Reg);		//发送命令确定寄存器
	return LCD_RD_DATA();	//读取数据
}

/**
  * @brief	LCD开启显示
  * @param	无
  * @retval	无
  */
void LCD_DisplayON(void)
{
	LCD_WR_Cmd(0x29);
}

/**
  * @brief	LCD关闭显示
  * @param	无
  * @retval	无
  */
void LCD_DisplayOFF(void)
{
	LCD_WR_Cmd(0x28);
}

/**
  * @brief	LCD设置扫描方向
  * @param	Dir: 扫描方向
  * @retval	无
  */
void LCD_ScanDir(uint8_t Dir)
{
	uint16_t RegVal = 0, Temp;
	
	//横屏使用时，改变扫描方向
	if(LCD_Dev.ScreenDir == 1)
	{
		switch(Dir)
		{
			case L2R_U2D:
				Dir = D2U_L2R;
				break;
			case L2R_D2U:
				Dir = D2U_R2L;
				break;
			case R2L_U2D:
				Dir = U2D_L2R;
				break;
			case R2L_D2U:
				Dir = U2D_R2L;
				break;
			case U2D_L2R:
				Dir = L2R_D2U;
				break;
			case U2D_R2L:
				Dir = L2R_U2D;
				break;
			case D2U_L2R:
				Dir = R2L_D2U;
				break;
			case D2U_R2L:
				Dir = R2L_U2D;
				break;
		}
	}
	switch (Dir)
    {
        case L2R_U2D:
            RegVal |= (0 << 7) | (0 << 6) | (0 << 5);
            break;

        case L2R_D2U:
            RegVal |= (1 << 7) | (0 << 6) | (0 << 5);
            break;

        case R2L_U2D:
            RegVal |= (0 << 7) | (1 << 6) | (0 << 5);
            break;

        case R2L_D2U:
            RegVal |= (1 << 7) | (1 << 6) | (0 << 5);
            break;

        case U2D_L2R:
            RegVal |= (0 << 7) | (0 << 6) | (1 << 5);
            break;

        case U2D_R2L:
            RegVal |= (0 << 7) | (1 << 6) | (1 << 5);
            break;

        case D2U_L2R:
            RegVal |= (1 << 7) | (0 << 6) | (1 << 5);
            break;

        case D2U_R2L:
            RegVal |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
    }
	RegVal |= 0x08;		//设置BGR位
	
	LCD_WriteReg(LCD_ScanDirCmd, RegVal);
	
	if (RegVal & 0X20)
	{
		if (LCD_Dev.Width < LCD_Dev.Height)   //交换宽高表达
		{
			Temp = LCD_Dev.Width;
			LCD_Dev.Width = LCD_Dev.Height;
			LCD_Dev.Height = Temp;
		}
	}
	else
	{
		if (LCD_Dev.Width > LCD_Dev.Height)   //交换宽高表达
		{
			Temp = LCD_Dev.Width;
			LCD_Dev.Width = LCD_Dev.Height;
			LCD_Dev.Height = Temp;
		}
	}
	
	//设置基本显示区域（全屏）
	LCD_WR_Cmd(LCD_SetxCmd);
	LCD_WR_DATA(0);		//起始列地址高八位
	LCD_WR_DATA(0);		//起始列地址低八位
	LCD_WR_DATA((LCD_Dev.Width - 1) >> 8);		//末尾列地址高八位
	LCD_WR_DATA((LCD_Dev.Width - 1) & 0xFF);	//末尾列地址低八位
	LCD_WR_Cmd(LCD_SetyCmd);
	LCD_WR_DATA(0);		//起始页地址高八位
	LCD_WR_DATA(0);		//起始页地址低八位
	LCD_WR_DATA((LCD_Dev.Height - 1) >> 8);		//末尾页地址高八位
	LCD_WR_DATA((LCD_Dev.Height - 1) & 0xFF);	//末尾页地址低八位
}

/**
  * @brief	LCD屏幕方向
  * @param	Dir: 0，竖屏；1，横屏
  * @retval	无
  */
void LCD_DisplayDir(uint8_t Dir)
{
	LCD_Dev.ScreenDir = Dir;
	
	if(Dir == 0)	//竖屏
	{
		LCD_Dev.Width = 240;
		LCD_Dev.Height = 320;
	}
	else			//横屏
	{
		LCD_Dev.Width = 320;
		LCD_Dev.Height = 240;
	}
	LCD_ScanDir(L2R_U2D);	//默认扫描方向
}

/**
  * @brief	LCD设置窗口（显示区域），在LCD_ScanDir()中已设置为全屏显示
  * @param	sx: 窗口左上角列地址
  * @param	sy: 窗口左上角页地址
  * @param	Width: 窗口宽度
  * @param	Height: 窗口高度
  * @retval	无
  */
void LCD_SetWindow(uint16_t sx, uint16_t sy, uint16_t Width, uint16_t Height)
{
	LCD_WR_Cmd(LCD_SetxCmd);
	LCD_WR_DATA(sx >> 8);		//起始列地址高八位
	LCD_WR_DATA(sx & 0xFF);		//起始列地址低八位
	LCD_WR_DATA((sx + Width - 1) >> 8);		//末尾列地址高八位
	LCD_WR_DATA((sx + Width - 1) & 0xFF);	//末尾列地址低八位
	LCD_WR_Cmd(LCD_SetyCmd);
	LCD_WR_DATA(sy >> 8);		//起始页地址高八位
	LCD_WR_DATA(sy & 0xFF);		//起始页地址低八位
	LCD_WR_DATA((sy + Height - 1) >> 8);		//末尾页地址高八位
	LCD_WR_DATA((sy + Height - 1) & 0xFF);		//末尾页地址低八位
}

/**
  * @brief	LCD设置光标位置
  * @param	x: 列地址
  * @param	y: 页地址
  * @retval	无
  */
void LCD_SetCursor(uint16_t x, uint16_t y)
{
	LCD_WR_Cmd(LCD_SetxCmd);	//写列地址指令
	LCD_WR_DATA(x >> 8);			//写高八位
	LCD_WR_DATA(x & 0xFF)			//写低八位
	LCD_WR_Cmd(LCD_SetyCmd);	//写页地址指令
	LCD_WR_DATA(y >> 8);			//写高八位
	LCD_WR_DATA(y & 0xFF);			//写低八位
}

/**
  * @brief	LCD写显存准备
  * @param	无
  * @retval	无
  */
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_Cmd(LCD_WRAMCmd);	//发送写显存命令
}

/**
  * @brief	LCD画点函数，需要先设置POINT_COLOR
  * @param	x: 列地址
  * @param	y: 页地址
  * @retval	无
  */
void LCD_DrawPoint(uint16_t x, uint16_t y)
{
	LCD_SetCursor(x, y);		//设置光标位置
	LCD_WriteRAM_Prepare();		//开始写显存
	LCD_WR_DATA(POINT_COLOR);	//写显存
}

/**
  * @brief	LCD快速画点函数
  * @param	x: 列地址
  * @param	y: 页地址
  * @param	Color: 该点颜色16位RGB
  * @retval	无
  */
void LCD_Fast_DrawPoint(uint16_t x, uint16_t y, uint16_t Color)
{
	//设置光标位置
	LCD_RS_CLR;
	LCD_CS_CLR;
	DATAOUT(LCD_SetxCmd);			//写列地址
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
	LCD_WR_DATA(x >> 8);			//写高八位
	LCD_WR_DATA(x & 0xFF)			//写低八位
	LCD_RS_CLR;
	LCD_CS_CLR;
	DATAOUT(LCD_SetyCmd);			//写页地址
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
	LCD_WR_DATA(y >> 8);			//写高八位
	LCD_WR_DATA(y & 0xFF);			//写低八位
	//开始写显存
	LCD_RS_CLR;
	LCD_CS_CLR;
	DATAOUT(LCD_WRAMCmd);
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
	//写显存
	LCD_WR_DATA(Color);
}

/**
  * @brief	LCD读取一个点RGB值
  * @param	x: 列地址
  * @param	y: 页地址
  * @retval	该点16位RGB值
  */
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y)
{
	uint16_t R, G, B, Temp;
	
	if(x >= LCD_Dev.Width || y >= LCD_Dev.Height)
	{
		return 0;	//输入坐标超出范围直接返回0
	}
	
	LCD_SetCursor(x, y);			//设置光标位置
	LCD_WR_Cmd(LCD_RRAMCmd);	//发送读显存命令
	LCD_RD_DATA();					//假读
	Temp = LCD_RD_DATA();			//第一个参数，含有R1，G1
	R = Temp >> 11;
	G = (Temp & 0xFF) >> 2;
	Temp = LCD_RD_DATA();			//第二个参数，含有B1，R2
	B = Temp >> 11;
	
	return (R << 11) | (G << 5) | B;
}

/**
  * @brief	LCD全屏清屏（全屏填充色）
  * @param	Color: 填充颜色16位RGB
  * @retval	无
  */
void LCD_Clear(uint16_t Color)
{
	uint32_t i = 0, TotalPoint;
    TotalPoint = LCD_Dev.Width;
    TotalPoint *= LCD_Dev.Height;    //计算总像素数

    //设置全屏
	LCD_SetWindow(0x0000, 0x0000, LCD_Dev.Width, LCD_Dev.Height);
	
    LCD_WriteRAM_Prepare();         //开始写显存

    for (i=0;i<TotalPoint;i++)
    {
        LCD_WR_DATA(Color);
    }
}

/**
  * @brief	LCD指定区域填色
  * @param	sx: 区域左上角列地址
  * @param	sy: 区域左上角页地址
  * @param	Width: 区域宽度
  * @param	Height: 区域高度
  * @param	Color: 填充颜色16位RGB
  * @retval	无
  */
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t Width, uint16_t Height, uint16_t Color)
{
	uint32_t i = 0, TotalPoint;
    TotalPoint = Width;
    TotalPoint *= Height;    //计算总像素数

    //设置填色区域
	LCD_SetWindow(sx, sy, Width, Height);
	
    LCD_WriteRAM_Prepare();         //开始写显存

    for (i=0;i<TotalPoint;i++)
    {
        LCD_WR_DATA(Color);
    }
}

/**
  * @brief	LCD初始化
  * @param	Dir: 0，竖屏；1，横屏
  * @retval	无
  */
void LCD_Init(uint8_t Dir)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE); //开启GPIOB、GPIOC、AFIO时钟
    
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);       //禁止JTAG

	GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8|GPIO_Pin_7|GPIO_Pin_6;  ///PC6~10复用推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_SetBits(GPIOC,GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8|GPIO_Pin_7|GPIO_Pin_6);	//PC6~10全置1

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All; //PB0~15复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB,GPIO_Pin_All);	//PB0~15全置1
	
	Delay_ms(50);
	LCD_WriteReg(0x0000, 0x0001);
	Delay_ms(50);
	
	//读ID
	LCD_WR_Cmd(LCD_GetIDCmd);
	LCD_RD_DATA();						//假读
	LCD_RD_DATA();						//0x00
	LCD_Dev.ID = LCD_RD_DATA() << 8;	//0x93
	LCD_Dev.ID |= LCD_RD_DATA();		//0x41
	
	if (LCD_Dev.ID == 0X9341)    //本代码基于9341驱动IC
    {
        //初始化序列
		LCD_WR_Cmd(0xCF);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0xC1);
        LCD_WR_DATAX(0X30);
        LCD_WR_Cmd(0xED);
        LCD_WR_DATAX(0x64);
        LCD_WR_DATAX(0x03);
        LCD_WR_DATAX(0X12);
        LCD_WR_DATAX(0X81);
        LCD_WR_Cmd(0xE8);
        LCD_WR_DATAX(0x85);
        LCD_WR_DATAX(0x10);
        LCD_WR_DATAX(0x7A);
        LCD_WR_Cmd(0xCB);
        LCD_WR_DATAX(0x39);
        LCD_WR_DATAX(0x2C);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x34);
        LCD_WR_DATAX(0x02);
        LCD_WR_Cmd(0xF7);
        LCD_WR_DATAX(0x20);
        LCD_WR_Cmd(0xEA);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x00);
        LCD_WR_Cmd(0xC0);       //Power control
        LCD_WR_DATAX(0x1B);     //VRH[5:0]
        LCD_WR_Cmd(0xC1);       //Power control
        LCD_WR_DATAX(0x01);     //SAP[2:0];BT[3:0]
        LCD_WR_Cmd(0xC5);       //VCM control
        LCD_WR_DATAX(0x30);     //3F
        LCD_WR_DATAX(0x30);     //3C
        LCD_WR_Cmd(0xC7);       //VCM control2
        LCD_WR_DATAX(0XB7);
        LCD_WR_Cmd(0x36);       // Memory Access Control
        LCD_WR_DATAX(0x48);
        LCD_WR_Cmd(0x3A);
        LCD_WR_DATAX(0x55);
        LCD_WR_Cmd(0xB1);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x1A);
        LCD_WR_Cmd(0xB6);       // Display Function Control
        LCD_WR_DATAX(0x0A);
        LCD_WR_DATAX(0xA2);
        LCD_WR_Cmd(0xF2);       // 3Gamma Function Disable
        LCD_WR_DATAX(0x00);
        LCD_WR_Cmd(0x26);       //Gamma curve selected
        LCD_WR_DATAX(0x01);
        LCD_WR_Cmd(0xE0);       //Set Gamma
        LCD_WR_DATAX(0x0F);
        LCD_WR_DATAX(0x2A);
        LCD_WR_DATAX(0x28);
        LCD_WR_DATAX(0x08);
        LCD_WR_DATAX(0x0E);
        LCD_WR_DATAX(0x08);
        LCD_WR_DATAX(0x54);
        LCD_WR_DATAX(0XA9);
        LCD_WR_DATAX(0x43);
        LCD_WR_DATAX(0x0A);
        LCD_WR_DATAX(0x0F);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x00);
        LCD_WR_Cmd(0XE1);       //Set Gamma
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x15);
        LCD_WR_DATAX(0x17);
        LCD_WR_DATAX(0x07);
        LCD_WR_DATAX(0x11);
        LCD_WR_DATAX(0x06);
        LCD_WR_DATAX(0x2B);
        LCD_WR_DATAX(0x56);
        LCD_WR_DATAX(0x3C);
        LCD_WR_DATAX(0x05);
        LCD_WR_DATAX(0x10);
        LCD_WR_DATAX(0x0F);
        LCD_WR_DATAX(0x3F);
        LCD_WR_DATAX(0x3F);
        LCD_WR_DATAX(0x0F);
        LCD_WR_Cmd(0x2B);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x01);
        LCD_WR_DATAX(0x3f);
        LCD_WR_Cmd(0x2A);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0xef);
        LCD_WR_Cmd(0x11);       //Exit Sleep
        Delay_ms(120);
        LCD_WR_Cmd(0x29);       //display on
		
		LCD_DisplayDir(Dir);	//设置屏幕方向
		LCD_BL_ON;				//开启背光
		LCD_Clear(BLACK);
    }
}

/**
  * @brief	LCD画线函数
  * @param	x1: 起点列地址
  * @param	y1: 起点页地址
  * @param	x2: 终点列地址
  * @param	y2: 终点页地址
  * @retval	无
  */
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t x = x1, y = y1, i;
	int16_t Delta_x = x2 - x1, Delta_y = y2 - y1, d;
	int8_t incx, incy;
	
	//确定单步方向
	if(Delta_x > 0)
	{
		incx = 1;
	}
	else if(Delta_x == 0)
	{
		incx = 0;
	}
	else
	{
		incx = -1;
		Delta_x = -Delta_x;
	}
	if(Delta_y > 0)
	{
		incy = 1;
	}
	else if(Delta_y == 0)
	{
		incy = 0;
	}
	else
	{
		incy = -1;
		Delta_y = -Delta_y;
	}
	
	//主位移轴
	if(Delta_x > Delta_y)
	{
		d = Delta_y;
		//画线
		for(i=0;i<=Delta_x;i++)
		{
			LCD_DrawPoint(x, y);
			x += incx;
			d += Delta_y;
			if(d > Delta_x)
			{
				d -= Delta_x;
				y += incy;
			}
		}
	}
	else
	{
		d = Delta_x;
		//画线
		for(i=0;i<=Delta_y;i++)
		{
			LCD_DrawPoint(x, y);
			y += incy;
			d += Delta_x;
			if(d > Delta_y)
			{
				d -= Delta_y;
				x += incx;
			}
		}
	}
}

/**
  * @brief	LCD画矩形函数
  * @param	x1: 顶点列地址
  * @param	y1: 顶点页地址
  * @param	x2: 对角顶点列地址
  * @param	y2: 对角顶点页地址
  * @retval	无
  */
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_DrawLine(x1, y1, x2, y1);
	LCD_DrawLine(x1, y1, x1, y2);
	LCD_DrawLine(x2, y2, x1, y2);
	LCD_DrawLine(x2, y2, x2, y1);
}

/**
  * @brief	LCD画矩形函数
  * @param	x: 圆心列地址
  * @param	y: 圆心页地址
  * @param	R: 半径
  * @retval	无
  */
void LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t R)
{
	uint16_t a, b;
	int16_t d = 1 - R;
	
	//画上下左右四点
	LCD_DrawPoint(x, y + R);
	LCD_DrawPoint(x, y - R);
	LCD_DrawPoint(x - R, y);
	LCD_DrawPoint(x + R, y);
	
	for(a=1,b=R;a<=b;a++)
	{
		if(d < 0)
		{
			d += (2 * a + 1);
		}
		else
		{
			b--;
			d += (2 * (a - b) + 1);
		}
		//画对称八点
		LCD_DrawPoint(x + a, y + b);
		LCD_DrawPoint(x + b, y + a);
		LCD_DrawPoint(x + b, y - a);
		LCD_DrawPoint(x + a, y - b);
		LCD_DrawPoint(x - a, y - b);
		LCD_DrawPoint(x - b, y - a);
		LCD_DrawPoint(x - b, y + a);
		LCD_DrawPoint(x - a, y + b);
	}
}

/**
  * @brief	LCD显示一个字符函数
  * @param	x: 字符左上角顶点列地址
  * @param	y: 字符左上角顶点页地址
  * @param	Char: ASCII字符
  * @param	Size: 字符大小（12/16/24）
  * @param	Mode: 0，非叠加模式；1，叠加模式
  * @retval	无
  */
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t Char, uint8_t Size, uint8_t Mode)
{
	uint8_t Temp, i, j, cSize = (Size / 8 + ((Size % 8) ? 1 : 0)) * (Size / 2);
	uint16_t y0 = y;
	
	for(i=0;i<cSize;i++)		//取字节
	{
		if(Size == 12)			//1206字库
		{
			Temp = ASCII_1206[Char - ' '][i];
		}
		else if(Size == 16)		//1608字库
		{
			Temp = ASCII_1608[Char - ' '][i];
		}
		else if(Size == 24)		//2412字库
		{
			Temp = ASCII_2412[Char - ' '][i];
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
  * @brief	LCD乘方函数
  * @param	n: 底数
  * @param	m: 指数
  * @retval	n^m
  */
uint32_t LCD_Pow(uint8_t n, uint8_t m)
{
	uint32_t Result = 1;
	
	for(;m>0;m--)
	{
		Result *= n;
	}
	return Result;
}

/**
  * @brief	LCD显示非负整数
  * @param	x: 显示区域左上角顶点列坐标
  * @param	y: 显示区域左上角顶点页坐标
  * @param	Num: 显示的数字，范围：0~4294967295
  * @param	Len: 显示位数
  * @param	Size: 字符大小
  * @param	Mode: Mode[4]:0，不足高位不显示，靠右对齐；1，不足高位补0显示
				  Mode[0]:0，非叠加；1，叠加
  * @retval	无
  */
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t Num, uint8_t Len, uint8_t Size, uint8_t Mode)
{
	uint8_t i, Temp, Flag = 0;
	
	if(Mode & 0x10)		//不足高位补0显示
	{
		for(i=1;i<=Len;i++)
		{
			Temp = Num / LCD_Pow(10, Len - i) % 10;		//拆位
			LCD_ShowChar(x + ((i - 1) * Size / 2), y, Temp + '0', Size, Mode & 0x01);
		}
	}
	else				//不足高位不显示，靠右对齐
	{
		for(i=1;i<=Len;i++)
		{
			Temp = Num / LCD_Pow(10, Len - i) % 10;		//拆位
			
			if(Flag == 0 && i < Len)	//未到有效数字（i<Len是因为0要显示最后一位）
			{
				if(Temp == 0)			//Temp不是有效数字
				{
					LCD_ShowChar(x + ((i - 1) * Size / 2), y, ' ', Size, Mode & 0x01);
					continue;
				}
				else
				{
					Flag = 1;			//Temp是有效数字，Flag置一
				}
			}
			LCD_ShowChar(x + ((i - 1) * Size / 2), y, Temp + '0', Size, Mode & 0x01);
		}
	}
}

/**
  * @brief	LCD显示带符号整数
  * @param	x: 显示区域左上角顶点列坐标
  * @param	y: 显示区域左上角顶点页坐标
  * @param	Num: 显示的数字，范围：-2147483648~2147483647
  * @param	Len: 显示位数
  * @param	Size: 字符大小
  * @param	Mode: Mode[4]:0，不足高位不显示，靠右对齐；1，不足高位补0显示
				  Mode[0]:0，非叠加；1，叠加
  * @retval	无
  */
void LCD_ShowSignedNum(uint16_t x, uint16_t y, int32_t Num, uint8_t Len, uint8_t Size, uint8_t Mode)
{
	if(Num < 0)
	{
		LCD_ShowChar(x, y, '-', Size, Mode & 0x01);
		Num = -Num;
	}
	else
	{
		LCD_ShowChar(x, y, '+', Size, Mode & 0x01);
	}
	LCD_ShowNum(x + (Size / 2), y, Num, Len, Size, Mode);
}

/**
  * @brief	LCD显示字符串，长度不超过256字节
  * @param	x: 显示区域左上角顶点列坐标
  * @param	y: 显示区域左上角顶点页坐标
  * @param	Width: 显示区域宽度
  * @param	Height: 显示区域高度
  * @param	*String: 字符串首地址
  * @param	Size: 字符大小
  * @param	Mode: 0，非叠加；1，叠加
  * @retval	无
  */
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, char *String, uint8_t Size, uint8_t Mode)
{
	uint8_t i;
	uint16_t x0 = x, y0 = y;
	
	for(i=0;String[i]!='\0';i++)				//遍历字符
	{
		if(String[i] == '\n')					//手动换行
		{
			x = x0;
			y += Size;
			continue;
		}
		if((y + Size) <= y0 + Height)			//y预判断写不下就退出
		{
			if((x + (Size / 2)) <= x0 + Width)	//x预判断写不下就换行
			{
				LCD_ShowChar(x, y, String[i], Size, Mode);
				x += (Size / 2);
			}
			else
			{
				x = x0;
				y += Size;
				i--;			//阻止i自增
			}
		}
		else
		{
			break;
		}
	}
}
