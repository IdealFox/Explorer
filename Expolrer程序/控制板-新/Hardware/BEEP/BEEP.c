#include "BEEP.h"

/** @brief	蜂鸣器初始化，PB8
  * @param	无
  * @retval	无
  **/
void BEEP_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);				//GPIOB使能
	
	//初始化结构体
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;						//推挽输出		
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;								//8号引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;						//翻转速度50MHz
	
	GPIO_Init(GPIOB,&GPIO_InitStructure);												//初始化GPIOB端口
	
	GPIO_SetBits(GPIOB,GPIO_Pin_13);														//蜂鸣器初始状态,灭
}

/** @brief	蜂鸣器状态
  * @param	1，响，0灭
  * @retval	无
  **/
void BEEP_State(u8 state)
{
	if(state)
	{GPIO_SetBits(GPIOB,GPIO_Pin_13);}
	else
	{GPIO_ResetBits(GPIOB,GPIO_Pin_13);}
}
