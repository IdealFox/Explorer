#include "stm32f10x.h"                  // Device header
#include "Interrupt.h"
#include "NRF24L01.h"
#include "Control.h"
#include "ADC.h"
#include "OLED.h"
#include "Key.h"
#include "LED.h"
#include "Timer.h"

/** @brief	中断初始化
  **/
void Interrupt_Init(void)
{
	//中断组选择
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//NVIC中断组2
	
	//中断初始化
	NRF24L01_Interrupt_Init(1,1);											//NRF24L01外部中断初始化
	Timer3_Interrupt_Init(0,1);												//TIM3中断初始化,20ms
}


/***************************
↓↓↓↓↓↓↓↓↓↓中断函数↓↓↓↓↓↓↓↓↓↓
***************************/

/** @brief	NRF24L01外部中断函数
  **/
void EXTI1_IRQHandler(void)
{	
	EXTI_ClearITPendingBit(EXTI_Line1);								//清除中断标志位
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
//		case 1:	NRF24L01_Receive_Data(NRF34L01_RX_Data);//接收数据
//						break;
		case 2: NRF24L01_Send_OK();											//发送完成操作
						break;
		case 3:	NRF24L01_Send_Error();									//发送失败操作
						break;
	}
}

extern u8 Control_State;

/** @brief	TIM3中断函数,20ms
  **/
void TIM3_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);													//清除中断标志位
	
	if(Control_State==1)
	{
		Servo_1_Set(AD_Final_Value[1]);					//舵机控制
		Servo_2_Set(AD_Final_Value[3]);
		Servo_3_Set(AD_Final_Value[2]);
	}
}















