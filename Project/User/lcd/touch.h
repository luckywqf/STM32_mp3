#ifndef _TOUCH_H
#define _TOUCH_H

#include "stm32f10x.h"
#include "eeprom.h"
#include "tft_lcd.h"
#include "delay.h"

#define ADJUST_TOUCH

#define TP_CS()  GPIO_ResetBits(GPIOG,GPIO_Pin_11)
#define TP_DCS() GPIO_SetBits(GPIOG,GPIO_Pin_11)
#define TOUCH_PEN    GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)


//触摸屏返回值范围
#define MIN_X_VAL	0x010
#define MAX_X_VAL	0xFF0
#define MIN_Y_VAL	0x010
#define MAX_Y_VAL	0xFF0

typedef enum{ Up, Down } TouchState; 
typedef struct 	  //笔杆结构体
{
	u16 x; 
	u16 y;						   	    
	TouchState  keyState;//笔的状态
}PenHolder;

typedef struct	//触摸屏校准参数   x= xFactor*Px + xOffset
{
	float xfac;
	float yfac;
	short xoff;
	short yoff;	
}PenConfig;


void Pen_Int_Set(u8 en);
void Touch_Init(void);
u8 GetTouchPoint(void);
u8 GetTouchValue(void);
u8 IsInRect(u16 startx,u16 starty,u16 sizex,u16 sizey);
#endif
