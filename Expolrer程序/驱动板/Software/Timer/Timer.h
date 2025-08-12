#ifndef __Timer_H
#define __Timer_H

void Timer3_Init(void);								//定时器3初始化

void Timer3_Interrupt_Init(u8 PreemptionPriorit,u8 SubPriority);		//TIM3中断初始化

#endif


