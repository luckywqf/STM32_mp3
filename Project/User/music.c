#include "music.h"
#include "explorer.h"
#include "eeprom.h"
#include "vs1003.h"
#include "tft_lcd.h"
#include "touch.h"
#include "bmpDecode.h"
#include "ff.h"
#include "lib.h"
#include<string.h>
#include<rtl.h>

extern u8 musicFile[MAX_FILENUM][MAX_FILENAME_LEN];
extern u8 musicFileNum;
extern u8 backKey;	//PC6
extern PenHolder penPoint; 

u32 currIndex=0;
char currFile[40]="/music/";	//当前播放的文件路径名
u8 play=0;			 			//播放状态，1播放，0暂停
u8 freshPlayTime=0;
u8 volume=0;
OS_TID task1,task2;
OS_SEM semaphore;

//得到保存在EEPROM里面的音量值，返回值：0,获取失败；>0成功获取数据
u8 GetVolume(void)
{					  
	u8 ReadBuffer[4];
	u8 temp;
	I2C_ReadByte(ReadBuffer,4,20, ADDR_24LC08);
	temp=ReadBuffer[1]; 		 
	if(temp&0x01)	//记录上次音量			   
	{    				   
		return ReadBuffer[0];	 
	}
	else return 0;
}

//保存音量到EEPROM，FM24C16的 20~21这段地址区间，总共2个字节
void SaveVolume(u8 vl)
{
	I2C_WriteByte(vl, 20, ADDR_24LC08);
	I2C_WriteByte(0xff, 21, ADDR_24LC08);							 		 
}
//设置并显示音量，范围0-100
void SetVolume(u8 volume)
{	
	VS1003_SetVol(volume);

	volume*=2;
	LCD_FillRect(20,215-volume,volume,20,Blue);
	LCD_FillRect(20,15,200-volume,20,White);
}

//将播放歌曲索引保存在以30开始的地址里
void SaveIndex(u32 index)
{
	u8 temp,i;
	u32 andNum;
	for(i=0,andNum=0xFF;i<4;i++,andNum<<=8)
	{
		temp = index&&andNum;
		I2C_WriteByte(temp, 30+i, ADDR_24LC08);
	}
}
u32 GetIndex()
{
	u8 i,ReadBuffer[4];
	u32 temp=0;
	I2C_ReadByte(ReadBuffer,4,30, ADDR_24LC08);

	for(i=0;i<4;i++)
	{
		temp<<=8;
		temp+=ReadBuffer[i];
	}
	
	return temp;
}
/**********************计算歌曲文件头长度****************************************
从文件头搜索前十个字节，判断最初三个字节是否有ID3的标识，如果没有，证明标签头不存在。
然后计算标签大小：一共四个字节，但每个字节只用7位，最高位不使用恒为0。所以格式如下 
0xxxxxxx 0xxxxxxx 0xxxxxxx 0xxxxxxx 计算大小时要将0去掉，得到一个28位的二进制数，就是标签大小。
*********************************************************************************/
u32 CalHeaderLen(u8 buf[32])
{
	u32 result;
	if(buf[0]=='I'&&buf[1]=='D'&&buf[2]=='3')
	{
		result=(buf[6]&0x7F)*0x200000+(buf[7]&0x7F)*0x4000+(buf[8]&0x7F)*0x80+(buf[9]&0x7F);
		return result;
	}
	else return 0;	//文件头无法识别
			
}
/***********************计算歌曲比特率********************************  
只是对固定比特率（CBR）的MP3总时长的计算方式，对于变比特率(VBR)的MP3，
由于每帧的比特率可能不同，用以上的公式就无法准确算出来mp3总时长
	播放时间 = 总帧bytes ÷ 比特率 × 8000
********************************************************************/
u16 CalBitRate(u8 buf[32])
{
	const u16 bitrate[6][15]={								 //单位Kbps
	{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448},//	 MPEG-1  Layer I
	{0,32,48,56,64,80,96,112,128,160,192,224,256,320,384},	 //			 Layer II
	{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320},	 //			 Layer III
	{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448},//	 MPEG-2  Layer I
	{0,32,48,56,64,80,96,112,128,160,192,224,256,320,384},	 //			 Layer II
	{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160}};		 //			 Layer III
	u8 layer,mpegVersion,index;
	u8 *i=strstr(buf,"\xFF");
	if(i==0)return 0;
	else
	{
		i++;
		layer=(*i>>1)&0x03;
		mpegVersion=(*i>>3)&0x03;
		i++;
		index=*i>>4;
		switch(layer)
		{
		case 1:
			if(mpegVersion==2)return bitrate[5][index];
			else if(mpegVersion==3) return bitrate[2][index];
			break;
		case 2:
			if(mpegVersion==2)return bitrate[4][index];
			else if(mpegVersion==3) return bitrate[1][index];
			break;
		case 3:
			if(mpegVersion==2)return bitrate[3][index];
			else if(mpegVersion==3) return bitrate[0][index];
			break;
		}
		return 0;			
	} 
}
/****************************************************************
			显示播放时间
*****************************************************************/
void ShowTime(u16 time,u16 decodeTime)
{
	u16 temp;
	u16 timev=time;
	u16 decodev=decodeTime;
	u8 timeStr[]="00:00/00:00";
	timeStr[4]=decodeTime%10+'0';
	decodeTime/=10;
	timeStr[3]=decodeTime%6+'0';
	decodeTime/=6;
	timeStr[1]=decodeTime%10+'0';
	decodeTime/=10;
	timeStr[0]=decodeTime%10+'0';

	timeStr[10]=time%10+'0';
	time/=10;
	timeStr[9]=time%6+'0';
	time/=6;
	timeStr[7]=time%10+'0';
	time/=10;
	timeStr[6]=time%10+'0';

	LCD_DisplayString(180,200,timeStr);

	if(timev>=decodev)
	{
		temp=decodev*200;
		temp/=timev;	
		LCD_FillRect(180,220,10,temp,Red);
		LCD_FillRect(180+temp,220,10,200-temp,White);
	}
}
void LoadGUI(void)
{
	/**************显示界面**********************/
	ShowImage("/icon/music.bmp",0,0);
	LCD_SetBackColor(ButtonColor);
	LCD_SetTextColor(Black);
	if(play==1)LCD_DisplayString(255+9,160+4,"暂停");
	else LCD_DisplayString(255+9,160+4,"播放");
	LCD_DisplayString(180+6,160+4,"上一首");
	LCD_DisplayString(320+6,160+4,"下一首");
	LCD_FillRect(100,200,25,60,ButtonColor);
	LCD_SetTextColor(Black);
	LCD_DrawRect(100,200,25,60);
	LCD_DisplayString(100+14,200+5,"列表");
	
	SetVolume(volume);

	LCD_FillRect(100,60,16,250,ButtonColor);
	LCD_DisplayString(100,60,musicFile[currIndex]);			//显示歌曲名
		
}
/****************************************************************
		用于音乐播放的进程	 ，论文可以加入信号量机制
*****************************************************************/
__task void MusicPlay (void)
{
	FRESULT result;
	FATFS fs;
	FIL file;
	u8 buf[32];
	u32 bw;
	u8 tempIndex;  //当前播放歌曲索引，与切换的歌曲索引分开
	u32 headerLen; //文件头长度
	u16 bitRate;   //比特率
	u16 time;	   //歌曲总时长
	u16 decodeTime;//当前播放时长
	
		
	while(1)							   //连续播放歌曲
	{
CS:		tempIndex=currIndex;
		FillName(currFile,musicFile[tempIndex]);

		result = f_mount(0, &fs);			/* 挂载文件系统 */	
		if (result != FR_OK)return;
		result = f_open(&file, currFile, FA_OPEN_EXISTING | FA_READ); 	/* 打开文件 */
		if (result != FR_OK)return;

		LCD_FillRect(100,60,16,250,ButtonColor);
		LCD_DisplayString(100,60,musicFile[tempIndex]);			//显示歌曲名

		f_read(&file, buf, sizeof(buf), &bw);  //读取头信息，计算头长度，为了后面计算时长
		headerLen = CalHeaderLen(buf);

		if(headerLen!=0)f_lseek(&file,headerLen+10); //正确找到
		else f_lseek(&file,0);

		f_read(&file, buf, sizeof(buf), &bw);  //读取帧头信息，计算比特率
		bitRate=CalBitRate(buf);
		if(bitRate!=0) time=(file.fsize-headerLen-10-128)/(bitRate/8)/1000;
		else time=0;

		ShowTime(time,0);			//初始显示时间

		VS1003_Reset();
		VS1003_SoftReset();			//reset后需要设置音量
		SetVolume(volume);

		//播放一首歌曲
		while(1)	
		{
			if(play==1&&tempIndex==currIndex)
			{	
				result = f_read(&file, buf, sizeof(buf), &bw);
				if (result != FR_OK)break;
				 	
				while( MP3_DREQ ==0 ){}	 /* 等待空闲 */

				tsk_lock ();	     
				VS1003_WriteData(buf);
				tsk_unlock ();

				if(bw<31)break;
			}
			else if(tempIndex!=currIndex)
			{
				f_close(&file);		// 关闭文件
				f_mount(0, NULL);  /* 卸载文件系统 */
				goto CS;			//Change song 切换歌曲
			}
			if(freshPlayTime==1&&play==1)						   //刷新时间，有RTC秒中断触发
			{
				freshPlayTime = 0;
				decodeTime = VS1003_GetDecodeTime();
				ShowTime(time,decodeTime);
			}				   	
		}
		if(currIndex==musicFileNum)currIndex = 1;	  //循环播放
		else currIndex++;
		f_close(&file);		// 关闭文件
		f_mount(0, NULL);  /* 卸载文件系统 */
	}
}


void MusicApp(void)
{
	u8 temp;
	play=1;			

	/**************初始化VS1003*********************/
	VS1003_Reset();
	VS1003_SoftReset();	

	/**************初始化音量*********************/
	volume=GetVolume();
	if(volume==0)volume=70;

	LoadGUI();		//显示界面
	

	FileCheck("/music",MP3);
	currIndex = GetIndex();
	if(currIndex<=0||currIndex>musicFileNum)currIndex=1;
		
	task1 = os_tsk_create (MusicPlay, 2);
	

	while(backKey==0)
	{
		if(penPoint.keyState==Down)
		{
			if(GetTouchPoint())
			{
				if(IsInRect(255,160,50,25))			//播放暂停
				{
					if(play==0){	play=1;	LCD_DisplayString(255+9,160+4,"暂停");	}
					else {	play=0;	LCD_DisplayString(255+9,160+4,"播放");	}
				}
				else if(IsInRect(180,160,60,25))	//上一首
				{
					if(currIndex>1)
					{
						currIndex--;
					}
					else  currIndex = musicFileNum;
				}
				else if(IsInRect(320,160,60,25))	//下一首
				{
					if(currIndex<musicFileNum)
					{
						currIndex++;
					}
					else currIndex = 1;
				}
				else if(IsInRect(20,15,20,200))		//音量
				{
					volume=100-(penPoint.y-20)/2;
					SetVolume(volume);
				}
				else if(IsInRect(100,200,60,25))	//列表
				{
					tsk_lock ();
					temp=FileExplorer(musicFileNum,MP3);
					if(temp!=0)currIndex=temp;
					LoadGUI();
					tsk_unlock ();
				}
			}
		}
		os_dly_wait (20);	
	}
	backKey=0;
	os_tsk_delete (task1);    	//删除音乐播放任务

	SaveIndex(currIndex);
	SaveVolume(volume);	//退出时保存音量，

	f_mount(0, NULL);  	// 卸载文件系统 
}

