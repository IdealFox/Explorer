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
#include "USART.h"
#include "BEEP.h"
#include "lcd.h"
#include "pic.h"

 
u16 ShowNum;
 
//函数声明
void Servo_Control_Init(void);
void LCD_Show_Motor_Data(void);
void LCD_Show_Servo_Data(void);
void LCD_Show_Slave_Connect(void);
void LCD_Show_Bump_State(void);
 

int main(void)
{
	RunLED_Init();							//运行指示灯初始化
	RunLED_State(0); 						//开启运行指示灯
	DIP_Switch();								//拨码按键初始化
//	OLED_Init();								//OLED初始化
//	USART1_Init();							//串口初始化
	Timer3_Init();							//TIM3初始化
	ADC1_Init();								//ADC1初始化
	ADC1_Start();								//开启AD转换
	SPI1_Init();								//SPI1初始化
	LCD_Init();									//LCD初始化
	LCD_Clear();								//LCD清屏
	if(NRF24L01_Init())					//NRF24L01初始化
	{	while(1)									//初始化失败
	LCD_ShowString(10,10,"NRF24L01_Error",RED,WHITE,20,0);}		
	Interrupt_Init();						//中断初始化
	
	Servo_Control_Init();				//舵机控制参数初始化
	
	LCD_ShowPicture(105,73,50,50,Marisa);
	while(1)
	{
		while(NRF24L01_State==5){Delay_us(1);}					//等待数据发送完成
		
//		USART1_SendPacket(0XAB,0XCD,(u8*)AD_Final_Value,8);
		
		if(Send_Data.Control_Select==0)									//电机控制
		{
			LCD_Show_Motor_Data();												//LCD显示电机参数
		}
		else if(Send_Data.Control_Select==1)						//舵机控制
		{
			LCD_Show_Servo_Data();												//LCD显示舵机参数
		}
		
		LCD_Show_Slave_Connect();												//LCD显示从机连接状态
		LCD_Show_Bump_State();													//LCD显示气泵状态
		
		Control();									//控制参数计算
		
		if(NRF24L01_State==0||NRF24L01_State==3||NRF24L01_State==6||NRF24L01_State==7)
		NRF24L01_Send_Data(&Send_Data.Control_Select);//发送数据
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

/** @brief	LCD显示从机连接状态
  **/
void LCD_Show_Slave_Connect(void)
{
	LCD_ShowIntNum(140,2,NRF24L01_State,1,GRAY,WHITE,24);
	if(NRF24L01_State==6)
	{
		LCD_ShowString(95,6,"Slave_",GREEN,WHITE,12,0);
		LCD_ShowString(95,16,"Connect",GREEN,WHITE,12,0);
	}
	if(NRF24L01_State==7)
	{
		LCD_ShowString(95,6,"Slave_",RED,WHITE,12,0);
		LCD_ShowString(95,16,"Loss   ",RED,WHITE,12,0);
	}
}

/** @brief	LCD显示气泵状态
  **/
void LCD_Show_Bump_State(void)
{
	if(Send_Data.Bump_State==1)
	LCD_ShowString(95,55,"Bump_ON ",DARKBLUE,WHITE,12,0);
	else
	LCD_ShowString(95,55,"Bump_OFF",DARKBLUE,WHITE,12,0);	
}

/** @brief	LCD显示电机参数
  **/
void LCD_Show_Motor_Data(void)
{
	LCD_ShowString(95,30,"Motor",BLACK,WHITE,12,0);
	LCD_ShowString(95,40,"Control",BLACK,WHITE,12,0);
	
	LCD_ShowString(6,8,"M_1:",BLACK,WHITE,16,0);
	LCD_ShowString(6,32,"M_2:",BLACK,WHITE,16,0);
	LCD_ShowString(6,56,"M_3:",BLACK,WHITE,16,0);
	LCD_ShowString(6,80,"M_4:",BLACK,WHITE,16,0);
	LCD_ShowString(6,102,"B_M:",BROWN,WHITE,16,0);
	
	if(Send_Data.Motor[0]<=400)//电机1参数
	LCD_ShowIntNum(50,8,Send_Data.Motor[0],4,GBLUE,WHITE,16);
	else if(Send_Data.Motor[0]>=800)
	LCD_ShowIntNum(50,8,Send_Data.Motor[0],4,RED,WHITE,16);
	else
	LCD_ShowIntNum(50,8,Send_Data.Motor[0],4,BLUE,WHITE,16);
	
	if(Send_Data.Motor[1]<=400)//电机2参数
	LCD_ShowIntNum(50,32,Send_Data.Motor[1],4,GBLUE,WHITE,16);
	else if(Send_Data.Motor[1]>=800)
	LCD_ShowIntNum(50,32,Send_Data.Motor[1],4,RED,WHITE,16);
	else
	LCD_ShowIntNum(50,32,Send_Data.Motor[1],4,BLUE,WHITE,16);
	
	if(Send_Data.Motor[2]<=400)//电机3参数
	LCD_ShowIntNum(50,56,Send_Data.Motor[2],4,GBLUE,WHITE,16);
	else if(Send_Data.Motor[2]>=800)
	LCD_ShowIntNum(50,56,Send_Data.Motor[2],4,RED,WHITE,16);
	else
	LCD_ShowIntNum(50,56,Send_Data.Motor[2],4,BLUE,WHITE,16);
	
	if(Send_Data.Motor[3]<=400)//电机4参数
	LCD_ShowIntNum(50,80,Send_Data.Motor[3],4,GBLUE,WHITE,16);
	else if(Send_Data.Motor[3]>=800)
	LCD_ShowIntNum(50,80,Send_Data.Motor[3],4,RED,WHITE,16);
	else
	LCD_ShowIntNum(50,80,Send_Data.Motor[3],4,BLUE,WHITE,16);
	
	if(Send_Data.B_Motor_Control<=400)//无刷电机参数
	LCD_ShowIntNum(58,102,Send_Data.B_Motor_Control,4,GBLUE,WHITE,16);
	else if(Send_Data.B_Motor_Control>=800)
	LCD_ShowIntNum(58,102,Send_Data.B_Motor_Control,4,RED,WHITE,16);
	else
	LCD_ShowIntNum(58,102,Send_Data.B_Motor_Control,4,BLUE,WHITE,16);
	
	if(Send_Data.Motor_State&0X01)//电机状态
	LCD_ShowChar(40,8,'B',BRRED,WHITE,16,0);
	else
	LCD_ShowChar(40,8,'A',BRRED,WHITE,16,0);
	
	if(Send_Data.Motor_State&0X02)//电机2状态
	LCD_ShowChar(40,32,'B',BRRED,WHITE,16,0);
	else
	LCD_ShowChar(40,32,'A',BRRED,WHITE,16,0);
	
	if(Send_Data.Motor_State&0X04)//电机3状态
	LCD_ShowChar(40,56,'B',BRRED,WHITE,16,0);
	else
	LCD_ShowChar(40,56,'A',BRRED,WHITE,16,0);
	
	if(Send_Data.Motor_State&0X08)//电机4状态
	LCD_ShowChar(40,80,'B',BRRED,WHITE,16,0);
	else
	LCD_ShowChar(40,80,'A',BRRED,WHITE,16,0);
	
	if(Motor_State==0)//无刷电机状态
	LCD_ShowString(40,102,"OF",RED,WHITE,16,0);
	else
	LCD_ShowString(40,102,"ON",GREEN,WHITE,16,0);
}

/** @brief	LCD显示舵机参数
  **/
void LCD_Show_Servo_Data(void)
{
	LCD_ShowString(95,30,"Servo",BLACK,WHITE,12,0);
	LCD_ShowString(95,40,"Control",BLACK,WHITE,12,0);
	
	LCD_ShowString(6,8,"S_1:",BLACK,WHITE,16,0);
	LCD_ShowString(6,32,"S_2:",BLACK,WHITE,16,0);
	LCD_ShowString(6,56,"S_3:",BLACK,WHITE,16,0);
	LCD_ShowString(6,80,"S_4:",BLACK,WHITE,16,0);
	LCD_ShowString(6,102,"            ",WHITE,WHITE,16,0);
	
	
	LCD_ShowIntNum(50,8,Send_Data.Servo[0],4,BLUE,WHITE,16);
	LCD_ShowIntNum(50,32,Send_Data.Servo[1],4,BLUE,WHITE,16);
	LCD_ShowIntNum(50,56,Send_Data.Servo[2],4,BLUE,WHITE,16);
	LCD_ShowIntNum(50,80,Send_Data.Servo[3],4,BLUE,WHITE,16);
	
}




