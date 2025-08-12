#include "stm32f10x.h"                  // Device header
#include "ADC.h"
#include "Delay.h"

u16 AD_Value[5];

/** @brief	ADC输入端口初始化，PA0,PA1,PA2,PA3,PB0
  **/
void ADC_GPIO_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);			//使能GPIOA
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);			//使能GPIOA
	
	//GPIO初始化
	GPIO_InitTypeDef	GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_AIN;								//模拟输入
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;//端口
	GPIO_InitStruct.GPIO_Speed	=GPIO_Speed_50MHz;						//速度
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_0;//端口
	GPIO_Init(GPIOB,&GPIO_InitStruct);
}

/** @brief	ADC1初始化
  **/
void ADC1_In_Init(void)
{
	ADC_GPIO_Init();																					//ADC输入端口初始化
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);				//使能ADC1
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);													//ADC时钟6分频，72m/6=12m
	
	//规则组输入通道选择，//ADC1,通道0，PA0,序列1，周期71.5
	ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_71Cycles5);
	//规则组输入通道选择，//ADC1,通道1，PA1,序列2，周期71.5
	ADC_RegularChannelConfig(ADC1,ADC_Channel_1,2,ADC_SampleTime_71Cycles5);
	//规则组输入通道选择，//ADC1,通道2，PA2,序列3，周期71.5
	ADC_RegularChannelConfig(ADC1,ADC_Channel_2,3,ADC_SampleTime_71Cycles5);
	//规则组输入通道选择，//ADC1,通道3，PA3,序列4，周期71.5
	ADC_RegularChannelConfig(ADC1,ADC_Channel_3,4,ADC_SampleTime_71Cycles5);
	//规则组输入通道选择，//ADC1,通道8，PB0,序列5，周期71.5
	ADC_RegularChannelConfig(ADC1,ADC_Channel_8,5,ADC_SampleTime_71Cycles5);
	
	//ADC初始化
	ADC_InitTypeDef ADC_InitStruct;
	ADC_InitStruct.ADC_Mode								=ADC_Mode_Independent;			//独立转换模式
	ADC_InitStruct.ADC_ExternalTrigConv		=ADC_ExternalTrigConv_None;	//无外部触发
	ADC_InitStruct.ADC_DataAlign					=ADC_DataAlign_Right;				//数据右对齐
	ADC_InitStruct.ADC_ContinuousConvMode	=ENABLE;										//连续转换
	ADC_InitStruct.ADC_ScanConvMode				=ENABLE;										//扫描模式
	ADC_InitStruct.ADC_NbrOfChannel				=5;													//5通道
	ADC_Init(ADC1,&ADC_InitStruct);
	
	ADC_Cmd(ADC1,ENABLE);			//开启ADC1
}

/** @brief	ADC1中断初始化	
  * @param	PreemptionPriority		抢占优先级
  * @param	SubPriority						响应优先级
  **/
void ADC1_Interrupt_Init(u8 PreemptionPriority,u8 SubPriority)
{
	ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);																		//开启ADC1中断
	
	//NVIC初始化结构体
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel										=	ADC1_2_IRQn;				//ADC1,2通道
	NVIC_InitStruct.NVIC_IRQChannelCmd								=	ENABLE;							//启用中断
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	=	PreemptionPriority;	//抢占优先级0
	NVIC_InitStruct.NVIC_IRQChannelSubPriority				=	SubPriority;				//响应优先级2 
	NVIC_Init(&NVIC_InitStruct);																						//初始化NVIC
}	

/** @brief	ADC1校准
	*	@retval	0:校准成功，1：校准失败
  **/
u8 ADC1_Calibrate(void)
{
	ADC_ResetCalibration(ADC1);													//复位ADC1
	for(u16 i=0;i<1000;i++)
	{	
		if(ADC_GetResetCalibrationStatus(ADC1)==RESET)		//等待ADC1复位完成，大约1us
		{	break;}																					//完成复位，跳出循环
		if(i==1000)
		{	return 1;}																			//1ms内，ADC1未完成复位，则AD1复位失败,返回0
	}
	ADC_StartCalibration(ADC1);													//校准ADC1
	for(u16 i=0;i<1000;i++)
	{	
		if(ADC_GetCalibrationStatus(ADC1)==RESET)					//等待ADC1校准完成，大约5us
		{	break;}																					//完成复位，跳出循环
		if(i==1000)
		{	return 1;}																			//1ms内，ADC1未完成校准，则AD1校准失败,返回0
	}
	
	return 0;																						//校准成功，返回0
}

/** @brief	DMA初始化	，DMA1通道1
  **/
void ADC_DMA1_1_Init(void)
{

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);												//使能DMA1
	
	//DMA初始化
	DMA_InitTypeDef DMA_InitStruct;
	//源数据设置
	DMA_InitStruct.DMA_PeripheralBaseAddr	=(u32)&ADC1->DR;									//数据地址为ADC1->DR
	DMA_InitStruct.DMA_PeripheralDataSize	=DMA_PeripheralDataSize_HalfWord;	//数据长度为short，16Bit
	DMA_InitStruct.DMA_PeripheralInc			=DMA_PeripheralInc_Disable;				//数据不自增
	//目标数据设置
	DMA_InitStruct.DMA_MemoryBaseAddr			=(u32)AD_Value;										//数据地址为数组ADC_Value
	DMA_InitStruct.DMA_MemoryDataSize			=DMA_MemoryDataSize_HalfWord;			//数据长度为short，16Bit
	DMA_InitStruct.DMA_MemoryInc					=DMA_MemoryInc_Enable;						//数据自增
	//DMA设置
	DMA_InitStruct.DMA_DIR								=DMA_DIR_PeripheralSRC;						//传输方向:源至目标
	DMA_InitStruct.DMA_BufferSize					=5;																//传输次数
	DMA_InitStruct.DMA_M2M								=DMA_M2M_Disable;									//硬件触发
	DMA_InitStruct.DMA_Mode								=DMA_Mode_Circular;								//传输计数器自动重装
	DMA_InitStruct.DMA_Priority						=DMA_Priority_Medium;							//优先级设置
	DMA_Init(DMA1_Channel1,&DMA_InitStruct);
	
	ADC_DMACmd(ADC1,ENABLE);										//开启ADC至DMA通道
	
	DMA_Cmd(DMA1_Channel1,ENABLE);							//开启DM1通道1
}


/** @brief	ADC1初始化
  * @retval	0:初始化成功，1：初始化失败
  **/
u8 ADC1_Init(void)
{
	ADC_GPIO_Init();					//GPIO初始化
	ADC1_In_Init();						//ADC1初始化
	ADC_DMA1_1_Init();				//DMA1初始化
	return ADC1_Calibrate();	//ADC校准
}

/** @brief	ADC1开始转换
  **/
void ADC1_Start(void)
{
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);		//触发转换
}








