#include "stm32f10x.h"                  // Device header
#include "Key.h"

/** @brief	按键初始化,拨码按键PB2
  * @param	无
  * @retval	无
  **/
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);				//GPIOB使能
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;							//上拉输入		
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;									//2号引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;						//翻转速度50MHz
	GPIO_Init(GPIOB,&GPIO_InitStructure);												//初始化GPIOF端口
}

/** @brief	控制状态检测
	* @retval	1:车轮控制		0:舵机控制
  **/
u8 TEST_Motor_Control(void)
{
	return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2);
}
