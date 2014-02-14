#ifndef _EXPLORER_H
#define _EXPLORER_H

#include "stm32f10x.h"

#define MAX_FILENAME_LEN	40
#define MAX_FILENUM			100+1	//索引从1开始

typedef enum {UNKNOWN,MP3,TXT,BMP}FileType;
typedef enum {SELECT=9,BACK=10,TURNUP=11,TURNDOWN=12}EXPLORER_BUTTON;

void FileCheck(char *dir,FileType ft);
u16 FileExplorer(u16 num,FileType ft);

#endif
