#ifndef __Key_H
#define __Key_H


void DIP_Switch(void);					//按键初始化

u8 TEST_Motor_Control(void);		//控制状态检测
u8 Get_B_Motor_State(void);			//获取无刷电机控制电位器状态

void Key_Loop(void);						//按键循环检测，由定时器调用
u8 Key_GetNum(void);						//查看当前键码值

u8 Get_Air_Pump_State(void);		//获取气泵控制状态

#endif


