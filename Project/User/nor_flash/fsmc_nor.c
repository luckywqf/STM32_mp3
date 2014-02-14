
/* Includes ------------------------------------------------------------------*/
#include "fsmc_nor.h"

/* Private define ------------------------------------------------------------*/
#define NOR_FLASH_START_ADDR       ((uint32_t)0x64000000)
#define NOR_FLASH_END_ADDR         ((uint32_t)0x64FFFFFF)

/* Delay definition */   
#define BlockErase_Timeout         ((uint32_t)0x00A00000)
#define ChipErase_Timeout          ((uint32_t)0x30000000) 
#define Program_Timeout            ((uint32_t)0x00001400)

#define ADDR_SHIFT(A)              (NOR_FLASH_START_ADDR + (2 * (A)))
#define NOR_WRITE(Address, Data)   (*(vu16 *)(Address) = (Data))

#define BUFFER_SIZE                0x400
#define WRITE_READ_ADDR            0x8000

/* Private variables ---------------------------------------------------------*/
static uint16_t TxBuffer[BUFFER_SIZE];
static uint16_t RxBuffer[BUFFER_SIZE];

/*******************************************************************************
* Function Name  : FSMC_NOR_Init
* Description    : Configures the FSMC and GPIOs to interface with the NOR memory.
*                  This function must be called before any write/read operation
*                  on the NOR.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void FSMC_NOR_Init(void)
{
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  p;
  GPIO_InitTypeDef GPIO_InitStructure;

  /*使能FSMC总线时钟*/
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
    
  /*使能FSMC总线使用的GPIO组时钟*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | 
                         RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO , ENABLE);

  /*FSMC数据线FSMC_D[0:15]初始化，推挽复用输出*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 |
                                GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                                GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 |
                                GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /*FSMC地址线FSMC_A[0:17]初始化，推挽复用输出*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
                                GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 |
                                GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOF, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 |
                                GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;                            
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /*FSMC NOE和NWE初试化，推挽复用输出*/  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /*FSMC NE2初试化，推挽复用输出*/  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  /*--------------FSMC 总线 存储器参数配置------------------------------*/
  p.FSMC_AddressSetupTime = 0x02;   //地址建立时间    
  p.FSMC_AddressHoldTime = 0x00;    //地址保持时间  
  p.FSMC_DataSetupTime = 0x05;      //数据建立时间
  p.FSMC_BusTurnAroundDuration = 0; //总线恢复时间x00
  p.FSMC_CLKDivision = 0x00;        // 时钟分频因子 
  p.FSMC_DataLatency = 0x00;        //数据产生时间
  p.FSMC_AccessMode = FSMC_AccessMode_B; //FSMC NOR控制器时序

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM2;//使用了FSMC的BANK1的子板块2
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;//禁止地址数据线复用
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;//存储器类型为Nor
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//存储器数据宽度为16位
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;//关闭突发模式访问
  //等待信号优先级，只有在使能突发访问模式才有效
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  //关闭Wrapped burst access mode，只有在使能突发访问模式才有效
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  //等待信号设置，只有在使能突发访问模式才有效
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  //使能这个BANK的写操作
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  //使能/关闭等待信息设置，只在使能突发访问模式才有效
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;//关闭Extend Mode
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;//关闭Write Burst Mode 
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;//读操作时序参数
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;//写操作时序参数

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

  /*--------------使能BANK1的子板块2------------------------------*/
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM2, ENABLE);
}

/******************************************************************************
* Function Name  : FSMC_NOR_ReadID
* Description    : Reads NOR memory's Manufacturer and Device Code.
* Input          : - NOR_ID: pointer to a NOR_IDTypeDef structure which will hold
*                    the Manufacturer and Device Code.
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void FSMC_NOR_ReadID(NOR_IDTypeDef* NOR_ID)
{
  NOR_WRITE(ADDR_SHIFT(0x0555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x0555), 0x0090);

  NOR_ID->Manufacturer_Code = *(vu16 *) ADDR_SHIFT(0x0000);
  NOR_ID->Device_Code1 = *(vu16 *) ADDR_SHIFT(0x0001);
  NOR_ID->Device_Code2 = *(vu16 *) ADDR_SHIFT(0x000E);
  NOR_ID->Device_Code3 = *(vu16 *) ADDR_SHIFT(0x000F);
}

/*******************************************************************************
* Function Name  : FSMC_NOR_EraseBlock
* Description    : Erases the specified Nor memory block.
* Input          : - BlockAddr: address of the block to erase.
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
* Attention		 : None
*******************************************************************************/
NOR_Status FSMC_NOR_EraseBlock(uint32_t BlockAddr)
{
  NOR_WRITE(ADDR_SHIFT(0x0555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x0555), 0x0080);
  NOR_WRITE(ADDR_SHIFT(0x0555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AA), 0x0055);
  NOR_WRITE((NOR_FLASH_START_ADDR + BlockAddr), 0x30);

  return (FSMC_NOR_GetStatus(BlockErase_Timeout));
}

/*******************************************************************************
* Function Name  : FSMC_NOR_EraseChip
* Description    : Erases the entire chip.
* Input          : None                      
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
* Attention		 : None
*******************************************************************************/
NOR_Status FSMC_NOR_EraseChip(void)
{
  NOR_WRITE(ADDR_SHIFT(0x0555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x0555), 0x0080);
  NOR_WRITE(ADDR_SHIFT(0x0555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x0555), 0x0010);

  return (FSMC_NOR_GetStatus(ChipErase_Timeout));
}

/******************************************************************************
* Function Name  : FSMC_NOR_WriteHalfWord
* Description    : Writes a half-word to the NOR memory. 
* Input          : - WriteAddr : NOR memory internal address to write to.
*                  - Data : Data to write. 
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
* Attention		 : None
*******************************************************************************/
NOR_Status FSMC_NOR_WriteHalfWord(uint32_t WriteAddr, uint16_t Data)
{
  NOR_WRITE(ADDR_SHIFT(0x0555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x0555), 0x00A0);
  NOR_WRITE((NOR_FLASH_START_ADDR + WriteAddr), Data);

  return (FSMC_NOR_GetStatus(Program_Timeout));
}

/*******************************************************************************
* Function Name  : FSMC_NOR_WriteBuffer
* Description    : Writes a half-word buffer to the FSMC NOR memory. 
* Input          : - pBuffer : pointer to buffer. 
*                  - WriteAddr : NOR memory internal address from which the data 
*                    will be written.
*                  - NumHalfwordToWrite : number of Half words to write. 
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
* Attention		 : None
*******************************************************************************/
NOR_Status FSMC_NOR_WriteBuffer(uint16_t* pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite)
{
  NOR_Status status = NOR_ONGOING; 

  do
  {
    /* Transfer data to the memory */
    status = FSMC_NOR_WriteHalfWord(WriteAddr, *pBuffer++);
    WriteAddr = WriteAddr + 2;
    NumHalfwordToWrite--;
  }
  while((status == NOR_SUCCESS) && (NumHalfwordToWrite != 0));
  
  return (status); 
}

/*******************************************************************************
* Function Name  : FSMC_NOR_ProgramBuffer
* Description    : Writes a half-word buffer to the FSMC NOR memory. This function 
*                  must be used only with S29GL128P NOR memory.
* Input          : - pBuffer : pointer to buffer. 
*                  - WriteAddr: NOR memory internal address from which the data 
*                    will be written.
*                  - NumHalfwordToWrite: number of Half words to write.
*                    The maximum allowed value is 32 Half words (64 bytes).
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
* Attention		 : None
*******************************************************************************/
NOR_Status FSMC_NOR_ProgramBuffer(uint16_t* pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite)
{       
  uint32_t lastloadedaddress = 0x00;
  uint32_t currentaddress = 0x00;
  uint32_t endaddress = 0x00;

  /* Initialize variables */
  currentaddress = WriteAddr;
  endaddress = WriteAddr + NumHalfwordToWrite - 1;
  lastloadedaddress = WriteAddr;

  /* Issue unlock command sequence */
  NOR_WRITE(ADDR_SHIFT(0x00555), 0x00AA);

  NOR_WRITE(ADDR_SHIFT(0x02AA), 0x0055);  

  /* Write Write Buffer Load Command */
  NOR_WRITE(ADDR_SHIFT(WriteAddr), 0x0025);
  NOR_WRITE(ADDR_SHIFT(WriteAddr), (NumHalfwordToWrite - 1));

  /* Load Data into NOR Buffer */
  while(currentaddress <= endaddress)
  {
    /* Store last loaded address & data value (for polling) */
    lastloadedaddress = currentaddress;
 
    NOR_WRITE(ADDR_SHIFT(currentaddress), *pBuffer++);
    currentaddress += 1; 
  }

  NOR_WRITE(ADDR_SHIFT(lastloadedaddress), 0x29);
  
  return(FSMC_NOR_GetStatus(Program_Timeout));
}

/******************************************************************************
* Function Name  : FSMC_NOR_ReadHalfWord
* Description    : Reads a half-word from the NOR memory. 
* Input          : - ReadAddr : NOR memory internal address to read from.
* Output         : None
* Return         : Half-word read from the NOR memory
* Attention		 : None
*******************************************************************************/
uint16_t FSMC_NOR_ReadHalfWord(uint32_t ReadAddr)
{
  NOR_WRITE(ADDR_SHIFT(0x00555), 0x00AA); 
  NOR_WRITE(ADDR_SHIFT(0x002AA), 0x0055);  
  NOR_WRITE((NOR_FLASH_START_ADDR + ReadAddr), 0x00F0 );

  return (*(vu16 *)((NOR_FLASH_START_ADDR + ReadAddr)));
}

/*******************************************************************************
* Function Name  : FSMC_NOR_ReadBuffer
* Description    : Reads a block of data from the FSMC NOR memory.
* Input          : - pBuffer : pointer to the buffer that receives the data read 
*                    from the NOR memory.
*                  - ReadAddr : NOR memory internal address to read from.
*                  - NumHalfwordToRead : number of Half word to read.
* Output         : None
*******************************************************************************/
void FSMC_NOR_ReadBuffer(uint16_t* pBuffer, uint32_t ReadAddr, uint32_t NumHalfwordToRead)
{
  NOR_WRITE(ADDR_SHIFT(0x0555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AA), 0x0055);
  NOR_WRITE((NOR_FLASH_START_ADDR + ReadAddr), 0x00F0);

  for(; NumHalfwordToRead != 0x00; NumHalfwordToRead--) /* while there is data to read */
  {
    /* Read a Halfword from the NOR */
    *pBuffer++ = *(vu16 *)((NOR_FLASH_START_ADDR + ReadAddr));
    ReadAddr = ReadAddr + 2; 
  }  
}

/******************************************************************************
* Function Name  : FSMC_NOR_ReturnToReadMode
* Description    : Returns the NOR memory to Read mode.
* Input          : None
* Output         : None
* Return         : NOR_SUCCESS
* Attention		 : None
*******************************************************************************/
NOR_Status FSMC_NOR_ReturnToReadMode(void)
{
  NOR_WRITE(NOR_FLASH_START_ADDR, 0x00F0);

  return (NOR_SUCCESS);
}

/******************************************************************************
* Function Name  : FSMC_NOR_Reset
* Description    : Returns the NOR memory to Read mode and resets the errors in
*                  the NOR memory Status Register.
* Input          : None
* Output         : None
* Return         : NOR_SUCCESS
* Attention		 : None
*******************************************************************************/
NOR_Status FSMC_NOR_Reset(void)
{
  NOR_WRITE(ADDR_SHIFT(0x00555), 0x00AA); 
  NOR_WRITE(ADDR_SHIFT(0x002AA), 0x0055); 
  NOR_WRITE(NOR_FLASH_START_ADDR, 0x00F0); 

  return (NOR_SUCCESS);
}

/******************************************************************************
* Function Name  : FSMC_NOR_GetStatus
* Description    : Returns the NOR operation status.
* Input          : - Timeout: NOR progamming Timeout
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
* Attention		 : None
*******************************************************************************/
NOR_Status FSMC_NOR_GetStatus(uint32_t Timeout)
{ 
  uint16_t val1 = 0x00, val2 = 0x00;
  NOR_Status status = NOR_ONGOING; 
  uint32_t timeout = Timeout;

  /* Poll on NOR memory Ready/Busy signal ------------------------------------*/
  while((GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) != RESET) && (timeout > 0)) 
  {
    timeout--;
  }

  timeout = Timeout;
  
  while((GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == RESET) && (timeout > 0))   
  {
    timeout--;
  }
  
  /* Get the NOR memory operation status -------------------------------------*/
  while((Timeout != 0x00) && (status != NOR_SUCCESS))
  {
    Timeout--;

	  /* Read DQ6 and DQ5 */
    val1 = *(vu16 *)(NOR_FLASH_START_ADDR);
    val2 = *(vu16 *)(NOR_FLASH_START_ADDR);

    /* If DQ6 did not toggle between the two reads then return NOR_Success */
    if((val1 & 0x0040) == (val2 & 0x0040)) 
    {
      return NOR_SUCCESS;
    }

    if((val1 & 0x0020) != 0x0020)
    {
      status = NOR_ONGOING;
    }

    val1 = *(vu16 *)(NOR_FLASH_START_ADDR);
    val2 = *(vu16 *)(NOR_FLASH_START_ADDR);
    
    if((val1 & 0x0040) == (val2 & 0x0040)) 
    {
      return NOR_SUCCESS;
    }
    else if((val1 & 0x0020) == 0x0020)
    {
      return NOR_ERROR;
    }
  }

  if(Timeout == 0x00)
  {
    status = NOR_TIMEOUT;
  } 

  /* Return the operation status */
  return (status);
}

/******************************************************************************
* Function Name  : FSMC_NOR_Test
* Description    : NOR Test
*******************************************************************************/
void FSMC_NOR_Test(void)
{ 
  uint16_t index;
  NOR_IDTypeDef NOR_ID;

  FSMC_NOR_Init();				  /* Initialize the FSMC NOR Flash Interface */
  
  FSMC_NOR_ReturnToReadMode();	 /* Set the NOR read modes */

  FSMC_NOR_ReadID(&NOR_ID);

  FSMC_NOR_ReturnToReadMode();	/* 读完ID后，必须返回读模式 */

  /* Erase the NOR memory block to write on */
  FSMC_NOR_EraseBlock(WRITE_READ_ADDR);

  /* Write data to FSMC NOR memory */
  /* Fill the buffer to send */
  for (index = 0; index < BUFFER_SIZE; index++ )
  {
    TxBuffer[index] = index + 0x3210;
  }

  FSMC_NOR_WriteBuffer(TxBuffer, WRITE_READ_ADDR, BUFFER_SIZE);

  /* Read data from FSMC NOR memory */
  FSMC_NOR_ReadBuffer(RxBuffer, WRITE_READ_ADDR, BUFFER_SIZE);  

}

