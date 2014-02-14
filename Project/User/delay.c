#include "delay.h"
#include "stm32f10x_tim.h"
uint32_t volatile gTimer_1ms=0;
/*******************************************************************************
* Function Name  : NVIC_Configuration	 
* Description    : 配置TIM2中断
*******************************************************************************/
void NVIC_TIM2Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;  

  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
}
/*************************************************
函数: void TIM2_IRQHandler(void)
功能: TIM2中断处理函数
说明：1ms中断1次
**************************************************/
void TIM2_IRQHandler(void)
{
   if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)
   {      
      TIM_ClearITPendingBit(TIM2,TIM_IT_Update);	 //清除中断标志
      gTimer_1ms++;
   }
}
/*************************************************
函数: void Timer2_Configuration(void)
功能: TIM2 配置
定时计算：(1 /(72 / (36 - 1 + 1))) * 2000 us = 1000us  = 1ms
**************************************************/
void Timer2_Configuration(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	    //打开TIM2定时器的时钟
    
  TIM_DeInit(TIM2);		                                    //TIMx寄存器重设为缺省值
  
  TIM_TimeBaseStructure.TIM_Period = 2000;		            //自动重装载寄存器周期的值
  TIM_TimeBaseStructure.TIM_Prescaler=36 - 1;               //TIMx时钟频率除数的预分频值
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //采样分频
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  
  TIM_ARRPreloadConfig(TIM2, ENABLE);                       //允许自动重装载寄存器（ARR）
  TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);	                //允许TIM2溢出中断
  
  TIM_Cmd(TIM2, ENABLE);	                                //TIM2开启时钟
}
/*************************************************
函数: void Delay(u32 nTime)
功能: 利用TIM2实现精确的延时
参数: nTime
返回: 无
**************************************************/
void DelayMS(uint32_t nTime)
{
  uint32_t count;
  
  count = gTimer_1ms;

  while((gTimer_1ms - count) < nTime);//定时时间到才退出
  
}
void Delay_Init(void)
{
	NVIC_TIM2Configuration();    //配置中断 
	Timer2_Configuration();  //定时器2初始化
}
