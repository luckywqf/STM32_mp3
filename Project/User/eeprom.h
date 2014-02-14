#ifndef _EEPROM_H
#define _EEPROM_H

#include "stm32f10x.h"
#include "delay.h"
/* Private define ------------------------------------------------------------*/
#define SCL_H         GPIO_SetBits(GPIOB , GPIO_Pin_6)
#define SCL_L         GPIO_ResetBits(GPIOB , GPIO_Pin_6)  
#define SDA_H         GPIO_SetBits(GPIOB , GPIO_Pin_7)
#define SDA_L         GPIO_ResetBits(GPIOB , GPIO_Pin_7)

#define SDA_read      GPIO_ReadInputDataBit(GPIOB , GPIO_Pin_7)

#define ADDR_24LC08		0xA0			 //Æ÷¼þµØÖ·

u8 I2C_ReadByte(u8* pBuffer,   u16 length,   u16 ReadAddress,  u8 DeviceAddress);
u8 I2C_WriteByte(u8 SendByte, u16 WriteAddress, u8 DeviceAddress);
void Eeprom_Init(void);

#endif
