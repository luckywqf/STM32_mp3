#include "explorer.h"
#include "tft_lcd.h"
#include "bmpDecode.h"
#include "touch.h"
#include "ff.h"
#include "lib.h"
#include<string.h>
#include<ctype.h>

u8 musicFile[MAX_FILENUM][MAX_FILENAME_LEN]={0};
u8 photoFile[MAX_FILENUM][MAX_FILENAME_LEN]={0};
u8 bookFile[MAX_FILENUM][MAX_FILENAME_LEN]={0};
u8 musicFileNum=0;
u8 photoFileNum=0;
u8 bookFileNum=0;
extern PenHolder penPoint;


EXPLORER_BUTTON GetExplorerIndex(void)
{
	if(!GetTouchPoint())return 0;
	if(penPoint.y<20)return 0;
	else if(penPoint.y>20&&penPoint.y<220)
	{
		if(penPoint.x<360)return (penPoint.x-20)/25+1;
		else if(penPoint.y<60)return TURNUP;
		else if(penPoint.y>180)return TURNDOWN; 
	}
	else if(penPoint.x<40)return SELECT;
	else if(penPoint.x>360)return BACK;
	return 0;
}

/****************查看文件后缀名****************************/
FileType CheckProperty(char *file)
{
	u8 len=strlen(file);
	u8 i=len-3;
	for(;i<len;i++)file[i]=tolower(file[i]);
	if(strcmp(file+len-3,"mp3")==0||strcmp(file+len-3,"wav")==0)return MP3;
	if(strcmp(file+len-3,"txt")==0)	return TXT;
	if(strcmp(file+len-3,"bmp")==0)	return BMP;
	return UNKNOWN;
}
/****************将文件夹内文件计入数组***************************/
void FileCheck(char *dir,FileType ft)
{
    FRESULT result;
	FATFS fs;
	DIR DirInf;  
	FILINFO FileInf;
	uint32_t cnt = 0;
	u16 i;

#if _USE_LFN
    static char lfname[_MAX_LFN + 1];
    FileInf.lfname =&lfname[0];
    FileInf.lfsize = sizeof(lfname);
#endif

	/* 挂载文件系统 */
	result = f_mount(0, &fs);			/* Mount a logical drive */
	if (result != FR_OK)return ;
	
	/* 打开根文件夹 */
	result = f_opendir(&DirInf, dir); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)return	;

	
	/* 读取当前文件夹下的文件和目录 */
	for (cnt = 0,i=0;cnt<MAX_FILENUM ;cnt++) 
	{
		result = f_readdir(&DirInf,&FileInf); 		/* 读取目录项，索引会自动下移 */

		if (result != FR_OK || FileInf.fname[0] == 0)  	break;	//结束
		
		if (FileInf.fname[0] == '.') continue;					//隐藏文件

		if (FileInf.fattrib != AM_DIR)
		{
			if(FileInf.lfname[0] == 0)strcpy(FileInf.lfname,FileInf.fname);	//短文件名复制到长文件名，便于处理
			if(ft==CheckProperty(FileInf.lfname))			 //统计该类型文件，不是该类型，不计入
			{
				switch(ft)
				{
				case MP3:
					strcpy(musicFile[++i],FileInf.lfname);
					break;
				case BMP:
					strcpy(photoFile[++i],FileInf.lfname);
					break;
				case TXT:
					strcpy(bookFile[++i],FileInf.lfname);
					break;
				}
			}
		} 
	}
	switch(ft)
	{
	case MP3:
		musicFileNum=i;
		break;
	case BMP:
		photoFileNum=i;
		break;
	case TXT:
		bookFileNum=i;
		break;
	}

	/* 卸载文件系统 */
	f_mount(0, NULL);
}

/**********************特定类型文件浏览器***************************/
u16 FileExplorer(u16 num,FileType ft)
{
	u16 cnt = 0;
	u16 index=0;		//返回的文件名索引
	u16 curruntIndex=0;	//当前显示的页数，每页8行，页数从0开始，每次加减8
	EXPLORER_BUTTON eb;
	u8 fresh=1;

	LCD_Clear(White);	//白色背景
	LCD_FillRect(0, 0, 20, 400,BackColor);	//蓝色标题栏
	LCD_FillRect(0, 220, 20, 400,BackColor);//蓝色菜单栏
	LCD_FillRect(360, 20, 200, 40,BackColor);//蓝色滚动条

	LCD_SetTextColor(Black);
	LCD_SetBackColor(BackColor);
	LCD_DisplayString(10,2,"文件浏览器");	
	LCD_DisplayString(5,222,"选择");
	LCD_DisplayString(360,222,"返回");
	
	LCD_SetBackColor(ButtonColor);
	LCD_DisplayString(360+12,20+12,"∧");
	LCD_DisplayString(360+12,220-12-16,"∨");
									   
	LCD_DrawLine(0,20,400,Horizontal);	  //黑色分割线
	LCD_DrawLine(0,220,400,Horizontal);
	for(cnt=1;cnt<8;cnt++)LCD_DrawLine(0,cnt*25+20,360,Horizontal);
	
	while(1)
	{
		if(fresh)
		{
			//文件显示
			switch(ft)
			{
			case MP3:
				for (cnt = 1;cnt<9&&cnt+curruntIndex<=num;cnt++)
				{
					ShowImage("/icon/mp3.bmp",0,cnt*25-5);
					LCD_FillRect(30,cnt*25,16,330,White);
					LCD_DisplayString(30,cnt*25,musicFile[cnt+curruntIndex]);
				}
				break;
			case BMP:
				for (cnt = 1;cnt<9&&cnt+curruntIndex<=num;cnt++)
				{
					ShowImage("/icon/bmp.bmp",0,cnt*25-5);
					LCD_FillRect(30,cnt*25,16,330,White);
					LCD_DisplayString(30,cnt*25,photoFile[cnt+curruntIndex]);
				}
				break;
			case TXT:
				for (cnt = 1;cnt<9&&cnt+curruntIndex<=num;cnt++)
				{
					ShowImage("/icon/txt.bmp",0,cnt*25-5);
					LCD_FillRect(30,cnt*25,16,330,White);
					LCD_DisplayString(30,cnt*25,bookFile[cnt+curruntIndex]);
				}
				break;
			case UNKNOWN:
				break;
			}
			for(;cnt<9;cnt++)LCD_FillRect(0,cnt*25-3,23,360,White);		//不足一页，清楚上页内容
			fresh=0;
		}
		if(penPoint.keyState==Down)
		{
			eb=GetExplorerIndex();
			switch(eb)
			{
			case BACK:
				goto RT;
			case TURNUP:
				if(curruntIndex>0)curruntIndex-=8;
				fresh=1;
				break;
			case TURNDOWN:
				if(curruntIndex<num)curruntIndex+=8;
				fresh=1;
				break;
			default:
				index=eb+curruntIndex;
				goto RT;
			}
		}
	}
RT:	return index;
}
