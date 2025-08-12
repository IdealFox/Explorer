#include "stm32f10x.h"                  // Device header
#include "Servo.h"


//舵机驱动程序
//PWM输出定时器TIM2
/*引脚定义		无重映像
	
		CH1					PA0
		CH2					PA1
		CH3					PA2
		CH4					PA3
*/

/** @brief	GPIO初始化	PA0,PA1,PA2,PA3
  * @param	无
  * @retval	无
  **/
void Servo_GPIO_Out_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);			//开启GPIOA时钟
	
	//GPIO初始化结构体
	GPIO_InitTypeDef	GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_AF_PP;															//复用推挽输出，由TIM2控制
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;	//0,1,2,3端口
	GPIO_InitStruct.GPIO_Speed	=GPIO_Speed_50MHz;														//速度50MHz
	GPIO_Init(GPIOA,&GPIO_InitStruct);																				//初始化GPIOA
}

/** @brief	TIM2初始化		100Hz
  * @param	无
  * @retval	无
  **/
void Timer2_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);			//开启TIM2时钟
	
	//时基单元初始化结构体
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision			=TIM_CKD_DIV1;					//1分频
	TIM_TimeBaseInitStruct.TIM_CounterMode				=TIM_CounterMode_Up;		//向上计数
	TIM_TimeBaseInitStruct.TIM_Period							=10000;									//ARR重装寄存器，10ms计时，100Hz
	TIM_TimeBaseInitStruct.TIM_Prescaler					=72-1;									//PSC预分频器，72分频
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter	=0;											//重复计数器关
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);												//初始化TIM2
}

/** @brief	PWM输出初始化
	* @param	Angle1		舵机1初始角度		0.1°精度 范围0~1800
	* @param	Angle2		舵机2初始角度		0.1°精度 范围0~1800
	* @param	Angle3		舵机3初始角度		0.1°精度 范围0~1800
	* @param	Angle4		舵机4初始角度		0.1°精度 范围0~1800
  * @retval	无
  **/
void Servo_Init(u16 Angle1,u16 Angle2,u16 Angle3,u16 Angle4)
{
	u16 Duty_Cycle1,Duty_Cycle2,Duty_Cycle3,Duty_Cycle4;
	
	Servo_GPIO_Out_Init();							//GPIO初始化
	Timer2_Init();											//TIM2初始化 
	
	//角度数据转换为CCR寄存器值
	Duty_Cycle1=(Angle1*1.111)+500;
	Duty_Cycle2=(Angle2*1.111)+500;
	Duty_Cycle3=(Angle3*1.111)+500;
	Duty_Cycle4=(Angle4*1.111)+500;
	
	//输出比较结构体初始化
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCStructInit(&TIM_OCInitStruct);												//结构体内所有参数赋初始值
	TIM_OCInitStruct.TIM_OCMode				=	TIM_OCMode_PWM1;				//PWM1输出模式
	TIM_OCInitStruct.TIM_OCPolarity		=	TIM_OCPolarity_High;		//输出极性为高电平
	TIM_OCInitStruct.TIM_OutputState	=	TIM_OutputState_Enable;	//输出使能
	
	
	TIM_OCInitStruct.TIM_Pulse				=	Duty_Cycle1;						//舵机1角度设置
	TIM_OC1Init(TIM2,&TIM_OCInitStruct);												//初始化CH1通道
	
	TIM_OCInitStruct.TIM_Pulse				=	Duty_Cycle2;						//舵机2角度设置
	TIM_OC2Init(TIM2,&TIM_OCInitStruct);												//初始化CH2通道
	
	TIM_OCInitStruct.TIM_Pulse				=	Duty_Cycle3;						//舵机3角度设置	
	TIM_OC3Init(TIM2,&TIM_OCInitStruct);												//初始化CH3通道
	
	TIM_OCInitStruct.TIM_Pulse				=	Duty_Cycle4;						//舵机4角度设置
	TIM_OC4Init(TIM2,&TIM_OCInitStruct);												//初始化CH4通道
	
	TIM_Cmd(TIM2,ENABLE);			//开启TIM2
}

	
/** @brief	舵机1角度更改
  * @param	Angle 舵机旋转角度 0.1°精度 范围0~1800
  * @retval	无
  **/
void Servo1_SetAngle(u16 Angle)
{
	u16 Duty_Cycle=0;
	Duty_Cycle=(Angle*1.111)+500;							//角度数据转换为PWM占空比数据
	TIM_SetCompare1(TIM2,Duty_Cycle);					//更改CH1通道CCR捕获比较寄存器值
}

/** @brief	舵机2角度更改
  * @param	Angle 舵机旋转角度 0.1°精度 范围0~1800
  * @retval	无
  **/
void Servo2_SetAngle(u16 Angle)
{
	u16 Duty_Cycle=0;
	Duty_Cycle=(Angle*1.111)+500;							//角度数据转换为PWM占空比数据
	TIM_SetCompare2(TIM2,Duty_Cycle);					//更改CH2通道CCR捕获比较寄存器值
}

/** @brief	舵机3角度更改
  * @param	Angle 舵机旋转角度 0.1°精度 范围0~1800
  * @retval	无
  **/
void Servo3_SetAngle(u16 Angle)
{
	u16 Duty_Cycle=0;
	Duty_Cycle=(Angle*1.111)+500;							//角度数据转换为PWM占空比数据
	TIM_SetCompare3(TIM2,Duty_Cycle);					//更改CH3通道CCR捕获比较寄存器值
}

/** @brief	舵机4角度更改
  * @param	Angle 舵机旋转角度 0.1°精度 范围0~1800
  * @retval	无
  **/
void Servo4_SetAngle(u16 Angle)
{
	u16 Duty_Cycle=0;
	Duty_Cycle=(Angle*1.111)+500;							//角度数据转换为PWM占空比数据
	TIM_SetCompare4(TIM2,Duty_Cycle);					//更改CH4通道CCR捕获比较寄存器值
}

extern u16 Angle[4];







