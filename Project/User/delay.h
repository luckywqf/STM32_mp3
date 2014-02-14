/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stdint.h"

/* 等待定时器超时期间，可以让CPU进入IDLE状态， 目前是空 */
#define CPU_IDLE()

void DelayMS(uint32_t nTime);
void Delay_Init(void);

#endif
