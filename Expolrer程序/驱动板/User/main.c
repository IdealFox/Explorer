/*
本程序所使用的硬件外设
		外设						作用
		TIM1						PWM输出（4路）
		TIM2						PWM输出（4路）
		SPI1						无线通讯
	
*/
#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Servo.h"
#include "TB6612.h"
#include "NRF24L01.h"
#include "Control.h"
#include "SPI.h"
#include "Timer.h"
#include "B_Motor.h"
#include "BEEP.h"
#include "Interrupt.h"

void Servo_Control_Init(void);

int main(void)
{
	
	RunLED_Init();					//指示灯初始化
	RunLED_State(1); 				//开启运行指示灯
	BEEP_Init();						//蜂鸣器初始化
	B_Motor_Init();					//无刷电机初始化
	Timer3_Init();					//定时器3初始化
	SPI1_Init();						//硬件SPI1初始化
	Motor_Control_Init();		//电机控制初始化
	Servo_Init(900,900,900,200);//舵机初始化
	
	
	if(NRF24L01_Init()==0)	//NRF24L01初始化
	{	IndicateLED_State(1);}
	Interrupt_Init();				//中断初始化
	
	Servo_Control_Init();		//舵机控制参数初始化
	
	while(1)
	{
		Control();

	}
		
}

/** @brief	舵机控制参数初始化
  **/
void Servo_Control_Init(void)
{
	Receive_Data.Servo[0]=900;
	Receive_Data.Servo[1]=900;
	Receive_Data.Servo[2]=900;
	Receive_Data.Servo[3]=200;
}

