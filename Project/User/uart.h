#ifndef _UART_H
#define _UART_H
#include <stdio.h>
/*******************************************************************************
	函数名：USART_Configuration
	功能说明：
	初始化串口硬件设备，启用中断
	配置步骤：
	(1)打开GPIO和USART的时钟
	(2)设置USART两个管脚GPIO模式
	(3)配置USART数据格式、波特率等参数
	(4)使能USART接收中断功能
	(5)最后使能USART功能
*/
void Uart_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	/* 第1步：打开GPIO和USART部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* 第3步：将USART Rx的GPIO配置为浮空输入模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 第4步：配置USART参数
	    - BaudRate = 9600 baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - No parity
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

    /* 若接收数据寄存器满，则产生中断 */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	/* 第5步：使能 USART， 配置完毕 */
	USART_Cmd(USART1, ENABLE);

    /* 如下语句解决第1个字节无法正确发送出去的问题 */
    USART_ClearFlag(USART1, USART_FLAG_TC);     // 清标志
}
/*******************************************************************************
* Function Name  : fputc
* Description    : Retargets the C library printf function to the USART.

*******************************************************************************/
int fputc(int ch, FILE *f)    //在源代码中添加fputc函数，可以用printf函数做为调试输出
{
  /* 将Printf内容发往串口 */
  USART_SendData(USART1, (u8) ch);  
  /* 循环，直到传送完成 */
  while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {
  }
  return ch;
}
/*******************************************************************************
	函数名：fputc
	重定义getc函数，这样可以使用scanff函数从串口1输入数据
********************************************************************************/
int fgetc(FILE *f)
{
	/* 等待串口1输入数据 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
	return (int)USART_ReceiveData(USART1);
}
#endif
