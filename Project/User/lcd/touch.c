#include"touch.h"
#include "lib.h"
#include <math.h>

PenHolder penPoint;
PenConfig penConfig;

void SPI_TouchConfiguration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;

	//GPIOA Periph clock enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOG,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//SPI1 Periph clock enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);

	//Configure SPI1 pins: SCK, MISO and MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //复用推挽输出
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	//Configure PG11 pin: TP_CS pin
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//推挽输出
	GPIO_Init(GPIOG,&GPIO_InitStructure);

	//Configure PB5 pin: TP_BUSY pin
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;     //上拉输入
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	/* Configure PC5 as input floating For TP_IRQ*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC,&GPIO_InitStructure);

	// SPI1 Config
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;   //SPI_NSS_Hard
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1,&SPI_InitStructure);

	// SPI1 enable
	SPI_Cmd(SPI1,ENABLE);
}
/*******************************************************************************
* Function Name  : EXTI_Configuration
* Description    : Configures the different EXTI lines.
*******************************************************************************/
void EXTI_Configuration(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 

  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource5);

  EXTI_ClearITPendingBit(EXTI_Line5);

  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_Line = EXTI_Line5;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures the nested vectored interrupt controller.
*******************************************************************************/
void NVIC_TouchConfiguration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure; 
  													
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;  
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	      
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 					 
}

u8 SPI_WriteByte(u8 data)
{
	u8 Data = 0;
	//Wait until the transmit buffer is empty
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET);
	// Send the byte
	SPI_I2S_SendData(SPI1,data); 	
	//Wait until a data is received
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET);
	// Get the received data
	Data = SPI_I2S_ReceiveData(SPI1);	
	// Return the shifted data
	return Data;
}

void SpiDelay(u32 DelayCnt)
{
	u32 i;
	for(i=0;i<DelayCnt;i++);
}
//读取触摸屏X返回值
u16 TouchReadX(void)
{
   u16 x=0;
   TP_CS();
   SpiDelay(10);
   SPI_WriteByte(0x90);
   SpiDelay(10);
   x=SPI_WriteByte(0x00);
   x<<=8;
   x+=SPI_WriteByte(0x00);
   SpiDelay(10);
   TP_DCS();
   x = x>>3;
   return (x);
}
//读取触摸屏Y返回值
u16 TouchReadY(void)
{
	u16 y=0;
	TP_CS();
	SpiDelay(10);
	SPI_WriteByte(0xD0);
	SpiDelay(10);
	y=SPI_WriteByte(0x00);
	y<<=8;
	y+=SPI_WriteByte(0x00);
	SpiDelay(10);
	TP_DCS();
	y = y>>3;
	return (y);
}

u16  TPReadX(void)
{
	u8 t=0,t1,count=0;
	u16 databuffer[9]={0};//数据组
	u16 temp=0,X=0;

	while(count<9)//循环读数10次
	{
		databuffer[count]=TouchReadX();
		count++;
	}
	if(count==9)//一定要读到10次数据,否则丢弃
	{
	    do//将数据X升序排列
		{
			t1=0;
			for(t=0;t<count-1;t++)
			{
				if(databuffer[t]>databuffer[t+1])//升序排列
				{
					temp=databuffer[t+1];
					databuffer[t+1]=databuffer[t];
					databuffer[t]=temp;
					t1=1;
				}
			}
		}while(t1);
		X=(databuffer[3]+databuffer[4]+databuffer[5])/3;
	}
	return(X);
}

u16  TPReadY(void) 
{
  	u8 t=0,t1,count=0;
	u16 databuffer[9]={0};//数据组
	u16 temp=0,Y=0;

    while(count<9)	//循环读数10次
	{
		databuffer[count]=TouchReadY();
		count++;
	}
	
	if(count==9)//一定要读到10次数据,否则丢弃
	{
	    do//将数据X升序排列
		{
			t1=0;
			for(t=0;t<count-1;t++)
			{
				if(databuffer[t]>databuffer[t+1])//升序排列
				{
					temp=databuffer[t+1];
					databuffer[t+1]=databuffer[t];
					databuffer[t]=temp;
					t1=1;
				}
			}
		}while(t1);
		Y=(databuffer[3]+databuffer[4]+databuffer[5])/3;
	}
	return(Y);
}

u8 GetTouchValue(void)
{
	int x[2],y[2];

	Pen_Int_Set(0);//关闭中断

	x[0]=TPReadX();
	y[0]=TPReadY();
	x[1]=TPReadX();
	y[1]=TPReadY();

	while(TOUCH_PEN==0);	//等待放开屏幕

	Pen_Int_Set(1);//开中断
	
	if(x[0]>MAX_X_VAL||x[0]<MIN_X_VAL||y[0]>MAX_Y_VAL||y[0]<MIN_Y_VAL)return 0;

	if(abs((float)x[0]-x[1])>50||abs((float)y[0]-y[1])>50)return 0;
	else
	{
		penPoint.x=(x[0]+x[1])/2;
		penPoint.y=(y[0]+y[1])/2;
		return 1;
	}
}
u8 GetTouchPoint(void)
{
	u8 res;
		
	if(GetTouchValue())
	{
		penPoint.x=penPoint.x*penConfig.xfac+penConfig.xoff;
		penPoint.y=penPoint.y*penConfig.yfac+penConfig.yoff;
		res=1;
	}
	else
		res=0;

	penPoint.keyState=Up;
	return res;
}
//得到保存在EEPROM里面的校准值
//返回值：1，成功获取数据
//        0，获取失败，要重新校准
u8 get_adjdata(void)
{					  
	u32 *tempfac;
	u16 *tempfac16;
	u8 ReadBuffer[20];
	u8 temp;
	I2C_ReadByte(ReadBuffer,20,0, ADDR_24LC08);
	temp=ReadBuffer[12]; 		 
	if(temp&0x01)//触摸屏已经校准过了			   
	{    
		tempfac=(u32*)&ReadBuffer[0];		   
		penConfig.xfac=(float)*tempfac/100000000;//得到x校准参数

		tempfac=(u32*)&ReadBuffer[4];	         
		penConfig.yfac=(float)*tempfac/100000000;//得到y校准参数
	    //得到x偏移量
		tempfac16=(u16*)&ReadBuffer[8];  	  
		penConfig.xoff=*tempfac16;					 
	    //得到y偏移量
		tempfac16=(u16*)&ReadBuffer[10];	  
		penConfig.yoff=*tempfac16;				   
		return 1;	 
	}
	return 0;
}
//保存校准参数
//保存区域：FM24C16的 0~12这段地址区间，总共12个字节
void save_adjdata(void)
{
	u32 temp;			 
	//保存校正结果!	    
	temp=penConfig.xfac*100000000;//保存x校正因素  
	I2C_WriteByte(temp&0xff, 0, ADDR_24LC08);
	I2C_WriteByte((temp>>8)&0xff, 1, ADDR_24LC08);
	I2C_WriteByte((temp>>16)&0xff, 2, ADDR_24LC08);
	I2C_WriteByte((temp>>24)&0xff, 3, ADDR_24LC08);

	temp=penConfig.yfac*100000000;//保存y校正因素
	I2C_WriteByte(temp&0xff, 4, ADDR_24LC08);
	I2C_WriteByte((temp>>8)&0xff, 5, ADDR_24LC08);
	I2C_WriteByte((temp>>16)&0xff, 6, ADDR_24LC08);
	I2C_WriteByte((temp>>24)&0xff, 7, ADDR_24LC08);
	 
	//保存x偏移量
	I2C_WriteByte(penConfig.xoff&0xff,8,ADDR_24LC08);
	I2C_WriteByte((penConfig.xoff>>8)&0xff,9,ADDR_24LC08);
	//保存y偏移量
	I2C_WriteByte(penConfig.yoff&0xff,10,ADDR_24LC08);
	I2C_WriteByte((penConfig.yoff>>8)&0xff,11,ADDR_24LC08);
	
	I2C_WriteByte(0xff,12,ADDR_24LC08);  //标记校准过了  							 		 
}

//PEN中断设置	 
void Pen_Int_Set(u8 en)
{
	if(en)EXTI->IMR|=1<<5;   //开启line1上的中断	  	
	else EXTI->IMR&=~(1<<5); //关闭line1上的中断	   
}


//判断校准是否准确

u8 IsOk(u16 pos_temp[][2])
{
	u32 tem1,tem2;
	u32 d1,d2;
	float fac;

	tem1=abs((float)pos_temp[0][0]-pos_temp[1][0]);//x1-x2
	tem2=abs((float)pos_temp[0][1]-pos_temp[1][1]);//y1-y2
	tem1*=tem1;
	tem2*=tem2;
	d1=sqrt(tem1+tem2);//得到1,2的距离
	
	tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
	tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
	tem1*=tem1;
	tem2*=tem2;
	d2=sqrt(tem1+tem2);//得到3,4的距离
	fac=(float)d1/d2;

	if(fac<0.95||fac>1.05||d1==0||d2==0)return 0;//不合格

	tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
	tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
	tem1*=tem1;
	tem2*=tem2;
	d1=sqrt(tem1+tem2);//得到1,3的距离
	
	tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
	tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
	tem1*=tem1;
	tem2*=tem2;
	d2=sqrt(tem1+tem2);//得到2,4的距离
	fac=(float)d1/d2;

	if(fac<0.95||fac>1.05||d1==0||d2==0)return 0;//不合格

				   
	//对角线相等
	tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
	tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
	tem1*=tem1;
	tem2*=tem2;
	d1=sqrt(tem1+tem2);//得到1,4的距离

	tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
	tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
	tem1*=tem1;
	tem2*=tem2;
	d2=sqrt(tem1+tem2);//得到2,3的距离
	fac=(float)d1/d2;

	if(fac<0.95||fac>1.05||d1==0||d2==0)return 0;//不合格

	return 1;
}
//触摸屏校准代码
//得到四个校准参数
void touch_adjust(void)
{
	u16 tempVal[4][2]={0};
	u16 adjPointX[4]={20,340,20,340};
	u16 adjPointY[4]={20,20,220,220};
	u8 i;
						 
	LCD_Clear(White);//清屏 
	LCD_SetBackColor(White);
	LCD_SetTextColor(Black);
	LCD_DisplayString(24,110,"点击屏幕上红点进行校准"); 
	DelayMS(1000);
	
	LCD_Clear(White);//清屏 
	LCD_SetTextColor(Red);
	penPoint.keyState=Up;
	
AG:	i=0;
	do
	{		
		LCD_DrawCircle(adjPointX[i],adjPointY[i],1);
		LCD_DrawCircle(adjPointX[i],adjPointY[i],2);
		LCD_DrawCircle(adjPointX[i],adjPointY[i],3);
		LCD_DrawCircle(adjPointX[i],adjPointY[i],4);

		if(penPoint.keyState==Down)
		{
			if(GetTouchValue())
			{
				tempVal[i][0]=penPoint.x;
				tempVal[i][1]=penPoint.y;
				LCD_Clear(White);//清屏
				DelayMS(100);	
				i++;
			}
		}
		penPoint.keyState=Up;		
	}while(i<4);

	if(IsOk(tempVal)==0)goto AG;

	penConfig.xfac=(float)(340-20)/(tempVal[1][0]-tempVal[0][0]);
	penConfig.yfac=(float)(220-20)/(tempVal[3][1]-tempVal[0][1]);
	penConfig.xoff=20-penConfig.xfac*tempVal[0][0];
	penConfig.yoff=20-penConfig.yfac*tempVal[0][1];
	
	LCD_Clear(White);//清屏
	LCD_DisplayString(24,110,"屏幕校准成功");
	save_adjdata();
	DelayMS(1000);
	LCD_Clear(White);//清屏	 	     
}
void Touch_Init()
{
	SPI_TouchConfiguration();
	EXTI_Configuration();
	NVIC_TouchConfiguration();
#ifndef ADJUST_TOUCH
	touch_adjust();
#else	 
	if(get_adjdata())return;//已经校准
	else touch_adjust();  	//屏幕校准,带自动保存
#endif	  
}

u8 IsInRect(u16 startx,u16 starty,u16 sizex,u16 sizey)
{
	sizex+=startx;
	sizey+=starty;
	if(penPoint.x<startx||penPoint.x>sizex||penPoint.y<starty||penPoint.y>sizey)return 0;
	else return 1;
}


