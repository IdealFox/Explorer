#include "stm32f10x.h"                  // Device header
#include "BEEP.h"

/** @brief	蜂鸣器初始化
  * @param	
  * @retval	
  **/
void BEEP_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Speed	=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_SET);
}

/** @brief	更改蜂鸣器状态
  * @param	0，蜂鸣器关闭，其他，蜂鸣器开启
  **/
void BEEP_State(u8 State)
{
	if(State==0)
	{	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_SET);}
	else
	{	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET);}
}


