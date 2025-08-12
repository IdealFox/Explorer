#ifndef __TB6612_H
#define __TB6612_H

//电机控制模式 
#define Motor_ALL_Enable  0X0300		//所有电机使能	0000 0011 00 00 00 00
#define Motor_ALL_Disable 0X0000		//所有电机失能	0000 0000 00 00 00 00
#define Motor_L_ENABLE		0X0100		//左电机使能		0000 0001 00 00 00 00
#define Motor_L_DISABLE		0X0000		//左电机失能		0000 0000 00 00 00 00
#define Motor_L_STOP			0X010F		//左电机制动		0000 0001 00 00 11 11
#define Motor_L_advance		0X0105		//左电机前进		0000 0001 00 00 01 01
#define Motor_L_back			0X010A		//左电机后退		0000 0001 00 00 10 10

#define Motor_R_ENABLE		0X0200		//右电机使能		0000 0010 00 00 00 00
#define Motor_R_DISABLE		0X0000		//右电机失能		0000 0000 00 00 00 00
#define Motor_R_STOP			0X02F0		//右电机制动		0000 0010 11 11 00 00
#define Motor_R_advance		0X0250		//右电机前进		0000 0010 01 01 00 00
#define Motor_R_back			0X02A0		//右电机后退		0000 0010 10 10 00 00

#define M1_A							0X0101		//电机1前进			0000 0001 00 00 00 01
#define M1_B							0X0102		//电机1后退			0000 0001 00 00 00 10
#define M2_A							0X0104		//电机2前进			0000 0001 00 00 01 00
#define M2_B							0X0108		//电机2后退			0000 0001 00 00 10 00
#define M3_A							0X0210		//电机3前进			0000 0010 00 01 00 00
#define M3_B							0X0220		//电机3后退			0000 0010 00 10 00 00
#define M4_A							0X0240		//电机4前进			0000 0010 01 00 00 00
#define M4_B							0X0280		//电机4后退			0000 0010 10 00 00 00

#define M_PUMP_ON					0X1400		//开启气泵			0001 0100 00 00 00 00
#define M_PUMP_OFF				0X0000		//关闭气泵			0000 0000 00 00 00 00
                                                                    
#define SER1	GPIO_WriteBit(GPIOB,GPIO_Pin_8,Bit_SET);
#define SER0	GPIO_WriteBit(GPIOB,GPIO_Pin_8,Bit_RESET);
#define RCK1	GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_SET);
#define RCK0	GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_RESET);
#define SCK1	GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_SET);
#define SCK0	GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_RESET);


void Timer1_Interrupt_Init(u8 PreemptionPriorit,u8 SubPriority);

void Motor_Control_Init(void);								//电机控制模块初始化
void Motor_Control(u16 Data);									//发送数据至74HC595,595作为输出控制电机
void Motor1_Speed_Set(u16 Duty);							//电机1速度控制函数，电机为左前方电机
void Motor2_Speed_Set(u16 Duty);							//电机2速度控制函数，电机为左后方电机
void Motor3_Speed_Set(u16 Duty);							//电机3速度控制函数，电机为右前方电机
void Motor4_Speed_Set(u16 Duty);							//电机4速度控制函数，电机为右后方电机


#endif


