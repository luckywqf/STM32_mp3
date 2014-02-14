#ifndef _LIB_H
#define _LIB_H
#include "stm32f10x.h"
#include "string.h"

#define NULL 0

typedef enum{
	DEFAULT,MAIN,
	MUSIC,PHOTO,BOOK,
	SETTING,GAME,TIME,
} Menu_BUTTON;


void reverse(char s[]);
uint8_t* itoa(int n, char s[]);
char* FillName(char *fileName,char* src);

void TimeApp(void);

#endif
