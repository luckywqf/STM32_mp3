#include "bmpDecode.h"
#include "lib.h"
#include "tft_lcd.h"
#include "ff.h"

char buf[8000];
FRESULT result;
FATFS fs;
FIL file; 
uint32_t bw;
BMP_FILEHEADER *bmpHead;

/*************************自动显示图片******************************/
void AutoShowImage(char *fileName)
{
	u16 Width,Height ; 
    u16 Type ,biBitCount; 
    u32 Compression; 

	if(fileName==0)return;

	 /* 挂载文件系统 */
	result = f_mount(0, &fs);			/* Mount a logical drive */
	if (result != FR_OK)return;

	/* 打开文件 */
	result = f_open(&file, fileName, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)return;

	/* 读取文件 */
	result = f_read(&file, buf, 100, &bw);
		
	f_close(&file);	   /* 关闭文件*/
	f_mount(0, NULL);  /* 卸载文件系统 */

	bmpHead=(BMP_FILEHEADER*)buf;
	Type        = bmpHead->bfType; 
    Width       = bmpHead->biWidth; 
    Height      = bmpHead->biHeight; 
    Compression = bmpHead->biCompression; 
	biBitCount  = bmpHead->biBitCount;
	if((Type!=0x4d42)||(Compression))return;  //	不支持该图片显示
  	if(biBitCount!=16&&biBitCount!=24)return;

	LCD_Clear(Grey);

	if(Width<=400&&Height<=240)ShowImage(fileName,200-Width/2,120-Height/2);
	else ShowBigImage(fileName);
	
}

/******************显示大图片****************************/
void ShowBigImage(char *fileName)
{
	u16 srcWidth,srcHeight,destWidth,destHeight; 
    u16 biBitCount; 
    u32 OffBits; 

	u16 bytePerLine;
	u16 rgb;
	SRGB16 *srgb16;
	SRGB24 *srgb24;
	u16 i,j;
	u16 tSrcH,tSrcW; 
		
	/* 挂载文件系统 */
	result = f_mount(0, &fs);			/* Mount a logical drive */
	if (result != FR_OK)return;

	/* 打开文件 */
	result = f_open(&file, fileName, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)return;

	/* 读取文件 */
	result = f_read(&file, buf, 100, &bw);
	bmpHead=(BMP_FILEHEADER*)buf;
	 
    srcWidth    = bmpHead->biWidth; 
    srcHeight   = bmpHead->biHeight; 
	OffBits	 	= bmpHead->bfOffBits;//bmp颜色数据起始位置 
    biBitCount 	= bmpHead->biBitCount;
	bytePerLine=(((srcWidth*biBitCount)+31)>>5)<<2;
  

	if(srcWidth/5<=srcHeight/3)
	{
		destWidth=srcWidth*240/srcHeight;
		destHeight=240;
		for (i=0; i<destHeight; i++)
		{
			tSrcH=srcHeight*i/240;
			f_lseek(&file,tSrcH*bytePerLine+OffBits);
			f_read(&file, buf, bytePerLine, &bw);

			LCD_SetCursor(200-destWidth/2,destHeight-i-1);
			LCD->LCD_REG = 0x202;//即设置显存寄存器地址

			for(j=0; j<destWidth; j++)
			{
				tSrcW=srcHeight*j/240;
				
				if(biBitCount==16)		//16位图像
				{
					srgb16 = (SRGB16*)&buf[tSrcW*2];
					rgb=(srgb16->R)<<11;
					rgb+=(srgb16->G)<<6;
					rgb+=srgb16->B;
				}
				else 					// 24位图像
				{
					srgb24 = (SRGB24*)&buf[tSrcW*3];
					rgb = srgb24->R>>3;
					rgb+=srgb24->G>>2<<5;
					rgb+=srgb24->B>>3<<11;
				}
				LCD->LCD_RAM = rgb;

			}
		}		
	}
	else
	{
	 	destWidth=400;
		destHeight=srcHeight*400/srcWidth;
		for (i=0; i<destHeight; i++)
		{
			tSrcH=srcWidth*i/400;
			f_lseek(&file,tSrcH*bytePerLine+OffBits);
			f_read(&file, buf, bytePerLine, &bw);

			LCD_SetCursor(0,120+destHeight-i-1);
			LCD->LCD_REG = 0x202;//即设置显存寄存器地址

			for(j=0; j<destWidth; j++)
			{
				tSrcW=srcWidth*j/400;
				
				if(biBitCount==16)		//16位图像
				{
					srgb16 = (SRGB16*)&buf[tSrcW*2];
					rgb=(srgb16->R)<<11;
					rgb+=(srgb16->G)<<6;
					rgb+=srgb16->B;
				}
				else 					// 24位图像
				{
					srgb24 = (SRGB24*)&buf[tSrcW*3];
					rgb = srgb24->R>>3;
					rgb+=srgb24->G>>2<<5;
					rgb+=srgb24->B>>3<<11;
				}
				LCD->LCD_RAM = rgb;

			}
		}
	}			
	f_close(&file);	   /* 关闭文件*/
	f_mount(0, NULL);  /* 卸载文件系统 */

}


//只显示16位bmp未压缩图片
bool ShowImage(char *fileName,u16 x,u16 y) 
{	
 
	int Width,Height ; 
    u16 Type ,biBitCount; 
    u32 OffBits,Compression ; 

	u16 bytePerLine;
	u16 rgb;
	SRGB16 *srgb16;
	SRGB24 *srgb24;
	u16 i,j; 
		
	/* 挂载文件系统 */
	result = f_mount(0, &fs);			/* Mount a logical drive */
	if (result != FR_OK)return FALSE;

	/* 打开文件 */
	result = f_open(&file, fileName, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)return FALSE;

	/* 读取文件 */
	result = f_read(&file, buf, 100, &bw);
	bmpHead=(BMP_FILEHEADER*)buf;
	
	Type        = bmpHead->bfType; 
    Width       = bmpHead->biWidth; 
    Height      = bmpHead->biHeight; 
    Compression = bmpHead->biCompression; 
	OffBits	 	= bmpHead->bfOffBits;//bmp颜色数据起始位置 
    biBitCount 	= bmpHead->biBitCount;
	if((Type!=0x4d42)||(Compression))return FALSE;  //	不支持该图片显示
  
    result=f_lseek(&file,OffBits); //找到bmp颜色数据起始位置 
	if(result) return FALSE; 

    bytePerLine=(((Width*biBitCount)+31)>>5)<<2;	
    for(i=0;i<Height;i++) 
    { 
		result=f_read(&file,buf,bytePerLine,&bw);//每次读取一行数据
		if(result) return FALSE;

		LCD_SetCursor(x,Height+y-i-1);
		LCD->LCD_REG = 0x202;//即设置显存寄存器地址
		for(j=0;j<Width*biBitCount/8;j+=biBitCount/8)
		{
			if(biBitCount==16)		//16位图像
			{
				srgb16 = (SRGB16*)&buf[j];
				rgb=(srgb16->R)<<11;
				rgb+=(srgb16->G)<<6;
				rgb+=srgb16->B;
			}
			else 						   // 24位图像
			{
				srgb24 = (SRGB24*)&buf[j];
				rgb = srgb24->R>>3;
				rgb+=srgb24->G>>2<<5;
				rgb+=srgb24->B>>3<<11;
			}
			LCD->LCD_RAM = rgb;
		} 
 	}

	f_close(&file);	   /* 关闭文件*/
	f_mount(0, NULL);  /* 卸载文件系统 */
	return TRUE; 
} 

