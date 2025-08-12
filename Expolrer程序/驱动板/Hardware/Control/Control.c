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

21				Bump_State					气泵控制								0：停止		1：启用

20				verify							数据校验
*/

#include "stm32f10x.h"                  // Device header
#include "Control.h"
#include "Servo.h"
#include "TB6612.h"

struct NRF24L01_Receive_Data Receive_Data;

/** @brief	电机状态控制
	* @param	State	电机状态		0~3位代表电机1~4状态		0正转		1反转
  **/
void Motor_State_Control(u8 State)
{
	
	u16 Motor_State=0X0000;
	if(Receive_Data.Control_Select==0)		//电机控制
	{
		Motor_State|=Motor_ALL_Enable;
		if(State&0X01)
		Motor_State|=M1_B;
		else
		Motor_State|=M1_A;
		
		if(State&0X02)
		Motor_State|=M2_B;
		else
		Motor_State|=M2_A;
		
		if(State&0X04)
		Motor_State|=M3_B;
		else
		Motor_State|=M3_A;
		
		if(State&0X08)
		Motor_State|=M4_B;
		else
		Motor_State|=M4_A;
	}
	else																	//非电机控制
	{Motor_State=Motor_ALL_Disable;}
	
	if(Receive_Data.Bump_State==1)
	{Motor_State|=M_PUMP_ON;}
	
	Motor_Control(Motor_State);						//执行控制
}


/** @brief	根据NRF24L01接收数据执行控制
  **/
void Control(void)
{
	
	Motor_State_Control(Receive_Data.Motor_State);		//电机状态控制
	
	Motor1_Speed_Set(Receive_Data.Motor[0]);					//电机速度控制
	Motor2_Speed_Set(Receive_Data.Motor[1]);
	Motor3_Speed_Set(Receive_Data.Motor[2]);
	Motor4_Speed_Set(Receive_Data.Motor[3]);
	
	Servo1_SetAngle(Receive_Data.Servo[0]);						//舵机控制
	Servo2_SetAngle(Receive_Data.Servo[1]);						//舵机控制
	Servo3_SetAngle(Receive_Data.Servo[2]);						//舵机控制
	Servo4_SetAngle(Receive_Data.Servo[3]);						//舵机控制

}

/** @brief	控制参数复位
  **/
void Control_RESET(void)
{
	Receive_Data.Control_Select=0;
	Receive_Data.Motor[0]=0;
	Receive_Data.Motor[1]=0;
	Receive_Data.Motor[2]=0;
	Receive_Data.Motor[3]=0;
	Receive_Data.Servo[0]=900;
	Receive_Data.B_Motor_Control=0;
}

















