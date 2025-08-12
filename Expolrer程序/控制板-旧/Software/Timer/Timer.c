#include "stm32f10x.h"                  // Device header
#include "Timer.h"

/** @brief	定时器3初始化，20ms
  **/
void Timer3_Init(void)
{
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);									//使能TIM3
	
	TIM_InternalClockConfig(TIM3);																			//内部时钟
	
	//时基单元结构体配置
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision			=	TIM_CKD_DIV1;				//1分频
	TIM_TimeBaseInitStruct.TIM_CounterMode				=	TIM_CounterMode_Up;	//向上计数
	TIM_TimeBaseInitStruct.TIM_Period							=	200-1;							//重装寄存器值
	TIM_TimeBaseInitStruct.TIM_Prescaler					=	7200-1;							//预分频器值
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter	=	0;									//重复计数器关，仅高级定时器可开启
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);											//定时器3初始化
	
	TIM_Cmd(TIM3,ENABLE);																								//开启定时器3
	
}

/** @brief	TIM3中断初始化
  * @param	PreemptionPriorit			抢占优先级
  * @param	SubPriority						响应优先级
  **/
void Timer3_Interrupt_Init(u8 PreemptionPriorit,u8 SubPriority)
{
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);													//清除中断标志位
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);														//开启中断
	
	//NVIC结构体配置
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel										= TIM3_IRQn;			//定时器中断通道
	NVIC_InitStruct.NVIC_IRQChannelCmd								= ENABLE;					//使能NVIC
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	= PreemptionPriorit;							//抢占优先级0
	NVIC_InitStruct.NVIC_IRQChannelSubPriority				= SubPriority;							//响应优先级1
	NVIC_Init(&NVIC_InitStruct);
}






