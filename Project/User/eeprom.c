#include "eeprom.h"

void I2C_delay(void)		  //软件延时（非精确）
{	
   uint8_t i=50;     
   while(i) 
   { 
     i--; 
   } 
}

u8 I2C_Start(void)	 //I2C开始位
{
	SDA_H;
	SCL_H;
	I2C_delay();
	if(!SDA_read)
	  return 0;	                    //SDA线为低电平则总线忙,退出
	SDA_L;
	I2C_delay();
	if(SDA_read) 
	  return 0;	                    //SDA线为高电平则总线出错,退出
	SDA_L;							//SCL为高电平时，SDA的下降沿表示停止位
	I2C_delay();
	return 1;
}

void I2C_Stop(void)			   //I2C停止位
{
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SDA_H;					   //SCL为高电平时，SDA的上升沿表示停止位
	I2C_delay();
}

static void I2C_Ack(void)		//I2C响应位
{	
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}

static void I2C_NoAck(void)		//I2C非响应位
{	
	SCL_L;
	I2C_delay();
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}

u8 I2C_WaitAck(void) 	  //I2C等待应答位
{
	SCL_L;
	I2C_delay();
	SDA_H;			
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(SDA_read)
	{
      SCL_L;
      return 0;
	}
	SCL_L;
	return 1;
}

 /*******************************************************************************
* Function Name  : I2C_SendByte
* Description    : 数据从高位到低位
* Input          : - SendByte: 发送的数据
* Output         : None
* Return         : None
*******************************************************************************/
void I2C_SendByte(u8 SendByte) 
{
    u8 i;
    for(i = 0;i < 8; i++) 
    {
      SCL_L;
      I2C_delay();
      if(SendByte & 0x80)
        SDA_H;  			  //在SCL为低电平时，允许SDA数据改变
      else 
        SDA_L;   
      SendByte <<= 1;
      I2C_delay();
      SCL_H;
      I2C_delay();
    }
    SCL_L;
}

/*******************************************************************************
* Function Name  : I2C_ReceiveByte
* Description    : 数据从高位到低位
* Return         : I2C总线返回的数据
*******************************************************************************/
u8 I2C_ReceiveByte(void)  
{ 
    u8 i,ReceiveByte = 0;

    SDA_H;				
    for(i = 0;i < 8; i++) 
    {
      ReceiveByte <<= 1;      
      SCL_L;
      I2C_delay();
	  SCL_H;
      I2C_delay();	
      if(SDA_read)				   //在SCL为高电平时，SDA上的数据保持不变，可以读回来
      {
        ReceiveByte |= 0x01;
      }
    }
    SCL_L;
    return ReceiveByte;
}

/*******************************************************************************
* Function Name  : I2C_WriteByte
* Description    : 写一字节数据
* Input          : - SendByte: 待写入数据
*           	   - WriteAddress: 待写入地址
*                  - DeviceAddress: 器件类型
* Output         : None
* Return         : 返回为:=1成功写入,=0失败
*******************************************************************************/           
u8 I2C_WriteByte(u8 SendByte, u16 WriteAddress, u8 DeviceAddress)
{		
    if(!I2C_Start()) return 0;

    I2C_SendByte(((WriteAddress & 0x0700) >>7) | DeviceAddress & 0xFFFE); //设置高起始地址+器件地址

    if(!I2C_WaitAck())
	{
	  I2C_Stop(); 
	  return 0;
	}

    I2C_SendByte((u8)(WriteAddress & 0x00FF));   //设置低起始地址
    I2C_WaitAck();	
    I2C_SendByte(SendByte);
    I2C_WaitAck();   
    I2C_Stop(); 
    DelayMS(10); 		  //注意：因为这里要等待EEPROM写完，可以采用查询或延时方式(10ms)
    return 1;
}									 

/*******************************************************************************
* Function Name  : I2C_ReadByte
* Description    : 读取一串数据
* Input          : - pBuffer: 存放读出数据
*           	   - length: 待读出长度
*                  - ReadAddress: 待读出地址
*                  - DeviceAddress: 器件类型
* Output         : None
* Return         : 返回为:=1成功读入,=0失败
*******************************************************************************/          
u8 I2C_ReadByte(u8* pBuffer,   u16 length,   u16 ReadAddress,  u8 DeviceAddress)
{		
    if(!I2C_Start()) return 0;

    I2C_SendByte(((ReadAddress & 0x0700) >> 7) | DeviceAddress & 0xFFFE); //设置高起始地址+器件地址

    if(!I2C_WaitAck())
	{
	  I2C_Stop(); 
	  return 0;
	}

    I2C_SendByte((u8)(ReadAddress & 0x00FF));   //设置低起始地址   
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(((ReadAddress & 0x0700) >>7) | DeviceAddress | 0x0001);
    I2C_WaitAck();

    while(length)
    {
      *pBuffer = I2C_ReceiveByte();
      if(length == 1)
	    I2C_NoAck();
      else 
	    I2C_Ack(); 
      pBuffer++;
      length--;
    }

    I2C_Stop();

    return 1;
}
/*************************************************
函数: void Eeprom_Init(void)
功能: EEPROM管脚配置
**************************************************/
void Eeprom_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure; 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //允许GPIOB时钟
	
	/* Configure I2C2 pins: PB6->SCL and PB7->SDA */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  		 //开漏输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
