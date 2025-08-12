#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "ADC.h"
#include "NRF24L01.h"
#include "Key.h"
#include "SPI.h"
#include "Interrupt.h"
#include "Timer.h"
#include "Control.h"
 
u16 ShowNum;
u8 Control_State=0;							//控制选择状态字节		0：电机控制		1：舵机控制
 
//函数声明
void Servo_Control_Init(void);
void OLED_Show_State1(void);
void OLED_Show_State2(void);
 

int main(void)
{
	RunLED_Init();							//运行指示灯初始化
	RunLED_State(1); 						//开启运行指示灯
	Key_Init();									//按键初始化
	B_Motor_Init();							//无刷电机控制电位器初始化
	OLED_Init();								//OLED初始化
	Timer3_Init();							//TIM3初始化
	if(ADC1_Init())							//ADC1初始化
	{	while(1)OLED_ShowString(1,1,"ADC_Error");}				//初始化失败
	ADC1_Start();								//开启AD转换
	SPI1_Init();								//SPI1初始化
	if(NRF24L01_Init())					//NRF24L01初始化
	{	while(1)OLED_ShowString(1,1,"NRF24L01_Error");}		//初始化失败
	Interrupt_Init();						//中断初始化
	
//	Delay_ms(100);
	
	Servo_Control_Init();				//舵机控制参数初始化
	
	while(1)
	{

		
		if(Get_KeyNum())
		{	Control_State++;}
		Control_State%=2;								//防溢出
		switch(Control_State)
		{
			case 0:	Motor_Control();			//电机控制
							OLED_Show_State1();		//OLED参数显示1，电机状态显示
							break;
			case 1:	Servo_Control();			//舵机控制
							OLED_Show_State2();		//OLED参数显示2，舵机状态显示
							break;
		}
		
		OLED_ShowNum(1,15,NRF24L01_State,2);		//显示状态
		if(NRF24L01_State==7||NRF24L01_State==8)
			OLED_ShowString(4,1,"Send_Error");
		if(NRF24L01_State==6)
			OLED_ShowString(4,1,"Send_OK   ");
		
		//发送数据
		if(NRF24L01_State==0||NRF24L01_State==3||NRF24L01_State==6||NRF24L01_State==7)
		NRF24L01_Send_Data(&Send_Data.Control_Select);
		
		Send_Data.verify++;					//校验位自增
	}
		
}

/** @brief	舵机控制参数初始化
  **/
void Servo_Control_Init(void)
{
	Send_Data.Servo[0]=900;
	Send_Data.Servo[1]=900;
	Send_Data.Servo[2]=900;
	Send_Data.Servo[3]=200;
}

/** @brief	OLED参数显示1，电机状态显示
  **/
void OLED_Show_State1(void)
{
	OLED_ShowString(1,1,"M_L:");
	OLED_ShowString(2,1,"M_R:");
	OLED_ShowString(3,1,"B_M:");
	
	if(Send_Data.Motor_State&0X01)
	{	OLED_ShowString(1,9," B");}
	else 
	{	OLED_ShowString(1,9," A");}
	if(Send_Data.Motor_State&0X04)
	{	OLED_ShowString(2,9," B");}
	else 
	{	OLED_ShowString(2,9," A");}
	
	OLED_ShowNum(1,5,Send_Data.Motor[0],4);
	OLED_ShowNum(2,5,Send_Data.Motor[2],4);
	
	if(Get_B_Motor_State()==0)
	{	OLED_ShowString(3,9," ON ");}
	else
	{	OLED_ShowString(3,9," OFF");}

	OLED_ShowNum(3,5,Send_Data.B_Motor_Control,4);
}

/** @brief	OLED参数显示2，舵机状态显示
  **/
void OLED_Show_State2(void)
{
	u8 High_Show1=Send_Data.Servo[0]/10;
	u8 High_Show2=Send_Data.Servo[1]/10;
	u8 High_Show3=Send_Data.Servo[2]/10;
	u8 High_Show4=Send_Data.Servo[3]/10;
	
	OLED_ShowString(1,1,"S1:   . ");
	OLED_ShowString(1,9,"S2:   . ");
	OLED_ShowString(2,1,"S3:   . ");
	OLED_ShowString(2,9,"S4:   . ");
	
	OLED_ShowNum(1,4,High_Show1,3);
	OLED_ShowNum(1,12,High_Show2,3);
	OLED_ShowNum(2,4,High_Show3,3);
	OLED_ShowNum(2,12,High_Show4,3);
	
	OLED_ShowNum(1,8,Send_Data.Servo[0],1);
	OLED_ShowNum(1,16,Send_Data.Servo[1],1);
	OLED_ShowNum(2,8,Send_Data.Servo[2],1);
	OLED_ShowNum(2,16,Send_Data.Servo[3],1);
}

