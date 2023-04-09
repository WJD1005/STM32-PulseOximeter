#ifndef __TFTLCD_H
#define __TFTLCD_H

//LCD参数集结构体
typedef struct
{
	uint16_t Width;			//LCD宽度
	uint16_t Height;		//LCD高度
	uint16_t ID;			//控制器ID
	uint8_t ScreenDir;		//屏幕方向：0，竖屏；1，横屏
}_LCD_Dev;

//LCD参数管理
extern _LCD_Dev LCD_Dev;

//LCD画笔颜色和背景色
extern uint16_t POINT_COLOR;
extern uint16_t BACK_COLOR;

//////////////////////////////////////////////////////////////////////
//LCD端口定义
//使用快速IO控制寄存器来提高速度
#define LCD_BL_ON	GPIOC->BSRR=1<<10	//开启LCD背光	PC10
#define LCD_BL_OFF	GPIOC->BRR=1<<10	//关闭LCD背光	PC10

#define LCD_CS_SET  GPIOC->BSRR=1<<9	//片选设置		PC9
#define LCD_RS_SET  GPIOC->BSRR=1<<8    //数据     		PC8
#define LCD_WR_SET  GPIOC->BSRR=1<<7    //写入设置		PC7
#define LCD_RD_SET  GPIOC->BSRR=1<<6    //读取设置		PC6

#define LCD_CS_CLR  GPIOC->BRR=1<<9     //CS清零		PC9
#define LCD_RS_CLR  GPIOC->BRR=1<<8     //RS清零		PC8
#define LCD_WR_CLR  GPIOC->BRR=1<<7     //WR清零		PC7
#define LCD_RD_CLR  GPIOC->BRR=1<<6     //RD清零		PC6

//PB0~15数据线 宏定义函数
#define DATAOUT(x)  GPIOB->ODR=x;	//数据输出
#define DATAIN   	GPIOB->IDR;		//数据输入

//////////////////////////////////////////////////////////////////////
//9341驱动IC命令
#define LCD_GetIDCmd	0xD3
#define LCD_ScanDirCmd	0x36
#define LCD_SetxCmd		0x2A
#define LCD_SetyCmd		0x2B
#define LCD_WRAMCmd		0x2C
#define LCD_RRAMCmd		0x2E

//////////////////////////////////////////////////////////////////////
//扫描方向定义（以竖屏为参照）
#define L2R_U2D  0		//从左到右，从上到下
#define L2R_D2U  1		//从左到右，从下到上
#define R2L_U2D  2		//从右到左，从上到下
#define R2L_D2U  3		//从右到左，从下到上

#define U2D_L2R  4		//从上到下，从左到右
#define U2D_R2L  5		//从上到下，从右到左
#define D2U_L2R  6		//从下到上，从左到右
#define D2U_R2L  7		//从下到上，从右到左

//////////////////////////////////////////////////////////////////////
//画笔颜色定义
#define WHITE           0xFFFF
#define BLACK           0x0000
#define BLUE            0x001F
#define BRED            0XF81F
#define GRED            0XFFE0
#define GBLUE           0X07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define GREEN           0x07E0
#define CYAN            0x7FFF
#define YELLOW          0xFFE0
#define BROWN           0XBC40
#define BRRED           0XFC07
#define GRAY            0X8430

#define DARKBLUE        0X01CF
#define LIGHTBLUE       0X7D7C
#define GRAYBLUE        0X5458

#define LIGHTGREEN      0X841F
#define LIGHTGRAY       0XC618

#define LGRAYBLUE       0XA651
#define LBBLUE          0X2B12

////////////////////////////////////////////////////////////////////
/**
  * @brief	LCD写数据宏定义函数
  *			比函数LCD_WR_DATAX()速度快但FLASH占用大，适用于高速写入数据
  * @param	Data: 16位数据
  * @retval	无
  */
#define LCD_WR_DATA(Data)\
		{\
			LCD_RS_SET;\
			LCD_CS_CLR;\
			DATAOUT(Data);\
			LCD_WR_CLR;\
			LCD_WR_SET;\
			LCD_CS_SET;\
		}

void LCD_Init(uint8_t Dir);
void LCD_DisplayON(void);
void LCD_DisplayOFF(void);
void LCD_ScanDir(uint8_t Dir);
void LCD_DisplayDir(uint8_t Dir);
void LCD_SetWindow(uint16_t sx, uint16_t sy, uint16_t Width, uint16_t Height);
void LCD_SetCursor(uint16_t x, uint16_t y);
void LCD_DrawPoint(uint16_t x, uint16_t y);
void LCD_Fast_DrawPoint(uint16_t x, uint16_t y, uint16_t Color);
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y);
void LCD_Clear(uint16_t Color);
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t Width, uint16_t Height, uint16_t Color);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t R);
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t Char, uint8_t Size, uint8_t Mode);
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t Num, uint8_t Len, uint8_t Size, uint8_t Mode);
void LCD_ShowSignedNum(uint16_t x, uint16_t y, int32_t Num, uint8_t Len, uint8_t Size, uint8_t Mode);
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, char *String, uint8_t Size, uint8_t Mode);

#endif
