#ifndef __Control_H
#define __Control_H

struct NRF24L01_Receive_Data			//NRF24L01待发区结构体
{
	u8 Control_Select;					//电机与舵机选择控制字节
	u8 Motor_State;							//电机正反转控制字节
	u16	Motor[4];								//电机参数
	u16 Servo[4];								//舵机参数
	u16 B_Motor_Control;				//无刷电机控制字节
	u8 Bump_State;							//气泵控制
	u8 verify; 									//校验位
	u8 Air_Data[13];						//空字节，占位置用
};

extern struct NRF24L01_Receive_Data Receive_Data;

void Control(void);						//根据NRF24L01接收数据执行控制
void Control_RESET(void);			//控制参数复位


#endif


