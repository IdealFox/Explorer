#include "stm32f10x.h"                  // Device header
#include "LED.h"

/** @brief	LED初始化函数
  * @param	无
  * @retval	无
  **/
void RunLED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Speed	=GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStruct);
	
	GPIO_WriteBit(GPIOC,GPIO_Pin_13,Bit_SET);
}

/** @brief	更改LED状态
  * @param	0，LED关闭，其他，LED开启
  * @retval	无
  **/
void RunLED_State(u8 state)
{
	if(state==0)
	{	GPIO_WriteBit(GPIOC,GPIO_Pin_13,Bit_SET);}
	else
	{	GPIO_WriteBit(GPIOC,GPIO_Pin_13,Bit_RESET);}
}

