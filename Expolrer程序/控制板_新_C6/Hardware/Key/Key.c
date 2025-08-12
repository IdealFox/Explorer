#include "stm32f10x.h"                  // Device header
#include "Key.h"

u8 Key_Test_Num=0;
//用于接收键码值

/** @brief	按键初始化,拨码按键PB2,PB5,独立按键PB8
  * @param	无
  * @retval	无
  **/
void DIP_Switch(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);				//GPIOB使能
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;							//上拉输入		
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2|GPIO_Pin_5|GPIO_Pin_8;//2,5,8号引脚
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

/** @brief	获取无刷电机控制电位器状态
	* @retval	1:启用控制		0:禁用控制
  **/
u8 Get_B_Motor_State(void)
{
	return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5);
}

/** @brief	按键检测函数,无消抖，本函数内调用
  * @retval	键码值
  **/
u8 Key_Num(void)
{
	u8 KeyNum=0;
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)==0)
	{KeyNum=1;}
	return KeyNum;
}

/** @brief	按键循环检测，由定时器调用，建议设置20ms定时器
  **/
void Key_Loop(void)
{
	static u8 Now_Num,Last_Num;
	Last_Num=Now_Num;
	Now_Num=Key_Num();														//检测键码值，并赋予变量
	
//	if(Last_Num==0 && Now_Num!=0)									//检测到按键按下
	
	if(Last_Num!=0 && Now_Num==0)									//检测到按键松开
	{
		Key_Test_Num=Last_Num;											//将键码值赋予全局变量
	}
}

/** @brief	查看当前键码值
  * @retval	当前键码值
  **/
u8 Key_GetNum(void)
{
	u8 Key_Test_Num_Return;
	Key_Test_Num_Return=Key_Test_Num;					//转移键码
	Key_Test_Num=0;														//清零键码
	return Key_Test_Num_Return;								//返回键码
}
//调用此函数后，需清零键码值

/** @brief	获取气泵控制状态
	* @retval	1:启用控制		0:禁用控制
  **/
u8 Get_Air_Pump_State(void)
{
	static u8 State=0;
	if(Key_GetNum()==1)
		State++;
	State%=2;
	return State;
}

