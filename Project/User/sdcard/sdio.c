#include "sdio.h"
/***功能说明：配置SDIO中断***/
void NVIC_SDIOConfiguration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		
	NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
