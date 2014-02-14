#ifndef _BMPDECODE_H
#define _BMPDECODE_H
#include "stm32f10x.h"
#include "sdcard.h"

#pragma pack(2)	 //2字节对齐

typedef struct 
{
	//BMP位图文件头
    u16 bfType;         //Offset:0x0000,文件标志.只对'BM',用来识别BMP位图类型 
    u32 bfSize;	        //Offset:0x0002,文件大小,占四个字节;注意是大字节序还是小字节序,实际BMP格式中是小字节序  
    u16 bfReserved1;    //Offset:0x0006,保留 
	u16 bfReserved2;    //Offset:0x0008,保留 
    u32 bfOffBits;      //Offset:0x000A,从文件开始到位图数据(bitmap data)开始之间的的偏移量,看后面的图象信息头的多少定,图象信息头大小为0x28则本字段值为0x36,本字段的值实际上为"图象信息头长度+14+调色板信息长度" 

	//BMP位图信息头 
	u32 biSize;	    	 //Offset:0x000E,位图信息头字节数量。一般为0x28,最好取其实际值 
    s32 biWidth;         //Offset:0x0012,说明图象的宽度，以象素为单位  
    s32 biHeight;	     //Offset:0x0016,说明图象的高度，以象素为单位，如果这是一个正数，说明图像数据是从图像左下角到右上角排列的。 
    u16 biPlanes;	     //Offset:0x001A,为目标设备说明位面数，其值将总是被设为1  
    u16 biBitCount;	     //Offset:0x001C,说明比特数/象素，其值为1、4、8、16、24、或32 
    u32 biCompression;   //Offset:0x001E,说明图象数据压缩的类型。其值可以是下述值之一： 
                                        //BI_RGB：没有压缩； 
                                        //BI_RLE8：每个象素8比特的RLE压缩编码，压缩格式由2字节组成(重复象素计数和颜色索引)；   
                                        //BI_RLE4：每个象素4比特的RLE压缩编码，压缩格式由2字节组成 
  	                                    //BI_BITFIELDS：每个象素的比特由指定的掩码决定。 
    u32 biSizeImage;    //Offset:0x0022,说明图象的大小，以字节为单位。当用BI_RGB格式时，可设置为0   
    s32 biXPelsPerMeter;//Offset:0x0026,说明水平分辨率，用象素/米表示,有符号整数 
    s32 biYPelsPerMeter;//Offset:0x002A,说明垂直分辨率，用象素/米表示,有符号整数 
    u32 biClrUsed;	   	//Offset:0x002E,说明位图实际使用的彩色表中的颜色索引数 
    u32 biClrImportant; //Offset:0x0032,说明对图象显示有重要影响的颜色索引的数目，如果是0，表示都重要。  
}BMP_FILEHEADER;

typedef struct {
	u16 B:5;
	u16 G:5;
	u16 R:5;
	u16 reserve:1;
}SRGB16;

typedef struct {
	u8 R;
	u8 G;
	u8 B;
}SRGB24;
void AutoShowImage(char *fileName);
void ShowBigImage(char *fileName);
bool ShowImage(char *fileName,u16 x,u16 y);
#endif
