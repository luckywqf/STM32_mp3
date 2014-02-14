#include "photo.h"
#include "bmpDecode.h"
#include "explorer.h"
#include "touch.h"
#include "lib.h"

extern u8 photoFile[MAX_FILENUM][MAX_FILENAME_LEN];
extern u8 photoFileNum;
extern u8 backKey;
extern PenHolder penPoint;

//char* FillName(char *fileName,char* src)
//{
//	u8 i;
//	strcpy(fileName,"/photo/");
//	for(i=0;src[i]!=0;i++)fileName[i+7]=src[i];
//	fileName[i+7]=0;
//	return fileName;
//}
void PhotoApp(void)
{
	u8 photoIndex=1,fresh=1;
	char fileName[46]="/photo/";
	FileCheck("/photo",BMP);
	photoIndex=1;
	while(backKey==0)
	{
		if(fresh==1)
		{
			AutoShowImage(FillName(fileName,photoFile[photoIndex]));
			fresh=0;
		}

		if(penPoint.keyState==Down)
		{
			if(GetTouchPoint())
			{
				if(penPoint.x<200)
				{
					if(photoIndex>1)
					{
						photoIndex--;
						fresh=1;
					}
				}
				else 
				{
					if(photoIndex<photoFileNum)
					{	
						photoIndex++;
						fresh=1;
					}
				}
				
			}
		}	
	}
	backKey=0;
}
