#include "stm32f10x_it.h"
#include "sdcard.h"
#include "delay.h"
#include "calendar.h"
#include "tft_lcd.h"
#include "key.h"
#include "touch.h"
#include "lib.h"

extern Menu_BUTTON mb;
u8 backKey;	//PC6
u8 selectKey; //PC7
u8 tamperKey; //PC13
extern PenHolder penPoint;
extern u8 freshPlayTime;

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/


/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : 处理SD卡中断.
*******************************************************************************/
void SDIO_IRQHandler(void)
{
  /* Process All SDIO Interrupt Sources */
  SD_ProcessIRQSrc();
}
/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : This function handles RTC global interrupt request.
*******************************************************************************/
void RTC_IRQHandler(void)
{
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	{ 		
		RTC_ClearITPendingBit(RTC_IT_SEC);	/* Clear the RTC Second interrupt */
		RTC_WaitForLastTask();	 			/* Wait until last write operation on RTC registers has finished */
		
		freshPlayTime=1;			//触发刷新时间
		if(mb==MAIN)Show_Calendar(240,5,HeadColor);	

	}	
}
/*******************************************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    : 处理触摸屏和USER1，USER2按键中断
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
	if ( EXTI_GetITStatus(EXTI_Line5) != RESET ) 
	{
		EXTI_ClearITPendingBit(EXTI_Line5);	
	    penPoint.keyState=Down; 	 	 //触摸屏按下 ----PC5
	}
	if ( EXTI_GetITStatus(EXTI_Line6) != RESET )
	{
		EXTI_ClearITPendingBit(EXTI_Line6);		
		//DelayMS(50);
		if(USER1_KEY==0)backKey=1;		//USER1键被按下	PC6
	}
	if ( EXTI_GetITStatus(EXTI_Line7) != RESET )
	{
		EXTI_ClearITPendingBit(EXTI_Line7);		
		//DelayMS(50);
		if(USER2_KEY==0)selectKey=1;		//USER2键被按下	PC7
	}				
}
/*******************************************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    : 处理TAMPER按键中断
*******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
	if ( EXTI_GetITStatus(EXTI_Line13) != RESET )
	{
		//DelayMS(50);
		//if(TAMPER_KEY==0)selectKey=1;		//USER2键被按下	PC13
		EXTI_ClearITPendingBit(EXTI_Line13);		
	}	
}

