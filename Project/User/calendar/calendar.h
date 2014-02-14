#ifndef __CALENDAR_H
#define __CALENDAR_H

#include "stm32f10x.h"

void Show_Calendar(u16 x,u16 y,__IO uint16_t Color);
void Calendar_Init(void);
void RTC_Configuration(void);
uint8_t Is_Leap_Year(uint16_t year);
void Set_Counter(uint16_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second);
void Get_Date(void);
void Get_Time(void);

#endif /* __CALENDAR_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
