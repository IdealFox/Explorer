#include "stm32f10x.h"                  // Device header
#include "TB6612.h"
//TB6612FNG电机驱动模块函数库
/*引脚定义	
TIM1复用推挽输出引脚					74HC595数据引脚
PWM1A		CH1通道		PA8					SER			PB8
PWM1B		CH2通道		PA9					RCK			PB9
PWM2A		CH3通道		PA10				SCK			PB10
PWM2B		CH4通道		PA11
*/

/** @brief	PWM信号输出GPIO初始化
  * @param	无
  * @retval	无
  **/
void PWM_GPIO_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	//使能GPIOA端口
	
	//GPIO初始化结构体配置
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode		=	GPIO_Mode_AF_PP;																//复用推挽输出
	GPIO_InitStruct.GPIO_Pin		=	GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;	//8，9，10，11引脚
	GPIO_InitStruct.GPIO_Speed	=	GPIO_Speed_50MHz;																//速度50MHz
	GPIO_Init(GPIOA,&GPIO_InitStruct);																						//GPIOA初始化
}

/** @brief	TIM1初始化，内部时钟，100Hz
  * @param	无
  * @retval	无
  **/
void Timer1_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);									//使能TIM1
	TIM_InternalClockConfig(TIM1);																			//内部时钟
	
	//时基单元结构体配置
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision			=	TIM_CKD_DIV1;				//1分频
	TIM_TimeBaseInitStruct.TIM_CounterMode				=	TIM_CounterMode_Up;	//向上计数
	TIM_TimeBaseInitStruct.TIM_Period							=	10000-1;						//ARR重装寄存器值
	TIM_TimeBaseInitStruct.TIM_Prescaler					=	72-1;								//PSC预分频器值
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter	=	0;									//重复计数器关
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStruct);											//定时器1初始化
	
	TIM_Cmd(TIM1,ENABLE);																								//开启定时器1
}

/** @brief	TIM1中断初始化
  * @param	PreemptionPriorit			抢占优先级
  * @param	SubPriority						响应优先级
  **/
void Timer1_Interrupt_Init(u8 PreemptionPriorit,u8 SubPriority)
{
	TIM_ClearITPendingBit(TIM1,TIM_IT_Update);													//清除中断标志位
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);														//开启中断
	
	//NVIC结构体配置
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel										= TIM1_UP_IRQn;			//定时器中断通道
	NVIC_InitStruct.NVIC_IRQChannelCmd								= ENABLE;						//使能NVIC
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	= PreemptionPriorit;//抢占优先级0
	NVIC_InitStruct.NVIC_IRQChannelSubPriority				= SubPriority;			//响应优先级1
	NVIC_Init(&NVIC_InitStruct);
}

/** @brief	电机驱动PWM初始化	初始占空比0
  * @param	无
  * @retval	无
  **/
void Motor_PWM_Init(void)
{
	//输出比较结构体初始化
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCStructInit(&TIM_OCInitStruct);												//结构体内所有参数赋初始值
	TIM_OCInitStruct.TIM_OCMode				=	TIM_OCMode_PWM1;				//PWM1输出模式
	TIM_OCInitStruct.TIM_OCPolarity		=	TIM_OCPolarity_High;		//输出极性为高电平
	TIM_OCInitStruct.TIM_OutputState	=	TIM_OutputState_Enable;	//输出使能
	TIM_OCInitStruct.TIM_Pulse				=	0;											//CCR捕获比较寄存器值，默认0
	TIM_OC1Init(TIM1,&TIM_OCInitStruct);												//初始化CH1通道
	TIM_OC2Init(TIM1,&TIM_OCInitStruct);												//初始化CH2通道
	TIM_OC3Init(TIM1,&TIM_OCInitStruct);												//初始化CH3通道
	TIM_OC4Init(TIM1,&TIM_OCInitStruct);												//初始化CH4通道
	
	TIM_CtrlPWMOutputs(TIM1,ENABLE);														//TIM1输出使能
}
///////////////////////////////////////////////////////////////////////////////////////////////////

/** @brief	74HC595信号端口初始化
  **/
void HC595_GPIO_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);				//使能GPIOB
	
	//GPIO初始化结构体配置
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode		=	GPIO_Mode_Out_PP;										//推挽输出
	GPIO_InitStruct.GPIO_Pin		=	GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;	//8，9，10引脚
	GPIO_InitStruct.GPIO_Speed	=	GPIO_Speed_10MHz;										//速度10MHz
	GPIO_Init(GPIOB,&GPIO_InitStruct);		
	
	GPIO_WriteBit(GPIOB,GPIO_Pin_8,Bit_RESET);												//所有端口置0
	GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_RESET);
}

/** @brief	发送数据至74HC595,595作为输出控制电机		
  * @param	@ref电机控制模式
  **/
void Motor_Control(u16 Data)
{
	RCK0;
	for(u8 i=0;i<16;i++)
	{	if(Data&(0x8000>>i))
		{	SER1}
		else
		{	SER0}
		SCK1;
		SCK0;
	}
	RCK1;
	RCK0;
}

/** @brief	电机控制模块初始化
  **/
void Motor_Control_Init(void)
{
	PWM_GPIO_Init();							//PWM信号输出GPIO初始化
	HC595_GPIO_Init();						//74HC595信号端口初始化
	Timer1_Init();								//TIM1初始化，内部时钟，100Hz
	Motor_PWM_Init();							//电机驱动PWM初始化	初始占空比0
}

/** @brief	电机1速度控制函数，电机为左前方电机
  * @param	PWM输出占空比，精度0.1%，范围0~1000
  **/
void Motor1_Speed_Set(u16 Duty)
{	TIM_SetCompare1(TIM1,(Duty*10));
}

/** @brief	电机2速度控制函数，电机为左后方电机
  * @param	PWM输出占空比，精度0.1%，范围0~1000
  **/
void Motor2_Speed_Set(u16 Duty)
{	TIM_SetCompare2(TIM1,(Duty*10));
}

/** @brief	电机1速度控制函数，电机为右前方电机
  * @param	PWM输出占空比，精度0.1%，范围0~1000
  **/
void Motor3_Speed_Set(u16 Duty)
{	TIM_SetCompare3(TIM1,(Duty*10));
}

/** @brief	电机1速度控制函数，电机为右后方电机
  * @param	PWM输出占空比，精度0.1%，范围0~1000
  **/
void Motor4_Speed_Set(u16 Duty)
{	TIM_SetCompare4(TIM1,(Duty*10));
}

///** @brief	TB6612模块控制
//  * @param	state stop待机   run使能
//  * @retval	无
//  **/
//void TB6612_Control(u8 state)
//{
//	if(state==stop)
//	{	GPIO_WriteBit(GPIOA,GPIO_Pin_3,Bit_RESET);}								//STYB引脚为低电平，TB6612待机
//	else if(state==run) 
//	{	GPIO_WriteBit(GPIOA,GPIO_Pin_3,Bit_SET);}									//STYB引脚为高电平，TB6612使能
//}

///** @brief	电机1控制
//  * @param	state 	stop待机	brake制动	right右转	left左转
//	* @param	Duty_Cycle	电机速度	0.01%精度	范围：0~10000
//  * @retval	
//  **/
//void Motor1_Control(u8 state,u16 Duty_Cycle)
//{
//	if(state==stop)
//	{	GPIO_WriteBit(GPIOA,GPIO_Pin_4|GPIO_Pin_5,Bit_RESET);}		//AIN1=0,AIN2=0,电机停止运行
//	else if(state==brake)
//	{	GPIO_WriteBit(GPIOA,GPIO_Pin_4|GPIO_Pin_5,Bit_SET);}			//AIN1=1,AIN2=1,电机制动
//	else if(state==right)
//	{	GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_RESET);
//		GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_SET);									//AIN1=0,AIN2=1,电机右转
//	}
//	else if(state==left)
//	{	GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_SET);
//		GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_RESET);								//AIN1=1,AIN2=0,电机左转
//	}
//	
//	TIM_SetCompare1(TIM3,Duty_Cycle);														//更改HC1通道CCR捕获比较寄存器值
//}
///** @brief	电机2控制
//  * @param	state 	stop待机	brake制动	right右转	left左转
//	* @param	Duty_Cycle	电机速度	0.01%精度	范围：0~10000
//  * @retval	
//  **/
//void Motor2_Control(u8 state,u16 Duty_Cycle)
//{
//	if(state==stop)
//	{	GPIO_WriteBit(GPIOA,GPIO_Pin_8|GPIO_Pin_9,Bit_RESET);}		//AIN1=0,AIN2=0,电机停止运行
//	else if(state==brake)
//	{	GPIO_WriteBit(GPIOA,GPIO_Pin_8|GPIO_Pin_9,Bit_SET);}			//AIN1=1,AIN2=1,电机制动
//	else if(state==right)
//	{	GPIO_WriteBit(GPIOA,GPIO_Pin_8,Bit_RESET);
//		GPIO_WriteBit(GPIOA,GPIO_Pin_9,Bit_SET);									//AIN1=0,AIN2=1,电机右转
//	}
//	else if(state==left)
//	{	GPIO_WriteBit(GPIOA,GPIO_Pin_8,Bit_SET);
//		GPIO_WriteBit(GPIOA,GPIO_Pin_9,Bit_RESET);								//AIN1=1,AIN2=0,电机左转
//	}
//	
//	TIM_SetCompare2(TIM3,Duty_Cycle);														//更改HC1通道CCR捕获比较寄存器值
//}




