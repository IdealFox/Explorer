/*
本函数提供STM32内部ADC获取的值处理电机与舵机控制参数
并通过NRF24L01发送数据
数据包长度：32字节

数据包格式
位					变量名						执行操作									取值范围
0				Control_Select				控制选择							0：电机控制							1：舵机控制
1					Motor_State					电机正反转						0~3位代表电机1~4状态		0正转		1反转

2~3				Motor[0]						电机1参数									0~1000
4~5				Motor[1]						电机2参数									0~1000
6~7				Motor[2]						电机3参数									0~1000
8~9				Motor[3]						电机4参数	   							0~1000

10~11			Servo[0]						舵机1参数									900+-200
12~13			Servo[1]						舵机2参数									100~1800
14~15			Servo[2]						舵机3参数									400~1800
16~17			Servo[3]						舵机4参数										0~300

18~19		B_Motor_Control			无刷电机参数								0~1000							为0，电机停止

20				verify							数据校验
*/

#include "stm32f10x.h"                  // Device header
#include "Control.h"
#include "ADC.h"

struct NRF24L01_Send_Data Send_Data;

/** @brief	电机正反转控制		用于运动时控制
  * @param	AD_Value				AD值
  **/
void Motor_Turn_Set(u16 AD_Value)
{
	if(AD_Value>=2500)
	{	Send_Data.Motor_State=0X00;}		//所有电机正转
	else if (AD_Value<=1500)
	{	Send_Data.Motor_State=0X0F;}		//所有电机反转
}

/** @brief	电机速度处理
	* @param	Speed_Value				原始速度参数		范围0~1500		2500~4096
  * @param	Turn_Value				原始旋转参数		范围0~600
  * @retval	最终速度
  **/
u16 Motor_Speed_Set(u16 Speed_Value,u16 Turn_Value)
{
	u16 Data;
	float Convert;
	if(Speed_Value>=2500)
	{	
		Speed_Value-=2500;
		Convert=(float)Speed_Value/1600;
		Data=Turn_Value*Convert;
		return Data;
	}
	if(Speed_Value<=1500)
	{	
		Convert=(float)Speed_Value/1500;
		Convert=1-Convert;
		Data=Turn_Value*Convert;
		return Data;
	}
	return 0;
}

/** @brief	电机移动数据处理			用于运动时控制
  * @param	Speed_Value			速度数据
	* @param	Turn_Value			旋转数据
	* @param	Motor_Select		电机选择		1：左侧电机		2：右侧电机
  * @retval	处理后的数据	范围400~1000
  **/
u16 Motor_Move_Convert(u16 Speed_Value,u16 Turn_Value,u8 Motor_Select)
{
	u8 State;						//状态字节		0：无旋转控制，1：右转控制，2：左转控制
	u16 Data,Turn_Set;
	float Convert;
	
	//获取当前状态
	if(Turn_Value>=2500)
	{	State=1;}												//右转控制状态
	else if(Turn_Value<=1500)
	{	State=2;}												//左转控制状态
	else 
	{	State=0;}												//无旋转控制
	
	switch(State)
	{
		case 0:	Data=Motor_Speed_Set(Speed_Value,1000);break;		//无旋转控制，获取最大速度值
		case 1:	if(Motor_Select==1)															//如果为左侧电机
						{	Data=Motor_Speed_Set(Speed_Value,1000);}				//获取最大值			
						else																						//如果为右侧电机
						{	Turn_Value-=2500;
							Convert=(float)Turn_Value/1600;
							Convert=1-Convert;
							Turn_Set=1000*Convert;
							Data=Motor_Speed_Set(Speed_Value,Turn_Set);		//获取处理值
						}				
						break;
		case 2:	if(Motor_Select==1)															//如果为左侧电机
						{	Convert=(float)Turn_Value/1500;
							Turn_Set=1000*Convert;
							Data=Motor_Speed_Set(Speed_Value,Turn_Set);		//获取处理值
						}				
						else																						//如果为右侧电机
						{	Data=Motor_Speed_Set(Speed_Value,1000);}				//获取最大值
						break;
	}
	return Data;
}

/** @brief	电机正反转控制		用于禁止时控制
  * @param	AD_Value				AD值
  **/
void Motor_Stop_Turn_Set(u16 AD_Value)
{
	if(AD_Value>=2500)
	{	Send_Data.Motor_State=0X0C;}		//电机1,2正转,3,4反转
	else if (AD_Value<=1500)
	{	Send_Data.Motor_State=0X03;}		//电机1,2反转,3,4正转
}

/** @brief	电机移动数据处理			用于静止时控制
	* @param	Turn_Value			旋转数据
  * @retval	处理后的数据	范围400~1000
  **/
u16 Motor_Stop_Convert(u16 Turn_Value)
{	
	u16 Data;
	float Convert;
	if(Turn_Value>=2500)
	{	
		Turn_Value-=2500;
		Convert=(float)Turn_Value/1600;
		Data=1000*Convert;
		return Data;
	}
	if(Turn_Value<=1500)
	{	
		Convert=(float)Turn_Value/1500;
		Convert=1-Convert;
		Data=1000*Convert;
		return Data;
	}
	return 0;
}

/** @brief	电机控制数据处理
  **/
void Motor_Control(void)
{
	Send_Data.Control_Select=0;			//电机控制
	Send_Data.Servo[0]=900;					//舵机1参数复位
	
	u16 Speed,Turn;			
	Speed=AD_Value[1];							//速度控制AD
	Turn=AD_Value[2];								//旋转控制AD
	
	if(Speed>=2500||Speed<=1500)		//速度参数值溢出
	{	
		Motor_Turn_Set(Speed);				//电机正反转设置
		
		//电机速度设置，传递参数：速度控制AD,旋转控制AD
		Send_Data.Motor[0]=Motor_Move_Convert(Speed,Turn,1);		//电机1，左前
		Send_Data.Motor[1]=Motor_Move_Convert(Speed,Turn,1);		//电机2，左后
		Send_Data.Motor[2]=Motor_Move_Convert(Speed,Turn,2);		//电机3，右前
		Send_Data.Motor[3]=Motor_Move_Convert(Speed,Turn,2);		//电机4，右后
	}
	else														//若AD值未溢出
	{	
		Send_Data.Motor[0]=0;					//电机速度参数复位
		Send_Data.Motor[1]=0;
		Send_Data.Motor[2]=0;
		Send_Data.Motor[3]=0;	
		if(Turn>=2500||Turn<=1500)		//旋转参数溢出
		{	Motor_Stop_Turn_Set(Turn);	//电机正反转设置
		Send_Data.Motor[0]=Motor_Stop_Convert(Turn);		//电机1，左前
		Send_Data.Motor[1]=Motor_Stop_Convert(Turn);		//电机2，左后
		Send_Data.Motor[2]=Motor_Stop_Convert(Turn);		//电机3，右前
		Send_Data.Motor[3]=Motor_Stop_Convert(Turn);		//电机4，右后
		}	
	}
	Send_Data.B_Motor_Control=B_Motor_Value();				//无刷电机控制
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/** @brief	舵机1角度控制
  * @param	AD_Value				AD原始参数
	* @retval	舵机0角度数据		范围：900+-200
  **/
u16	Servo_0_Set(u16 AD_Value)
{
	u16 Data;
	float Convert;
	if(AD_Value>=2500)
	{
		AD_Value-=2500;
		Convert=(float)AD_Value/1600;
		Data=900-150*Convert;
		return Data;
	}
	else if(AD_Value<=1500)
	{
		Convert=(float)AD_Value/1500;
		Convert=1-Convert;
		Data=150*Convert+900;
		return Data;
	}
	else
	{return 900;}
}

/** @brief	舵机2角度控制
  * @param	AD_Value				AD原始参数
	* @retval	舵机0角度数据		范围：100~1800
  **/
void Servo_1_Set(u16 AD_Value)
{
	s8 Add_Value=0;
	float Convert;
	if(AD_Value>=2500)
	{
		AD_Value-=2500;
		Convert=(float)AD_Value/1600;
		Add_Value+=(50*Convert);
	}
	else if(AD_Value<=1500)
	{
		Convert=(float)AD_Value/1600;
		Convert=1-Convert;
		Add_Value-=(50*Convert);
	}
	Send_Data.Servo[1]+=Add_Value;
	
	//舵机2参数防溢出		100~1800
	if(Send_Data.Servo[1]<=100){Send_Data.Servo[1]=100;}
	if(Send_Data.Servo[1]>=1800){Send_Data.Servo[1]=1800;}
}

/** @brief	舵机3角度控制
  * @param	AD_Value				AD原始参数
	* @retval	舵机0角度数据		范围：400~1800
  **/
void Servo_2_Set(u16 AD_Value)
{
	s8 Add_Value=0;
	float Convert;
	if(AD_Value>=2500)
	{
		AD_Value-=2500;
		Convert=(float)AD_Value/1600;
		Add_Value-=(50*Convert);
	}
	else if(AD_Value<=1500)
	{
		Convert=(float)AD_Value/1600;
		Convert=1-Convert;
		Add_Value+=(50*Convert);
	}
	Send_Data.Servo[2]+=Add_Value;
	
	//舵机3参数防溢出		400~1800
	if(Send_Data.Servo[2]<=400){Send_Data.Servo[2]=400;}
	if(Send_Data.Servo[2]>=1800){Send_Data.Servo[2]=1800;}
}

/** @brief	舵机4角度控制
  * @param	AD_Value				AD原始参数
	* @retval	舵机0角度数据		范围：0~300
  **/
void Servo_3_Set(u16 AD_Value)
{
	s8 Add_Value=0;
	float Convert;
	if(AD_Value>=2500)
	{
		AD_Value-=2500;
		Convert=(float)AD_Value/1600;
		Add_Value+=(50*Convert);
	}
	else if(AD_Value<=1500)
	{
		Convert=(float)AD_Value/1600;
		Convert=1-Convert;
		Add_Value-=(50*Convert);
	}
	Send_Data.Servo[3]+=Add_Value;
	
	//舵机4参数防溢出		0~300
	if(Send_Data.Servo[3]>=10000){Send_Data.Servo[3]=0;}
	if(Send_Data.Servo[3]>=300){Send_Data.Servo[3]=300;}
}

/** @brief	舵机控制数据处理
  **/
void Servo_Control(void)
{
	Send_Data.Control_Select=1;			//舵机控制
	Send_Data.Motor[0]=0;						//电机速度参数复位
	Send_Data.Motor[1]=0;
	Send_Data.Motor[2]=0;
	Send_Data.Motor[3]=0;	
	
	Send_Data.B_Motor_Control=0;		//无刷电机复位
	
	Send_Data.Servo[0]=Servo_0_Set(AD_Value[0]);
	
//	//舵机3参数防溢出		400~1800
//	if(Send_Data.Servo[1]<=100){Send_Data.Servo[1]=400;}
//	if(Send_Data.Servo[1]>=1800){Send_Data.Servo[1]=1800;}
//	//舵机4参数防溢出		0~300
//	if(Send_Data.Servo[1]<=100){Send_Data.Servo[1]=0;}
//	if(Send_Data.Servo[1]>=1800){Send_Data.Servo[1]=300;}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/** @brief	无刷电机控制电位器初始化
  **/
void B_Motor_Init(void)
{ 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Speed	=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
}

/** @brief	获取无刷电机控制电位器状态
  **/
u8 Get_B_Motor_State(void)
{
	return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14);
}

/** @brief	获取无刷电机控制参数
	* @retval	获取的参数					范围：0~1000			为0，电机停止
  **/
u16 B_Motor_Value(void)
{
	u16 Value;
	float Convert;
	
	if(Get_B_Motor_State()==1)
	{	return 0;}
	
	Convert=(float)AD_Value[4]/4096;
	Value=Convert*1000;
	return Value;
}


	

