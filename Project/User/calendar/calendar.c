#include "calendar.h"
#include "tft_lcd.h"
//平年的月份日期表
const uint8_t mon_table[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};
uint8_t timeStr[10]="00:00:00";
uint8_t dateStr[12]="2000/01/01";

uint8_t hour=0;
uint8_t minute=0;
uint8_t second=0;
uint16_t year=2000;
uint8_t month=1;
uint8_t day=1;
		
static uint32_t counter=0;

void Show_Calendar(u16 x,u16 y,__IO uint16_t Color);
void Calendar_Init(void);
void RTC_Configuration(void);
uint8_t Is_Leap_Year(uint16_t year);
void Set_Counter(uint16_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second);
void Get_Date(void);
void Get_Time(void);

/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
*******************************************************************************/
void RTC_Configuration(void)
{
  /* PWR and BKP clocks selection --------------------------------------------*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  
  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Reset Backup Domain */
  BKP_DeInit();

  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ON);
  /* Wait till LSE is ready */
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC registers synchronization */
  RTC_WaitForSynchro();

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
  
  /* Enable the RTC Second */  
  RTC_ITConfig(RTC_IT_SEC, ENABLE);

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
  
  /* Set RTC prescaler: set RTC period to 1sec */
  RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
  
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}
/*************************************************
函数: void NVIC_Configuration(void）
功能: NVIC(嵌套中断控制器)配置
**************************************************/
void NVIC_RTCConfiguration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
 
  /* Enable the RTC Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
/*******************************************************************************
* Function Name  : Calendar_Init
* Description    : Initializes calendar application.
*******************************************************************************/
void Calendar_Init(void)
{  	
	NVIC_RTCConfiguration();

	if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
		RTC_Configuration();	/* RTC Configuration */
		//Time_Adjust();		/* Adjust time by values entred by the user on the hyperterminal */
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	}
	else
	{
		RTC_WaitForSynchro();			  /* Wait for RTC registers synchronization */
		RTC_ITConfig(RTC_IT_SEC, ENABLE); /* Enable the RTC Second*/
		RTC_WaitForLastTask();			   /* Wait until last write operation on RTC registers has finished */
	}
	/* Clear reset flags */
	RCC_ClearFlag();

	//Show_Calendar(240,5,HeadColor);
}

void Show_Calendar(u16 x,u16 y,__IO uint16_t Color)
{
	LCD_SetBackColor(Color);
	LCD_SetTextColor(Black);
	Get_Time();
	LCD_DisplayString(x, y, timeStr);
	Get_Date();
	LCD_DisplayString(x+70, y, dateStr);	
}
//判断是否是闰年函数，输出:该年份是不是闰年.1,是.0,不是
uint8_t Is_Leap_Year(uint16_t year)
{			  
	if(year%4==0) //必须能被4整除
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//如果以00结尾,还要能被400整除 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}
//设置时钟计数值，
void Set_Counter(uint16_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second)
{
	volatile uint32_t counterSet=0;//计数值
	uint16_t t;
	if(year>2100||year<2000)year=2000;
	if(month>12||month<1)month=1;
	if(Is_Leap_Year(year)&&month==2)
	{
		if(day>29||day<1)day=1;
	}

	for(t=2000;t<year;t++)	//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t))counterSet+=31622400;//闰年的秒钟数
		else counterSet+=31536000;			  //平年的秒钟数
	}

	for(t=1;t<month;t++)	   //把前面月份的秒钟数相加
	{
		counterSet+=(uint32_t)mon_table[t]*86400;//月份秒钟数相加
		if(Is_Leap_Year(year)&&t==2)counterSet+=86400;//闰年2月份增加一天的秒钟数	   
	}
	counterSet+=(uint32_t)(day-1)*86400;//把前面日期的秒钟数相加 
	counterSet+=(uint32_t)hour*3600;//小时秒钟数
    counterSet+=(uint32_t)minute*60;	 //分钟秒钟数
	counterSet+=second;//最后的秒钟加上去
	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);   
	PWR_BackupAccessCmd(ENABLE);
	RTC_SetCounter(counterSet);		
	RTC_WaitForLastTask();	/* Wait until last write operation on RTC registers has finished */
}

void Get_Date(void)
{
	uint32_t tmp=0;

	counter=RTC_GetCounter();

	for(year=2000;;year++)
	{
		if(Is_Leap_Year(year))			//闰年的秒钟数
		{
			if(counter>=31622400)counter-=31622400;
			else break;
		}
		else 					  //平年的秒钟数
		{
			if(counter>=31536000)counter-=31536000;
			else break;								   
		}				
	}


	for(month=1;;month++)	   //把前面月份的秒钟数相加
	{
		tmp=(uint32_t)mon_table[month]*86400;//月份秒钟数相加
		if(Is_Leap_Year(year)&&month==2)tmp+=86400;//闰年2月份增加一天的秒钟数
		if(counter>=tmp)counter-=tmp;
		else break;	   
	}
	day=counter/86400+1;			
	
	tmp=year;
	dateStr[3]=tmp%10+'0';
	tmp/=10;
	dateStr[2]=tmp%10+'0';
	tmp/=10;
	dateStr[1]=tmp%10+'0';

	dateStr[5]=month/10+'0';
	dateStr[6]=month%10+'0';
	dateStr[8]=day/10+'0';
	dateStr[9]=day%10+'0';
}
void Get_Time(void)
{
	counter=RTC_GetCounter();

	hour=counter/3600%24;
	minute=counter/60%60;
	second=counter%60;
	timeStr[0]=hour/10+'0';
	timeStr[1]=hour%10+'0';
	timeStr[3]=minute/10+'0';
	timeStr[4]=minute%10+'0';
	timeStr[6]=second/10+'0';
	timeStr[7]=second%10+'0';
}
