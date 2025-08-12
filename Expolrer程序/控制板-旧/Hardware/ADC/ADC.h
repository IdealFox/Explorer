#ifndef __ADC_H
#define __ADC_H

extern u16 AD_Value[5];

u8 ADC1_Init(void);								//ADC模块初始化
void ADC1_Interrupt_Init(u8 PreemptionPriority,u8 SubPriority);		//ADC1中断初始化
void ADC1_Start(void);						//ADC1开始转换

#endif


