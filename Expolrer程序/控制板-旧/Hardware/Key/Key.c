#include "stm32f10x.h"                  // Device header
#include "Key.h"

u8 KeyNum;

/** @brief	按键初始化
  **/
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed	=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
}

/** @brief	获取键码值
	* @retval	键码值	0:无按键按下
  **/
u8 KeyNum_Test(void)
{
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15))
	{	return 1;}
	return 0;	
}

/** @brief	按键循环检测，由定时器中断调用
  **/
void Key_Loop(void)
{
	static u8 Key_LastNum,Key_NowNum;
	Key_LastNum=Key_NowNum;
	Key_NowNum=KeyNum_Test();
	
//	if(Key_LastNum==0&&Key_NumNum!=0)				//按键按下
//	{	KeyNum=Key_NowNum;}
	
		if(Key_LastNum!=0&&Key_NowNum==0)				//按键松开
		{	KeyNum=Key_LastNum;}
}

/** @brief	获取键码值
  * @retval	键码值
  **/
u8 Get_KeyNum(void)
{
	u8 KeyNum_Return;
	KeyNum_Return=KeyNum;								//转移键码
	KeyNum=0;														//清零键码
	return KeyNum_Return;								//返回键码
}
