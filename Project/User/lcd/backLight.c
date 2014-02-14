#include "stm32f10x.h"
#include "BackLight.h"
#include "eeprom.h"

uint8_t autosetBackLight;
uint8_t backLightValue;
/*******************************************************************************
	函数名: InitBackLight
	输  入: _bright 亮度，0是灭，255是最亮
	输  出:
	功能说明：初始化GPIO,配置为PWM模式

	背光口线是 PB1, 复用功能选择 TIM3_CH4

	(1)当关闭背光时，
		将CPU IO设置为浮动输入模式（推荐设置为推挽输出，并驱动到低电平)
		将TIM3关闭 省电

	(2)
*/
void SetBackLight(uint8_t _bright)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	/* 第1步：打开GPIOB RCC_APB2Periph_AFIO 的时钟	*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	if (_bright == 0)
	{
		/* 配置背光GPIO为输入模式 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		/* 关闭TIM3 */
		TIM_Cmd(TIM3, DISABLE);
		return;
	}
	else if (_bright == BRIGHT_MAX)	/* 最大亮度 */
	{
		/* 配置背光GPIO为推挽输出模式 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		GPIO_SetBits(GPIOB, GPIO_Pin_1);

		/* 关闭TIM3 */
		TIM_Cmd(TIM3, DISABLE);
		return;
	}

	/* 配置背光GPIO为复用推挽输出模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* 使能TIM3的时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	/* -----------------------------------------------------------------------

	TIM3 ARR Register = 1894 => TIM3 Frequency = TIM3 counter clock/(ARR + 1)
	TIM3 Frequency = 37.99 KHz (接近38KHz 红外发射管的调制基频)
	TIM3 Channel1 duty cycle = (TIM3_CCR1/ TIM3_ARR)* 100 = 50%
	----------------------------------------------------------------------- */

	/*
		TIM3 配置: 产生1路PWM信号;
		TIM3CLK = 72 MHz, Prescaler = 0(不分频), TIM3 counter clock = 72 MHz
		计算公式：
		PWM输出频率 = TIM3 counter clock /(ARR + 1)

		我们期望设置为100Hz

		如果不对TIM3CLK预分频，那么不可能得到100Hz低频。
		我们设置分频比 = 1000， 那么  TIM3 counter clock = 72KHz
		TIM_Period = 720 - 1;
		频率下不来。
	 */
	TIM_TimeBaseStructure.TIM_Period = 720 - 1;	/* TIM_Period = TIM3 ARR Register */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	/*
		_bright = 1 时, TIM_Pulse = 1
		_bright = 255 时, TIM_Pulse = TIM_Period
	*/
	TIM_OCInitStructure.TIM_Pulse = (TIM_TimeBaseStructure.TIM_Period * _bright) / BRIGHT_MAX;	/* 改变占空比 */

	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM3, ENABLE);

	/* TIM3 enable counter */
	TIM_Cmd(TIM3, ENABLE);
}
/*******************************************************************************
	函数名：ADC_Configuration
	功能说明：配置ADC, PC1 ,光敏电阻
*/
void ADC_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

    /* 使能 ADC1 and GPIOC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);              //72M/6=12,ADC最大时间不能超过14M

	/* 配置PC1为模拟输入(ADC Channel11) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* 配置ADC1, 不用DMA, 用软件自己触发 */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		 //ADC1工作模式:独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;			 //单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		 //单次转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	  //转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	 //ADC1数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	   //顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);		   //根据ADC_InitStruct中指定的参数，初始化外设ADC1的寄存器

	/* ADC1 regular channel11 configuration */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_239Cycles5);	//ADC1,ADC通道11,规则采样顺序值为1,采样时间为239.5周期

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);		  //使能ADC1

	/* Enable ADC1 reset calibaration register */
	ADC_ResetCalibration(ADC1);						//重置ADC1的校准寄存器
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));		//获取ADC1重置校准寄存器的状态,设置状态则等待

	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);					//开始ADC1的校准状态
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));		//等待校准完成

	/* Start ADC1 Software Conversion */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能ADC1的软件转换启动功能
}
uint16_t GetLightValue(void)
{
	uint16_t value;
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	        //软件启动ADC转换
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC )); //等待转换结束
	value = ADC_GetConversionValue(ADC1);		//读取ADC值
	return value;
}

//保存到FM24C16的 30~31这段地址区间，总共2个字节,
//第一个字节记录是否自动设置，第二个记录背光值
void SaveBackLight(void)
{
	I2C_WriteByte(autosetBackLight, 30, ADDR_24LC08);
	I2C_WriteByte(backLightValue, 31, ADDR_24LC08);
}
//获取背光值，若没有，则返回默认值200
u8 GetBackLight(void)
{
	u8 ReadBuffer[2];
	I2C_ReadByte(ReadBuffer,2,30, ADDR_24LC08);
	if(ReadBuffer[1]==0)return 200;
	else return 	ReadBuffer[1];	 
}
//获取是否自动设置
u8 GetAutoSet(void)
{
	u8 ReadBuffer[2];
	I2C_ReadByte(ReadBuffer,2,30, ADDR_24LC08);
	return 	ReadBuffer[0];	 
}

void BackLight_Init(void)
{
	uint16_t value;
	ADC_Configuration();

	autosetBackLight=GetAutoSet();
	if(autosetBackLight==1)
	{
		value=GetLightValue();
		backLightValue=255-(value>>7);
	}
	else  backLightValue=GetBackLight();
	SetBackLight(backLightValue);
}
