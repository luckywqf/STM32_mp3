/****************************************************************************
*
* 文件名: tft_lcd.c
* 内容简述: 本模块包含TFT LCD显示器的驱动函数。
*
*	冰凌科技开发板标配的TFT显示器的驱动芯片为 SPFD5420A，分辨率为400x240
*	驱动芯片的访问地址为
*		(1) 0x60000000
*/

#include "stm32f10x.h"
#include "delay.h"
#include "tft_lcd.h"


static __IO uint16_t s_TextColor = 0x0000;
static __IO uint16_t s_BackColor = 0xFFFF;

/* 添加透明标志
	s_Transparent = 1 表示显示文字时，不修改背景颜色
*/
static __IO uint8_t s_Transparent = 0;

/*******************************************************************************
*	函数名: LCD_Init
*	参  数: 无
*	返  回: 无
*	功  能: 初始化TFT显示器
*/
void LCD_Init(void)
{
	/* 配置LCD控制口线GPIO */
	LCD_CtrlLinesConfig();

	/* 配置FSMC接口，数据总线 */
	LCD_FSMCConfig();

	/* FSMC重置后必须加延迟才能访问总线设备  */
	DelayMS(20);

	/* 初始化LCD，写LCD寄存器进行配置 */
	LCD_WriteReg(0x0000, 0x0000);	 //ID Read Register
	LCD_WriteReg(0x0001, 0x0100);	 //Driver Output Control Register
	LCD_WriteReg(0x0002, 0x0100);	 //LCD Driving Waveform Control
	LCD_WriteReg(0x0003, 0x1018);	 //Entry Mode	  0x1018 1030

	LCD_WriteReg(0x0007, 0x0001);	 //Display Control 1
	LCD_WriteReg(0x0008, 0x0808);	 //Display Control 2
	LCD_WriteReg(0x0009, 0x0001);	 //Display Control 3

	LCD_WriteReg(0x000B, 0x0010);	 //Low Power Control
	LCD_WriteReg(0x000C, 0x0000);	 //External Display Interface Control 1
	LCD_WriteReg(0x000F, 0x0000);	 //External Display Interface Control 2
	LCD_WriteReg(0x0010, 0x0013);	 //Panel Interface Control 1
	LCD_WriteReg(0x0011, 0x0501);	 //Panel Interface Control 2
	LCD_WriteReg(0x0012, 0x0300);	 //Panel Interface control 3
	LCD_WriteReg(0x0020, 0x021E);	 //Panel Interface control 4
	LCD_WriteReg(0x0021, 0x0202);	 //Panel Interface Control 5
	LCD_WriteReg(0x0090, 0x8000);	 //Frame Marker Control
	LCD_WriteReg(0x0100, 0x17B0);	 //Power Control 1
	LCD_WriteReg(0x0101, 0x0147);	 //Power Control 2
	LCD_WriteReg(0x0102, 0x0135);	 //Power Control 3
	LCD_WriteReg(0x0103, 0x0700);	 //Power Control 4
	LCD_WriteReg(0x0107, 0x0000);	 //Power Control 5
	LCD_WriteReg(0x0110, 0x0001);	 //Power Control 6
	LCD_WriteReg(0x0210, 0x0000);
	LCD_WriteReg(0x0211, 0x00EF);
	LCD_WriteReg(0x0212, 0x0000);
	LCD_WriteReg(0x0213, 0x018F);
	LCD_WriteReg(0x0280, 0x0000);	 //NVM read data 1
	LCD_WriteReg(0x0281, 0x0004);	 //NVM read data 2
	LCD_WriteReg(0x0282, 0x0000);	 //NVM read data 3
	LCD_WriteReg(0x0300, 0x0101);
	LCD_WriteReg(0x0301, 0x0B2C);
	LCD_WriteReg(0x0302, 0x1030);
	LCD_WriteReg(0x0303, 0x3010);
	LCD_WriteReg(0x0304, 0x2C0B);
	LCD_WriteReg(0x0305, 0x0101);
	LCD_WriteReg(0x0306, 0x0807);
	LCD_WriteReg(0x0307, 0x0708);
	LCD_WriteReg(0x0308, 0x0107);
	LCD_WriteReg(0x0309, 0x0105);
	LCD_WriteReg(0x030A, 0x0F04);
	LCD_WriteReg(0x030B, 0x0F00);
	LCD_WriteReg(0x030C, 0x000F);
	LCD_WriteReg(0x030D, 0x040F);
	LCD_WriteReg(0x030E, 0x0300);
	LCD_WriteReg(0x030F, 0x0701);
	LCD_WriteReg(0x0400, 0x3500);	 //Base Image Number of Line
	LCD_WriteReg(0x0401, 0x0001);	 //Base Image Display Control
	LCD_WriteReg(0x0404, 0x0000);	 //Based Image Vertical Scroll Control
	LCD_WriteReg(0x0500, 0x0000);	 //Display Position - Partial Display 1
	LCD_WriteReg(0x0501, 0x0000);	 //RAM Address Start – Partial Display	1
	LCD_WriteReg(0x0502, 0x0000);	 //RAM Address End – Partial Display 1
	LCD_WriteReg(0x0503, 0x0000);	 //Display Position – Partial Display 2
	LCD_WriteReg(0x0504, 0x0000);	 //RAM Address Start – Partial Display 2
	LCD_WriteReg(0x0505, 0x0000);	 //RAM Address End – Partial Display 2
	LCD_WriteReg(0x0600, 0x0000);
	LCD_WriteReg(0x0606, 0x0000);	 //Pin Control
	LCD_WriteReg(0x06F0, 0x0000);	 //NVM Access Control
	LCD_WriteReg(0x07F0, 0x5420);	 //
	LCD_WriteReg(0x07DE, 0x0000);	 //
	LCD_WriteReg(0x07F2, 0x00DF);
	LCD_WriteReg(0x07F3, 0x0810);
	LCD_WriteReg(0x07F4, 0x0077);
	LCD_WriteReg(0x07F5, 0x0021);
	LCD_WriteReg(0x07F0, 0x0000);
	LCD_WriteReg(0x0007, 0x0173);

	/* 设置显示窗口 WINDOWS */
	LCD_WriteReg(0x0210, 0);	/* 水平起始地址 */
	LCD_WriteReg(0x0211, 239);	/* 水平结束坐标 */
	LCD_WriteReg(0x0212, 0);	/* 垂直起始地址 */
	LCD_WriteReg(0x0213, 399);	/* 垂直结束地址 */
}

/*******************************************************************************
*	函数名: LCD_SetTextColor
*	参  数: Color : 文本颜色
*	返  回: 无
*	功  能: 设置文本颜色，保存在全部变量s_TextColor
*/
void LCD_SetTextColor(__IO uint16_t Color)
{
	s_TextColor = Color;
}

/*******************************************************************************
*	函数名: LCD_SetBackColor
*	参  数: Color : 背景颜色
*	返  回: 无
*	功  能: 设置背景颜色，保存在全部变量 s_BackColor
*/
void LCD_SetBackColor(__IO uint16_t Color)
{
	s_BackColor = Color;
}

/*******************************************************************************
*	函数名: LCD_ClearLine
*	参  数: Line : 文本行号0 - 9
*	返  回: 无
*	功  能: 清除选定的文本行
*/
void LCD_ClearLine(uint8_t Line)
{
	LCD_DisplayString(0, Line, "                          ");
}

/*******************************************************************************
*	函数名: LCD_Clear
*	参  数: Color : 背景色
*	返  回: 无
*	功  能: 根据输入的颜色值清屏
*/
void LCD_Clear(uint16_t Color)
{
	uint32_t index = 0;

	LCD_SetCursor(0, 0);	/* 设置光标位置 */

	LCD_WriteRAM_Prepare(); 	/* 准备写显存 */

	for (index = 0; index < 400 * 240; index++)
	{
		LCD->LCD_RAM = Color;
	}
}

/*******************************************************************************
*	函数名: LCD_SetCursor
*	参  数: Xpos : X坐标; Ypos: Y坐标
*	返  回: 无
*	功  能: 设置光标位置
*/
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	/*
		px，py 是物理坐标， x，y是虚拟坐标
		转换公式:
		py = 399 - x;
		px = y;
	*/
	LCD_WriteReg(0x0200, Ypos);  		/* px */
	LCD_WriteReg(0x0201, 399 - Xpos);	/* py */
}

/*******************************************************************************
*	函数名: LCD_SetTransparent
*	参  数: _mode ; 透明标志，1表示透明 0 表示不透明
*	返  回: 无
*	功  能: 设置光标位置
*/
void LCD_SetTransparent(uint8_t _mode)
{
	if (_mode == 0)
	{
		s_Transparent = 0;	/* 设置为背景不透明 */
	}
	else
	{
		s_Transparent = 1;	/* 设置为背景透明 */
	}
}

/*******************************************************************************
*	函数名: LCD_DrawChar
*	参  数:
*		Xpos : X坐标;
*		Ypos: Y坐标；
*		c : 指向字符点阵的指针
*		width : 点阵的宽度，字符是8，汉字是16
*	返  回: 无
*	功  能: 在LCD上显示一个字符(16x24)
*/
void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint8_t *c, uint8_t width)
{
	uint32_t index = 0, i = 0;
	uint8_t Yaddress;

	Yaddress = Ypos;

	LCD_SetCursor(Xpos, Ypos);

	if (s_Transparent == 0)
	{
		for (index = 0; index < 16; index++)	/* 字符高度 */
		{
			LCD_WriteRAM_Prepare();
			for (i = 0; i < width; i++)	/* 字符宽度 */
			{
				if ((c[index * (width / 8) + i / 8] & (0x80 >> (i % 8))) == 0x00)
				{
					LCD_WriteRAM(s_BackColor);
				}
				else
				{
					LCD_WriteRAM(s_TextColor);
				}
			}
			Yaddress++;
			LCD_SetCursor(Xpos, Yaddress);
		}
	}
	else	/* 实现文字叠加在图片上的功能 */
	{
		for (index = 0; index < 16; index++)	/* 字符高度 */
		{
			uint16_t x = Xpos;

			for (i = 0; i < width; i++)	/* 字符宽度 */
			{
				if ((c[index * (width / 8) + i / 8] & (0x80 >> (i % 8))) != 0x00)
				{
					LCD_SetCursor(x, Yaddress);

					LCD->LCD_REG = 0x202;
					LCD->LCD_RAM = s_TextColor;
				}
				x++;
			}
			Yaddress++;
		}
	}
}

/*******************************************************************************
*	函数名: LCD_DisplayString
*	参  数:
*		Xpos : X坐标 0 - 399
*		Ypos : Y坐标 0 - 239
*		ptr  : 字符串指针；
*	返  回: 无
*	功  能: 在LCD指定文本行显示一个字符串
*/
void LCD_DisplayString(uint16_t Xpos, uint16_t Ypos, uint8_t *ptr)
{
	uint32_t i = 0;
	uint8_t code1;
	uint8_t code2;
	uint32_t address;


	while ((*ptr != 0) & (i < 50))
	{
		code1 = *ptr;	/* ascii代码 或者汉字代码的高字节 */
		if (code1 < 0x80)
		{
			LCD_DrawChar(Xpos, Ypos, &Ascii16[code1 * 16], 8);	/* 16 表示1个字符字模的字节数 */
			Xpos += 8;		/* 列地址+8 */
		}
		else	/* 汉字内码 */
		{
			code2 = *++ptr;
			if (code2 == 0)
			{
				break;
			}
			/* 计算16点阵汉字点阵地址
				ADDRESS = [(code1-0xa1) * 94 + (code2-0xa1)] * 32
				;
			*/
			address = ((code1-0xa1) * 94 + (code2-0xa1)) * 32 + HZK16_ADDR;
			LCD_DrawChar(Xpos, Ypos, (const uint8_t *)address, 16);
			Xpos += 16;		/* 列地址+16 */
		}
		ptr++;			/* 指向下一个字符 */
		i++;
	}
}

/*******************************************************************************
*	函数名: LCD_SetDisplayWindow
*	参  数:
*		Xpos : 显示行号
*		Ypos  : 字符串指针；
*		Height: 窗口高度
*		Width : 窗口宽度
*	返  回: 无
*	功  能: 设置显示窗口
*/
void LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{
	/* 水平起始地址 */
	if (Xpos >= Height)
	{
		LCD_WriteReg(R80, (Xpos - Height + 1));
	}
	else
	{
		LCD_WriteReg(R80, 0);
	}

	/* 水平结束地址 */
	LCD_WriteReg(R81, Xpos);

	/* 垂直起始地址 */
	if (Ypos >= Width)
	{
		LCD_WriteReg(R82, (Ypos - Width + 1));
	}
	else
	{
		LCD_WriteReg(R82, 0);
	}

	/* 垂直结束地址 */
	LCD_WriteReg(R83, Ypos);

	LCD_SetCursor(Xpos, Ypos);
}

/*******************************************************************************
*	函数名: LCD_WindowModeDisable
*	参  数: 无
*	返  回: 无
*	功  能: 退出窗口显示模式，变为全屏显示模式
*/
void LCD_WindowModeDisable(void)
{
	LCD_SetDisplayWindow(239, 399, 240, 400);
	LCD_WriteReg(R3, 0x1018);	/* 这个寄存器改变扫描方向 */
}

/*******************************************************************************
*	函数名: LCD_DrawLine
*	参  数: Xpos ：X坐标
*			YPos ：Y坐标
*			Length    ：长度
*			Direction ：方向(Horizontal，Vertical)
*	返  回: 无
*	功  能: 在LCD上画1条线
*/
void LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction)
{
	uint32_t i = 0;

	/* 设置线的起始坐标 */
	LCD_SetCursor(Xpos, Ypos);

	if (Direction == Horizontal)	/* 水平绘制 */
	{
		LCD_WriteRAM_Prepare();
		for (i = 0; i < Length; i++)
		{
			LCD_WriteRAM(s_TextColor);
		}
	}
	else	/* 垂直绘制 */
	{
		for (i = 0; i < Length; i++)
		{
			LCD_WriteRAM_Prepare();
			LCD_WriteRAM(s_TextColor);
			Ypos++;
			LCD_SetCursor(Xpos, Ypos);
		}
	}
}

/*******************************************************************************
*	函数名: LCD_DrawPoints
*	参  数: _points ：点坐标数组
*			_offset : 偏移坐标
*			_Size : 点个数
*			_Color ：颜色
*	返  回: 无
*	功  能: 在LCD上画一组点
*/
void LCD_DrawPoints(POINT_T _offset, POINT_T *_points, uint16_t _Size, uint16_t _Color)
{
#if 1
	uint16_t i;

	for (i = 0 ; i < _Size; i++)
	{
		/* 设置线的起始坐标 */
		LCD_SetCursor(_points[i].x + _offset.x, _points[i].y + _offset.y);
		/* 修改显存数据 */
		LCD_WriteReg(0x202, _Color);
	}
#else
	/* 下面是优化后的代码 */
	for (i = 0 ; i < _Size; i++)
	{
		LCD->LCD_REG = 0x0200;
		LCD->LCD_RAM = _points[i]->x;

		LCD->LCD_REG = 0x0201;
		LCD->LCD_RAM = _points[i]->y;

		LCD->LCD_REG = 0x0202;
		LCD->LCD_RAM = _Color;
	}
#endif

}
/*******************************************************************************
*	函数名: LCD_DrawRect
*	参  数: Xpos ：X坐标
*			YPos ：Y坐标
*			Height ：高度
*			Width  ：宽度
*	返  回: 无
*	功  能: 在LCD上填充一个矩形框
*/
void LCD_FillRect(uint16_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width,uint16_t color)
{
	u16 i;
	u16 temp=s_TextColor;
	s_TextColor=color;
	
	for(i=0;i<Height;i++)
		LCD_DrawLine(Xpos, Ypos+i, Width, Horizontal);
	
	s_TextColor= temp;
}
/*******************************************************************************
*	函数名: LCD_DrawRect
*	参  数: Xpos ：X坐标
*			YPos ：Y坐标
*			Height ：高度
*			Width  ：宽度
*	返  回: 无
*	功  能: 在LCD上画一个矩形框
*/
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width)
{
	/*
	 ---------------->---
	|(Xpos，Ypos)        |
	V                    V  Height
	|                    |
	 ---------------->---
		  Width
	*/

	LCD_DrawLine(Xpos, Ypos, Width, Horizontal);  				/* 顶 */
	LCD_DrawLine(Xpos, Ypos + Height, Width, Horizontal);		/* 底 */

	LCD_DrawLine(Xpos, Ypos, Height, Vertical);					/* 左 */
	LCD_DrawLine(Xpos + Width, Ypos, Height, Vertical);	/* 右 */
}

/*******************************************************************************
*	函数名: LCD_DrawCircle
*	参  数: Xpos ：X坐标
*			Radius ：圆的半径
*	返  回: 无
*	功  能: 在LCD上画一个圆
*/
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
	int32_t  D;			/* Decision Variable */
	uint32_t  CurX;		/* 当前 X 值 */
	uint32_t  CurY;		/* 当前 Y 值 */

	D = 3 - (Radius << 1);
	CurX = 0;
	CurY = Radius;

	while (CurX <= CurY)
	{
		LCD_SetCursor(Xpos + CurX, Ypos + CurY);
		LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
		LCD_WriteRAM(s_TextColor);

		LCD_SetCursor(Xpos + CurX, Ypos - CurY);
		LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
		LCD_WriteRAM(s_TextColor);

		LCD_SetCursor(Xpos - CurX, Ypos + CurY);
		LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
		LCD_WriteRAM(s_TextColor);

		LCD_SetCursor(Xpos - CurX, Ypos - CurY);
		LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
		LCD_WriteRAM(s_TextColor);

		LCD_SetCursor(Xpos + CurY, Ypos + CurX);
		LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
		LCD_WriteRAM(s_TextColor);

		LCD_SetCursor(Xpos + CurY, Ypos - CurX);
		LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
		LCD_WriteRAM(s_TextColor);

		LCD_SetCursor(Xpos - CurY, Ypos + CurX);
		LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
		LCD_WriteRAM(s_TextColor);

		LCD_SetCursor(Xpos - CurY, Ypos - CurX);
		LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
		LCD_WriteRAM(s_TextColor);

		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

/*******************************************************************************
*	函数名: LCD_DrawMonoPict
*	参  数: Pict ： 图片点阵指针
*	返  回: 无
*	功  能: 在LCD上画一个单色图片
*/
void LCD_DrawMonoPict(const uint32_t *Pict)
{
	uint32_t index = 0, i = 0;

	LCD_SetCursor(0, 400);

	LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
	for (index = 0; index < 2400; index++)
	{
		for (i = 0; i < 32; i++)
		{
			if ((Pict[index] & (1 << i)) == 0x00)
			{
				LCD_WriteRAM(s_BackColor);
			}
			else
			{
				LCD_WriteRAM(s_TextColor);
			}
		}
	}
}

/*******************************************************************************
*	函数名: LCD_WriteBMP
*	参  数: ptr ： 图片点阵指针
*	返  回: 无
*	功  能: 在LCD上显示一个BMP位图
*/
void LCD_WriteBMP(const uint16_t *ptr)
{
	uint32_t index = 0;

	LCD_WriteRAM_Prepare();

	for (index = 0; index < 96000; index++)
	{
		LCD->LCD_RAM = ptr[index];//*p++;
	}
}

/*******************************************************************************
*	函数名: LCD_WriteReg
*	参  数: LCD_Reg ：寄存器地址;  LCD_RegValue : 寄存器值
*	返  回: 无
*	功  能: 修改LCD控制器的寄存器的值
*/
void LCD_WriteReg(__IO uint16_t LCD_Reg, uint16_t LCD_RegValue)
{
	/* Write 16-bit Index, then Write Reg */
	LCD->LCD_REG = LCD_Reg;
	/* Write 16-bit Reg */
	LCD->LCD_RAM = LCD_RegValue;
}

/*******************************************************************************
*	函数名: LCD_ReadReg
*	参  数: LCD_Reg ：寄存器地址
*	返  回: 寄存器的值
*	功  能: 读LCD控制器的寄存器的值
*/
uint16_t LCD_ReadReg(__IO uint16_t LCD_Reg)
{
	/* Write 16-bit Index (then Read Reg) */
	LCD->LCD_REG = LCD_Reg;
	/* Read 16-bit Reg */
	return (LCD->LCD_RAM);
}

/*******************************************************************************
*	函数名: LCD_WriteRAM_Prepare
*	参  数: 无
*	返  回: 无
*	功  能: 写显存前的准备，即设置显存寄存器地址。
*/
void LCD_WriteRAM_Prepare(void)
{
	LCD->LCD_REG = 0x202;
}

/*******************************************************************************
*	函数名: LCD_WriteRAM
*	参  数: RGB_Code : 颜色代码
*	返  回: 无
*	功  能: 写显存，显存地址自动增加
*/
void LCD_WriteRAM(uint16_t RGB_Code)
{
	/* Write 16-bit GRAM Reg */
	LCD->LCD_RAM = RGB_Code;
}

/*******************************************************************************
*	函数名: LCD_ReadRAM
*	参  数: 无
*	返  回: 显存数据
*	功  能: 读显存，地址自动增加
*/
uint16_t LCD_ReadRAM(void)
{
#if 0
  /* Write 16-bit Index (then Read Reg) */
  LCD->LCD_REG = 0x202; /* Select GRAM Reg */
#endif

  /* Read 16-bit Reg */
  return LCD->LCD_RAM;
}

/*******************************************************************************
*	函数名: LCD_DisplayOn
*	参  数: 无
*	返  回: 无
*	功  能: 打开显示
*/
void LCD_DisplayOn(void)
{
	/* Display On */
	LCD_WriteReg(R7, 0x0173); /* 262K color and display ON */
}

/*******************************************************************************
*	函数名: LCD_DisplayOff
*	参  数: 无
*	返  回: 无
*	功  能: 关闭显示
*/
void LCD_DisplayOff(void)
{
	/* Display Off */
	LCD_WriteReg(R7, 0x0);
}

/*******************************************************************************
*	函数名: LCD_CtrlLinesConfig
*	参  数: 无
*	返  回: 无
*	功  能: 配置LCD控制口线，FSMC管脚设置为复用功能
*/
static void LCD_CtrlLinesConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 使能 FSMC, GPIOD, GPIOE, GPIOF, GPIOG 和 AFIO 时钟 */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
                         RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |
                         RCC_APB2Periph_AFIO, ENABLE);

  /* 设置 PD.00(D2), PD.01(D3), PD.04(NOE), PD.05(NWE), PD.08(D13), PD.09(D14),
     PD.10(D15), PD.14(D0), PD.15(D1) 为复用推挽输出 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
                                GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 |
                                GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* 设置 PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
     PE.14(D11), PE.15(D12) 为复用推挽输出 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                                GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
                                GPIO_Pin_15;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* 设置 PF.00(A0 (RS))  为复用推挽输出 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOF, &GPIO_InitStructure);

  /* 设置 PG.12(NE4 (LCD/CS)) 为复用推挽输出 - CE3(LCD /CS) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

}

/*******************************************************************************
*	函数名: LCD_FSMCConfig
*	参  数: 无
*	返  回: 无
*	功  能: 配置FSMC并口访问时序
*/
static void LCD_FSMCConfig(void)
{
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  FSMC_NORSRAMTimingInitStructure;

  /*-- FSMC Configuration ------------------------------------------------------*/
  /*----------------------- SRAM Bank 4 ----------------------------------------*/
  /* FSMC_Bank1_NORSRAM4 configuration */
  FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 1;
  FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0;
  FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 2;
  FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0;
  FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 0;
  FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0;
  FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_B;

  /* Color LCD configuration ------------------------------------
     LCD configured as follow:
        - Data/Address MUX = Disable
        - Memory Type = SRAM
        - Data Width = 16bit
        - Write Operation = Enable
        - Extended Mode = Enable
        - Asynchronous Wait = Disable */
  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

  /* - BANK 3 (of NOR/SRAM Bank 0~3) is enabled */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
}

