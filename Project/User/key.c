#include "key.h"

/*	Tamper Button -> PC13 , User1 Button -> PC6 , User2 Button -> PC7	*/				
void GPIO_KeyConfiguration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;  
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);
					 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource6);
 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource7);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource13);
}
void NVIC_KeyConfiguration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	 													
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	      
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;   
	NVIC_Init(&NVIC_InitStructure);
}
void EXTI_KeyConfiguration(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 
	
	EXTI_ClearITPendingBit(EXTI_Line13 | EXTI_Line6 | EXTI_Line7);
	
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_Line = EXTI_Line6 | EXTI_Line7 | EXTI_Line13;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

void Key_Init(void)
{
	GPIO_KeyConfiguration();
	NVIC_KeyConfiguration();
	EXTI_KeyConfiguration();
}


