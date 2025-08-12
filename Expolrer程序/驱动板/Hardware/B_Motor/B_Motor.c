#include "stm32f10x.h"                  // Device header
#include "B_Motor.h"

/** @brief	无刷电机初始化,PB13
  **/
void B_Motor_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Speed	=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET);
}

/** @brief	无刷电机控制
	* @param	State		0:低电平		1:高电平
  **/
void B_Motor_Control(u8 State)
{
	if(State==0)
	{	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET);}
	else
	{	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_SET);}
}


