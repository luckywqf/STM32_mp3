#include "setting.h"
#include "tft_lcd.h"
#include "backLight.h"
#include "touch.h"

extern PenHolder penPoint;
extern u8 backKey;
extern uint8_t autosetBackLight;
extern uint8_t backLightValue;

void ShowBackLightBar(u8 light)
{
	LCD_FillRect(100,130,30,light*2,Red);
	LCD_FillRect(100+light*2,130,30,200-light*2,White);	
}
void SettingApp(void)
{
	u16 value;
	LCD_Clear(Blue);
	LCD_SetTextColor(Black);
	LCD_SetBackColor(Blue);
	LCD_DisplayString(130,80,"自动调节屏幕亮度");
	LCD_SetBackColor(White);
	LCD_DisplayString(290,80,"  ");

	if(autosetBackLight)LCD_DisplayString(290,80,"√");
	else
	{
		LCD_SetBackColor(Blue);
		LCD_DisplayString(150,160,"调节屏幕亮度");
		ShowBackLightBar(backLightValue*100/255);
	}
	while(backKey==0)
	{
		if(penPoint.keyState==Down)
		{
			if(GetTouchPoint())
			{
				if(autosetBackLight)
				{
					if(IsInRect(280,70,30,30))
					{
						autosetBackLight=0;
						LCD_SetBackColor(White);
						LCD_DisplayString(290,80,"  ");
						LCD_SetBackColor(Blue);
						LCD_DisplayString(150,160,"调节屏幕亮度");
						ShowBackLightBar(backLightValue*100/255);
					}
				}
				else 
				{
					if(IsInRect(280,70,30,30))
					{
						autosetBackLight=1;
						LCD_SetBackColor(White);
						LCD_DisplayString(290,80,"√");
						LCD_FillRect(100,130,80,200,Blue);
						value=GetLightValue();
						backLightValue=255-(value>>7);
						SetBackLight(backLightValue);
					}
					else if(IsInRect(100,130,200,30))
					{
						backLightValue=(penPoint.x-100)*5/4;
						ShowBackLightBar((penPoint.x-100)/2);
						SetBackLight(backLightValue);		
					}
				}
			}
		}
	}
	backKey=0;
	SaveBackLight();	   //保存配置值
}



