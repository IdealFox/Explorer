#ifndef __Control_H
#define __Control_H

struct NRF24L01_Send_Data			//NRF24L01待发区结构体
{	
	u8 Control_Select;					//电机与舵机选择控制字节
	u8 Motor_State;							//电机正反转控制字节
	u16	Motor[4];								//电机参数
	u16 Servo[4];								//舵机参数
	u16 B_Motor_Control;				//无刷电机控制字节
	u8 verify; 									//校验位
	u8 Air_Data[14];						//空字节，占位置用
};

extern struct NRF24L01_Send_Data Send_Data;

void Motor_Control(void);			//电机控制数据处理
void Servo_Control(void);			//舵机控制数据处理

void B_Motor_Init(void);			//无刷电机控制电位器初始化
u8 Get_B_Motor_State(void);		//获取无刷电机控制电位器状态
u16 B_Motor_Value(void);			//获取无刷电机控制参数

void Servo_1_Set(u16 AD_Value);	//舵机2角度控制
void Servo_2_Set(u16 AD_Value);	//舵机3角度控制
void Servo_3_Set(u16 AD_Value);	//舵机4角度控制

#endif


