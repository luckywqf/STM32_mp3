#include "stm32f10x.h"
#include "delay.h"
#include "backLight.h"
#include "tft_lcd.h"
#include "calendar.h"
#include "eeprom.h"
#include "touch.h"
#include "key.h"
#include "lib.h"
#include "bmpDecode.h"
#include "sdio.h"
#include "vs1003.h"
#include "music.h"
#include "photo.h"
#include "book.h"
#include "setting.h"

#include<rtl.h>

extern PenHolder penPoint;
Menu_BUTTON mb=DEFAULT;

/*******************************************************************************
* Function Name  : EXTIConfig_Init
* Description    : 初始化中断向量表和中断优先级分组
*******************************************************************************/
void EXTIConfig_Init(void)
{
#ifdef  VECT_TAB_RAM                            //如果C/C++ Compiler\Preprocessor\Defined symbols中的定义了VECT_TAB_RAM 
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);   //则在RAM调试
#else                                           //如果没有定义VECT_TAB_RAM, 则定义VECT_TAB_FLASH
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0); //则在Flash里调试
#endif 
	//设置中断优先级分组，其中1位用作抢占式优先级，3位用作响应优先级，代表抢占优先级有2个和响应优先级有8个
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
}
/*******************************************************************************
* Function Name  : GetMainIndex
* Description    : 获取按键
*******************************************************************************/
Menu_BUTTON GetMainIndex(void)
{
	if(!GetTouchPoint())return DEFAULT;
	if(penPoint.y>35)
	{
		if(penPoint.y<140)
		{
			if(penPoint.x<130)return MUSIC;
			else if(penPoint.x<270)	return PHOTO;
			else return BOOK;
		}
		else
		{
			if(penPoint.x<130) return SETTING;
			else if(penPoint.x<270)	return GAME;
			else  return TIME;
		} 
	}
	else return DEFAULT;
}
/*******************************************************************************
* Function Name  : ShowMainMenu
* Description    : 显示主菜单
*******************************************************************************/
void ShowMainMenu(void)
{
	ShowImage("/icon/main.bmp",0,0);
	LCD_SetBackColor(BackColor);
	LCD_SetTextColor(Black);
	LCD_DisplayString(45,120,"音乐");
	LCD_DisplayString(175,120,"图库");
	LCD_DisplayString(300,120,"电子书");
	LCD_DisplayString(45,210,"设置");
	LCD_DisplayString(175,210,"游戏");
	LCD_DisplayString(300,210,"时间");
	
	mb=MAIN;		//显示时间
}

__task void MainApp (void) 
{
	while(1)
	{
		ShowMainMenu();
		while(penPoint.keyState==Up);
		if(penPoint.keyState==Down)
		{
			mb=GetMainIndex();
			switch (mb)
			{
			case MUSIC :
				MusicApp();
				break; 
			case PHOTO :
				PhotoApp();
				break; 
			case BOOK :
				BookApp();
				break; 
			case SETTING :
				SettingApp();
				break;
			case GAME :
			
				break; 
			case TIME :
				TimeApp();
				break; 
			}
		}
	}
}
/*******主函数********/
int main (void) 
{
	SystemInit();		 //初始化时钟
	EXTIConfig_Init();	 //设置中断优先级分组及中断向量
	Delay_Init();		 //初始化延时，使用TIM2做延时
	Eeprom_Init(); 		 //Eeprom初始化，EEPROM记录触摸屏校准数据，音量和背光
	LCD_Init();			 //LCD显示屏初始化
	BackLight_Init();	 //LCD背光初始化
	LCD_Clear(White);	 //
	VS1003_Init();		 //初始化VS1003
	Touch_Init();		 //初始化触摸屏，并校准
	Key_Init();			 //初始化按键
	NVIC_SDIOConfiguration(); //初始化SD中断
	Calendar_Init();	 //初始化 日历


	os_sys_init (MainApp);   /* Initialize RTX and start init    */                 
}
