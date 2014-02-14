#include "stm32f10x.h"
#include "calendar.h"
#include "touch.h"
#include "tft_lcd.h"
#include "lib.h"
#include "key.h"

u16 timeButtonPos[5]={20,60,100,140,180};
typedef enum{
	UP_YEAR=1,UP_MONTH,UP_DAY,UP_HOUR,UP_MINUTE,
	DOWN_YEAR,DOWN_MONTH,DOWN_DAY,DOWN_HOUR,DOWN_MINUTE	
} TIME_BUTTON;

extern uint16_t year;
extern uint8_t month;
extern uint8_t day;
extern uint8_t hour;
extern uint8_t minute;
extern PenHolder penPoint;
extern const uint8_t mon_table[13];
extern u8 backKey;

char yearStr[]="2000";
char monthStr[]="01";
char dayStr[]="01";
char hourStr[]="00";
char minuteStr[]="00";

TIME_BUTTON GetTimeIndex(void)
{
	if(!GetTouchPoint())return DEFAULT;
	if(penPoint.y>20&&penPoint.y<40)
	{
		if(penPoint.x>timeButtonPos[0]&&penPoint.x<timeButtonPos[0]+20)return UP_YEAR;
		else if(penPoint.x>timeButtonPos[1]&&penPoint.x<timeButtonPos[1]+20)return UP_MONTH;
		else if(penPoint.x>timeButtonPos[2]&&penPoint.x<timeButtonPos[2]+20)return UP_DAY;
		else if(penPoint.x>timeButtonPos[3]&&penPoint.x<timeButtonPos[3]+20)return UP_HOUR;
		else if(penPoint.x>timeButtonPos[4]&&penPoint.x<timeButtonPos[4]+20)return UP_MINUTE;
		else return DEFAULT;

	}
	else if(penPoint.y>60&&penPoint.y<80)
	{
		if(penPoint.x>timeButtonPos[0]&&penPoint.x<timeButtonPos[0]+20)return DOWN_YEAR;
		else if(penPoint.x>timeButtonPos[1]&&penPoint.x<timeButtonPos[1]+20)return DOWN_MONTH;
		else if(penPoint.x>timeButtonPos[2]&&penPoint.x<timeButtonPos[2]+20)return DOWN_DAY;
		else if(penPoint.x>timeButtonPos[3]&&penPoint.x<timeButtonPos[3]+20)return DOWN_HOUR;
		else if(penPoint.x>timeButtonPos[4]&&penPoint.x<timeButtonPos[4]+20)return UP_MINUTE;
		else return DEFAULT;
	}
	else return DEFAULT;
}
void TimeClick(TIME_BUTTON button)
{
	switch (button)
	{
	case UP_YEAR:
		if(year<2099)year++;
		LCD_DisplayString(6,42,itoa(year,yearStr));
		break;
	case UP_MONTH:
		if(month<12)month++;
		LCD_DisplayString(62,42,itoa(month,monthStr));
		break;
	case UP_DAY:
		if(month!=2)
		{
			if(day<mon_table[month])day++;
		}
		else if(Is_Leap_Year(year)&&day<29)day++;
		else if(!Is_Leap_Year(year)&&day<29)day++;
		LCD_DisplayString(102,42,itoa(day,dayStr));
		break;
	case UP_HOUR:
		if(hour<23)hour++;
		LCD_DisplayString(142,42,itoa(hour,hourStr));
		break;
	case UP_MINUTE:
		if(minute<59)minute++;
		LCD_DisplayString(182,42,itoa(minute,minuteStr));
		break;
	case DOWN_YEAR:
		if(year>2000)year--;
		LCD_DisplayString(6,42,itoa(year,yearStr));
		break;
	case DOWN_MONTH:
		if(month>1)month--;
		LCD_DisplayString(62,42,itoa(month,monthStr));
		break;
	case DOWN_DAY:
		if(day>1)day--;
		LCD_DisplayString(102,42,itoa(day,dayStr));
		break;
	case DOWN_HOUR:
		if(hour>0)hour--;
		LCD_DisplayString(142,42,itoa(hour,hourStr));
		break;
	case DOWN_MINUTE:
		if(minute>0)minute--;
		LCD_DisplayString(182,42,itoa(minute,minuteStr));
		break;
	}
}
void TimeApp(void)
{
	u8 i;
	TIME_BUTTON b;


	LCD_Clear(BackColor);
	LCD_SetBackColor(ButtonColor);
	for(i=0;i<5;i++)
	{	
		//上按键
		LCD_DrawRect(timeButtonPos[i],20,20,20); 	
		LCD_DisplayString(timeButtonPos[i]+2,22,"∧");
		//下按键
		LCD_DrawRect(timeButtonPos[i],60,20,20);	
		LCD_DisplayString(timeButtonPos[i]+2,62,"∨");
	}
	Get_Date();
	Get_Time();
	LCD_SetBackColor(BackColor);
	LCD_DisplayString(6,42,itoa(year,yearStr));
	LCD_DisplayString(62,42,itoa(month,monthStr));
	LCD_DisplayString(102,42,itoa(day,dayStr));
	LCD_DisplayString(142,42,itoa(hour,hourStr));
	LCD_DisplayString(182,42,itoa(minute,minuteStr));
	LCD_DisplayString(38,42,"年");
	LCD_DisplayString(78,42,"月");
	LCD_DisplayString(118,42,"日");
	LCD_DisplayString(158,42,"时");
	LCD_DisplayString(198,42,"分");
	while(backKey==0)
	{
		if(penPoint.keyState==Down)
		{
			b = GetTimeIndex();
			TimeClick(b);
		}
	}
	backKey=0;
	Set_Counter(year,month,day,hour,minute,0);
}

