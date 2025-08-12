#include "stm32f10x.h"                  // Device header
#include "Interrupt.h"
#include "NRF24L01.h"
#include "OLED.h"
#include "TB6612.h"
#include "Control.h"
#include "B_Motor.h"
#include "Timer.h"
#include "BEEP.h"
#include "LED.h"

/** @brief	中断初始化
  **/
void Interrupt_Init(void)
{
	//中断组选择
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//NVIC中断组2
	
	//中断初始化
	NRF24L01_Interrupt_Init(1,1);											//NRF24L01外部中断
	Timer1_Interrupt_Init(0,0);												//TIM1定时中断，10ms
	Timer3_Interrupt_Init(0,1);												//TIM3定时中断，10ms
}


/***************************
↓↓↓↓↓↓↓↓↓↓中断函数↓↓↓↓↓↓↓↓↓↓
***************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief	NRF24L01外部中断函数
  **/
void EXTI15_10_IRQHandler(void)
{	
	EXTI_ClearITPendingBit(EXTI_Line12);								//清除中断标志位
	u8 Status,State;
	Status=NRF24L01_Read_STATUS();										//获取STATUS参数
	NRF24L01_W_Data(W_REGISTER|STATUS,Status);				//清除STATUS标志位
	
	if(Status==0XFF||Status==0X00){	State=0;}					//状态0，出现异常
	else if(Status&RX_DR){	State=1;NRF24L01_State=1;}//状态1，接收完成中断
	else if(Status&TX_DS){	State=2;}									//状态2，发送完成中断
	else if(Status&MAX_RT){	State=3;}									//状态3，最大重发中断
	
	switch(State)																			//执行各状态对应指令
	{	
		case 0:	NRF24L01_State=8;												//状态异常
						break;
		case 1:	NRF24L01_Receive_Data(&Receive_Data.Control_Select);//接收数据
						break;
		case 2: NRF24L01_Send_OK();											//发送完成操作
						break;
		case 3:	NRF24L01_Send_Error();									//发送失败操作
						break;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief	TIM1定时中断函数		10ms
  **/
void TIM1_UP_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM1,TIM_IT_Update);				//清除中断标志位
	
	static u8 Now_virecy,Last_Virecy;
	static u8 Data_State=0;							//数据状态位，0：数据接收正常，1：数据接收异常
	static u8 Check_Time=0;							//校验检测字节，10次检测到的校验位数据为一样，则接收数据异常
	static u8 BEEP_Time=0;							//蜂鸣器控制字节，20次翻转一次电平
	
	
	Last_Virecy=Now_virecy;							//获取校验位数据
	Now_virecy=Receive_Data.verify;
	
	if(Now_virecy==Last_Virecy)
	{	Check_Time++;}
	else
	{	Check_Time=0;}
	if(Check_Time>=20)									//10次检测到的校验位数据为一样，则接收数据异常
	{	Data_State=1;}
	
	if(Data_State==1)										//数据异常
	{
		Control_RESET();									//控制参数复位
		
		BEEP_Time++;											//蜂鸣器控制，200ms次翻转一次电平
		BEEP_Time%=41;
		if(BEEP_Time==20){BEEP_State(1);}
		if(BEEP_Time==40){BEEP_State(0);}
		
		if(Now_virecy!=Last_Virecy)				//数据恢复正常
		{	Data_State=0;
			BEEP_Time=0;
			BEEP_State(0);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief	TIM3定时中断函数		10us
  **/
void TIM3_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);													//清除中断标志位
	
	static u16 Duty,Cycle;
	
	
	Duty=Receive_Data.B_Motor_Control;
	Duty/=8;
	Duty+=87;
	
	Cycle++;
	Cycle%=2000;
	
	if(Cycle==0)
	{	B_Motor_Control(1);}
	if(Duty==Cycle)
	{	B_Motor_Control(0);}
}


