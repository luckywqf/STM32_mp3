#include "vs1003.h"
#include "delay.h"
		
/*******************************************************************************
* Function Name  : VS1003_SPI_Init
* Description    : VS1003 SPI Init
*******************************************************************************/
static void VS1003_SPI_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;

  RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE);
  /* SPI2 configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI2, &SPI_InitStructure);

  /* Enable SPI2  */
  SPI_Cmd(SPI2, ENABLE);
}

/*******************************************************************************
* Function Name  : VS1003_SPI_SetSpeed
* Description    : None
* Input          : - SpeedSet: SPI_SPEED_HIGH 1  SPI_SPEED_LOW 0
*******************************************************************************/
static void VS1003_SPI_SetSpeed( uint8_t SpeedSet)
{
  SPI_InitTypeDef SPI_InitStructure ;
    
  SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex ;
  SPI_InitStructure.SPI_Mode=SPI_Mode_Master ;
  SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b ;
  SPI_InitStructure.SPI_CPOL=SPI_CPOL_High ;
  SPI_InitStructure.SPI_CPHA=SPI_CPHA_2Edge ;
  SPI_InitStructure.SPI_NSS=SPI_NSS_Soft ;

  if( SpeedSet == SPI_SPEED_LOW )
  {
      SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_128;
  }
  else 
  {
      SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_8;
  }
  SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB ;
  SPI_InitStructure.SPI_CRCPolynomial=7 ;
  SPI_Init(SPI2,&SPI_InitStructure);
}

/*******************************************************************************
* Function Name  : VS1003_SPI_ReadWriteByte
* Description    : None
* Input          : - TxData: Write Byte
*******************************************************************************/
static uint8_t VS1003_SPI_ReadWriteByte( uint8_t TxData )
{
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

  /* Send byte through the SPI2 peripheral */
  SPI_I2S_SendData(SPI2, TxData);

  /* Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI2);
}

/*******************************************************************************
* Function Name  : VS1003_Init
* Description    : VS1003端口初始化
***SPI2接口****
XCS---PB12
SCK---PB13
MISO--PB14
MSIO--PB15
XDCS--PA4
Reset-PB10
DREQ--PB11
*******************************************************************************/	 
void VS1003_Init(void)
{		  
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO , ENABLE);
  /* Configure SPI2 pins: SCK, MISO and MOSI ---------------------------------*/ 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13  | GPIO_Pin_14 | GPIO_Pin_15; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
  /* XCS */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  /* XDCS */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* DREQ */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 

  /* Reset */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 

  MP3_Reset(0);
  DelayMS(1);
  MP3_Reset(1);

  MP3_DCS(1); 
  MP3_CCS(1); 

  VS1003_SPI_Init();
}


/*******************************************************************************
* Function Name  : VS1003_WriteReg
* Description    : VS1003写寄存器
* Input          : - reg: 命令地址
*				   - value: 命令数据
*******************************************************************************/
static void VS1003_WriteReg( uint8_t reg,uint16_t value )
{  
   while(  MP3_DREQ ==0 );           /* 等待空闲 */
   
   VS1003_SPI_SetSpeed( SPI_SPEED_LOW );	 
   MP3_DCS(1); 
   MP3_CCS(0); 
   VS1003_SPI_ReadWriteByte(VS_WRITE_COMMAND); /* 发送VS1003的写命令 */
   VS1003_SPI_ReadWriteByte(reg);             
   VS1003_SPI_ReadWriteByte(value>>8);        
   VS1003_SPI_ReadWriteByte(value);	          
   MP3_CCS(1);         
   VS1003_SPI_SetSpeed( SPI_SPEED_HIGH );
} 

/*******************************************************************************
* Function Name  : VS1003_ReadReg
* Description    : VS1003读寄存器
* Input          : - reg: 命令地址
* Output         : None
* Return         : - value: 数据
* Attention		 : None
*******************************************************************************/
uint16_t VS1003_ReadReg( uint8_t reg)
{ 
   uint16_t value;
    
   while(  MP3_DREQ ==0 );           /* 等待空闲 */
   VS1003_SPI_SetSpeed( SPI_SPEED_LOW );
   MP3_DCS(1);     
   MP3_CCS(0);     
   VS1003_SPI_ReadWriteByte(VS_READ_COMMAND);/* 发送VS1003的读命令 */
   VS1003_SPI_ReadWriteByte( reg );   
   value = VS1003_SPI_ReadWriteByte(0xff);		
   value = value << 8;
   value |= VS1003_SPI_ReadWriteByte(0xff); 
   MP3_CCS(1);   
   VS1003_SPI_SetSpeed( SPI_SPEED_HIGH );
   return value; 
} 

/*******************************************************************************
* Function Name  : VS1003_ResetDecodeTime
*******************************************************************************/                       
void VS1003_ResetDecodeTime(void)
{
   VS1003_WriteReg(SPI_DECODE_TIME,0x0000);
   VS1003_WriteReg(SPI_DECODE_TIME,0x0000); /* 操作两次 */
}

/*******************************************************************************
* Function Name  : VS1003_GetDecodeTime
*******************************************************************************/    
uint16_t VS1003_GetDecodeTime(void)
{ 
   return VS1003_ReadReg(SPI_DECODE_TIME);   
} 
//FOR WAV HEAD0 :0X7761 HEAD1:0X7665    
//FOR MIDI HEAD0 :other info HEAD1:0X4D54
//FOR WMA HEAD0 :data speed HEAD1:0X574D
//FOR MP3 HEAD0 :data speed HEAD1:ID
//比特率预定值
const u16 bitrate[2][16]=
{ 
{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}, 
{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
};
//返回Kbps的大小
//得到mp3&wma的波特率
u16 VS1003_GetHeadInfo(void)
{
	unsigned int HEAD0;
	unsigned int HEAD1;            
    HEAD0=VS1003_ReadReg(SPI_HDAT0); 
    HEAD1=VS1003_ReadReg(SPI_HDAT1);
	if(HEAD0==0)return 0;
    switch(HEAD1)
    {        
        case 0x7665:return 0;//WAV格式
        case 0X4D54:return 1;//MIDI格式 
        case 0X574D://WMA格式
        {
            HEAD1=HEAD0*2/25;
            if((HEAD1%10)>5)return HEAD1/10+1;
            else return HEAD1/10;
        }
        default://MP3格式
        {
            HEAD1>>=3;
            HEAD1=HEAD1&0x03; 
            if(HEAD1==3)HEAD1=1;
            else HEAD1=0;
            return bitrate[HEAD1][HEAD0>>12];
        }
    } 
}
/*******************************************************************************
* Function Name  : VS1003_SoftReset
* Description    : VS1003软复位
*******************************************************************************/
void VS1003_SoftReset(void)
{
   uint8_t retry; 	
   			   
   while(  MP3_DREQ ==0 );   /* 等待软件复位结束 */
   VS1003_SPI_ReadWriteByte(0xff);  /* 启动传输 */
   retry = 0;
   while( VS1003_ReadReg(0) != 0x0804 )   /* 软件复位,新模式 */  
   {
	   VS1003_WriteReg(SPI_MODE,0x0804);  /* 软件复位,新模式 */
	   DelayMS(2);                        /* 等待至少1.35ms */
	   if( retry++ > 100 )
	   { 
	      break; 
	   }
   }	 	

   while(  MP3_DREQ ==0 );   /* 等待软件复位结束 */

   retry = 0;

   while( VS1003_ReadReg(SPI_CLOCKF) != 0X9800 ) /* 设置vs1003的时钟,3倍频 ,1.5xADD */ 
   {
	   VS1003_WriteReg(SPI_CLOCKF,0X9800);       /* 设置vs1003的时钟,3倍频 ,1.5xADD */
	   if( retry++ > 100 )
	   { 
	      break; 
	   }
   }		   
   retry = 0;

   while( VS1003_ReadReg(SPI_AUDATA) != 0XBB81 ) /* 设置vs1003的时钟,3倍频 ,1.5xADD */
   {
	   VS1003_WriteReg(SPI_AUDATA,0XBB81);
	   if( retry++ > 100 )
	   { 
	      break; 
	   }   
   }

   VS1003_WriteReg(11,0x2020);	 /* 音量 */			 
   VS1003_ResetDecodeTime();     /* 复位解码时间 */	     
   /* 向vs1003发送4个字节无效数据，用以启动SPI发送 */	
   MP3_DCS(0);  /* 选中数据传输 */
   VS1003_SPI_ReadWriteByte(0XFF);
   VS1003_SPI_ReadWriteByte(0XFF);
   VS1003_SPI_ReadWriteByte(0XFF);
   VS1003_SPI_ReadWriteByte(0XFF);
   MP3_DCS(1);  /* 取消数据传输 */
   DelayMS(20);
} 

/*******************************************************************************
* Function Name  : VS1003_Reset
* Description    : VS1003复位
*******************************************************************************/
void VS1003_Reset(void)
{
   DelayMS(20);
   VS1003_SPI_ReadWriteByte(0XFF);
   MP3_DCS(1);              /* 取消数据传输 */
   MP3_CCS(1);              /* 取消数据传输 */

   while(  MP3_DREQ ==0 );  /* 等待DREQ为高 */
   DelayMS(20);			 
}

/*******************************************************************************
* Function Name  : VS1003_SineTest
* Description    : VS1003正弦测试
*******************************************************************************/
void VS1003_SineTest(void)
{	
   VS1003_WriteReg(0x0b,0X2020);	  /* 设置音量 */	 
   VS1003_WriteReg(SPI_MODE,0x0820); /* 进入vs1003的测试模式 */	    
   while(  MP3_DREQ ==0 );  /* 等待DREQ为高 */
 	
   /* 向vs1003发送正弦测试命令：0x53 0xef 0x6e n 0x00 0x00 0x00 0x00 */
   MP3_DCS(0);/* 选中数据传输 */
   VS1003_SPI_ReadWriteByte(0x53);
   VS1003_SPI_ReadWriteByte(0xef);
   VS1003_SPI_ReadWriteByte(0x6e);
   VS1003_SPI_ReadWriteByte(0x24);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   DelayMS(100);
   MP3_DCS(1); 
   /* 退出正弦测试 */
   MP3_DCS(0);/* 选中数据传输 */
   VS1003_SPI_ReadWriteByte(0x45);
   VS1003_SPI_ReadWriteByte(0x78);
   VS1003_SPI_ReadWriteByte(0x69);
   VS1003_SPI_ReadWriteByte(0x74);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   DelayMS(100);
   MP3_DCS(1);		 

   /* 再次进入正弦测试并设置n值为0x44，即将正弦波的频率设置为另外的值 */
   MP3_DCS(0);/* 选中数据传输 */      
   VS1003_SPI_ReadWriteByte(0x53);
   VS1003_SPI_ReadWriteByte(0xef);
   VS1003_SPI_ReadWriteByte(0x6e);
   VS1003_SPI_ReadWriteByte(0x44);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   DelayMS(100);
   MP3_DCS(1);
   /* 退出正弦测试 */
   MP3_DCS(0);/* 选中数据传输 */      
   VS1003_SPI_ReadWriteByte(0x45);
   VS1003_SPI_ReadWriteByte(0x78);
   VS1003_SPI_ReadWriteByte(0x69);
   VS1003_SPI_ReadWriteByte(0x74);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   DelayMS(100);
   MP3_DCS(1);	 
}	 

/*******************************************************************************
* Function Name  : VS1003_RamTest
* Description    : VS1003 RAM测试
*******************************************************************************/																			 
void VS1003_RamTest(void)
{
   volatile uint16_t value;
     
   VS1003_Reset();     
   VS1003_WriteReg(SPI_MODE,0x0820);/* 进入vs1003的测试模式 */
   while(  MP3_DREQ ==0 );           /* 等待DREQ为高 */
   MP3_DCS(0);	       			     /* xDCS = 1，选择vs1003的数据接口 */
   VS1003_SPI_ReadWriteByte(0x4d);
   VS1003_SPI_ReadWriteByte(0xea);
   VS1003_SPI_ReadWriteByte(0x6d);
   VS1003_SPI_ReadWriteByte(0x54);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   VS1003_SPI_ReadWriteByte(0x00);
   DelayMS(50);  
   MP3_DCS(1);
   value = VS1003_ReadReg(SPI_HDAT0); /* 如果得到的值为0x807F，则表明完好 */
}     
		 				
/*******************************************************************************
* Function Name  : VS1003_SetBass
* Description    : 设定vs1003播放的高低音	  默认值全0
*******************************************************************************/   
void VS1003_SetBass(u8 ST_AMPLITUDE,u8 ST_FREQLIMIT,u8 SB_AMPLITUDE,u8 SB_FREQLIMIT)
{
	uint16_t bass=0;  /* 暂存音调寄存器值 */
	
	bass+=ST_AMPLITUDE<<12+ST_FREQLIMIT<<8+SB_AMPLITUDE<<4+SB_FREQLIMIT;		     
	
	VS1003_WriteReg(SPI_BASS,bass);/* BASS */
	while(  MP3_DREQ ==0 );   	 //等待完成
}    
/*******************************************************************************
* Function Name  : VS1003_SetVol
* Description    : 设定vs1003播放的音量	范围0-100
*******************************************************************************/   
void VS1003_SetVol(u8 vol)
{
	u16 vset;	 
	vset=vol*5/2;
	vset=254-vset;
	vol=vset;
	vset<<=8;
	vset+=vol;

	VS1003_WriteReg(SPI_BASS,0);/* BASS */
	VS1003_WriteReg(SPI_VOL,vset); /* 设音量 */ 
	while(  MP3_DREQ ==0 );		 //等待完成
} 
/*******************************************************************************
* Function Name  : VS1003_WriteData
* Description    : VS1003写数据
*******************************************************************************/  
void VS1003_WriteData( uint8_t * buf)
{		
   uint8_t  count = 32;

   MP3_DCS(0);   
   while( count-- )
   {
	  VS1003_SPI_ReadWriteByte( *buf++ );
   }
   MP3_DCS(1);  
   MP3_CCS(1);  
} 

/*******************************************************************************
* Function Name  : VS1003_Record_Init
* Description    : VS1003录音初始化
* Attention		 : VS1003录音初始化成功之后可以听到耳机有回放
*                  此时不能用喇叭容易产生自激
*******************************************************************************/ 
void VS1003_Record_Init(void)
{
  uint8_t retry; 	

  /* 配置时钟 */
  while( VS1003_ReadReg(SPI_CLOCKF) != 0x9800 )   
  {
	  VS1003_WriteReg(SPI_CLOCKF,0x9800);   
	  DelayMS(2);                        /* 等待至少1.35ms */
	  if( retry++ > 100 )
	  { 
	      break; 
	  }
  }	

  while( VS1003_ReadReg(SPI_BASS) != 0x0000 )   
  {
	  VS1003_WriteReg(SPI_CLOCKF,0x0000);   
	  DelayMS(2);                        /* 等待至少1.35ms */
	  if( retry++ > 100 )
	  { 
	      break; 
	  }
  }	

  /* Set sample rate divider=12 */ 
  while( VS1003_ReadReg(SPI_AICTRL0) != 0x0012 )   
  {
	  VS1003_WriteReg(SPI_AICTRL0,0x0012);   
	  DelayMS(2);                        /* 等待至少1.35ms */
	  if( retry++ > 100 )
	  { 
	      break; 
	  }
  }	

  /* AutoGain OFF, reclevel 0x1000 */ 
  while( VS1003_ReadReg(SPI_AICTRL1) != 0x1000 )   
  {
	  VS1003_WriteReg(SPI_AICTRL1,0x1000);   
	  DelayMS(2);                        /* 等待至少1.35ms */
	  if( retry++ > 100 )
	  { 
	      break; 
	  }
  }	

  /* RECORD,NEWMODE,RESET */ 
  while( VS1003_ReadReg(SPI_MODE) != 0x1804 )   
  {
	  VS1003_WriteReg(SPI_MODE,0x1804);   
	  DelayMS(2);                        /* 等待至少1.35ms */
	  if( retry++ > 100 )
	  { 
	      break; 
	  }
  }	

  while( VS1003_ReadReg(SPI_CLOCKF) != 0x9800 )   
  {
	  VS1003_WriteReg(SPI_CLOCKF,0x9800);   
	  DelayMS(2);                        /* 等待至少1.35ms */
	  if( retry++ > 100 )
	  { 
	      break; 
	  }
  }	
}

