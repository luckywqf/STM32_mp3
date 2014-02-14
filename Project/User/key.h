#ifndef _KEY_H
#define _KEY_H
#include "stm32f10x.h"
#define USER1_KEY    GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6)
#define USER2_KEY    GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7)
#define TAMPER_KEY    GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13)
void Key_Init(void);	 

#endif
