#ifndef __BACKLIGHT_H
#define __BACKLIGHT_H

#define BRIGHT_MAX		255
#define BRIGHT_MIN		0
#define BRIGHT_DEFAULT	200

#define BRIGHT_STEP		5

void SetBackLight(uint8_t _bright);
void BackLight_Init(void);
void SaveBackLight(void);
u8 GetBackLight(void);
u8 GetAutoSet(void);
uint16_t GetLightValue(void);

#endif
