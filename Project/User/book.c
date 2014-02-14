#include "stm32f10x.h"
#include "tft_lcd.h"
#include "touch.h"
#include "book.h"
#include "ff.h"
#include "lib.h"
#include "explorer.h"

#define MAX_PAGE_NUM 100

u8 text[602]={0};
extern u8 backKey;	//PC6
extern PenHolder penPoint;
extern u8 bookFile[MAX_FILENUM][MAX_FILENAME_LEN];
extern u8 bookFileNum;

static u32 pageIndex[MAX_PAGE_NUM]; 

/***************显示一页，返回该页显示的字节数***********************/
u16 ShowPage(char *str)
{
	u16 i;
	u8 j,lineNum=0;
	u8 linetext[51]={0};

	if(str==NULL)return 0;

	LCD_Clear(White);
	LCD_SetTextColor(Black);
	LCD_SetBackColor(White);

	for(i=0,lineNum=0;lineNum<12&&str[i]!=0;lineNum++)
	{
		j=0;
		while(j<50&&str[i]!=0)
		{
			if(str[i]=='\r'&&str[i+1]=='\n')	   //换行
			{
				i+=2;
				break;
			}		
			if(str[i]>=0x80)linetext[j++]=str[i++];	//中文编码，两个字节
		    linetext[j++]=str[i++];
		}
		linetext[j]=0;
		LCD_DisplayString(0,lineNum*20,linetext);
	}
	
	return i;			  //返回该页显示的字节数
}

/********************电子书程序**************************************/
void BookApp(void)
{
	FRESULT result;
	FATFS fs;
	FIL file; 
	uint32_t bw;

	u8 index;
	u8 fresh=1;		//是否刷新页面
	u8 end=0;		//结束标志
	u16 words;		//当前页面字节数
	u8 pages=0;		//记录当前显示的页数，便于上下翻页
	char fileName[MAX_FILENAME_LEN]="/book/";

	FileCheck("/book",TXT);			//检索文件

	index = FileExplorer(bookFileNum,TXT); 	//显示文件浏览
	if(index==0)return;						//返回键
	
	strcat(fileName,bookFile[index]);
	
	result = f_mount(0, &fs);	/* 挂载文件系统 */
	if (result != FR_OK)return ;//FALSE;

	/* 打开文件 */
	result = f_open(&file, fileName, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)return ;//FALSE;
	backKey=0;

	/* 读取文件 */
	while(backKey==0)
	{
		if(fresh==1)
		{
			pageIndex[pages]=f_tell(&file);		 //记录页指针

			result = f_read(&file, text, sizeof(text), &bw);
			if(bw<sizeof(text))end=1;
			if (result !=  FR_OK)return ;//FALSE;
			text[bw]=0;					 //置字符串结束标志
			words=ShowPage(text);
			f_lseek(&file,f_tell(&file)-bw+words);	//设置文件读取指针
			fresh=0;
		}
		if(penPoint.keyState==Down)
		{
			if(GetTouchPoint())
			{
				if(penPoint.y<120)
				{ 
					if(pages>0)
					{	
						f_lseek(&file,pageIndex[--pages]);	//设置文件读取指针
						fresh=1;
						end=0;
					}
				}
				else 
				{	
					if(end==0)
					{
						pages++;
						fresh=1;
					}
				}
			}
		}
	}

    backKey=0;

	f_close(&file);	   /* 关闭文件*/
	f_mount(0, NULL);  /* 卸载文件系统 */
}

