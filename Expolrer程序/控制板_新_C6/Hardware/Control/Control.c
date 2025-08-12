/*
本函数提供STM32内部ADC获取的值处理电机与舵机控制参数
并通过NRF24L01发送数据
数据包长度：32字节

数据包格式
位					变量名						执行操作									取值范围
0				Control_Select				控制选择							0：电机控制							1：舵机控制
1					Motor_State					电机正反转						0~3位代表电机1~4状态		0正转		1反转

2~3				Motor[0]						电机1参数									0~1000
4~5				Motor[1]						电机2参数									0~1000
6~7				Motor[2]						电机3参数									0~1000
8~9				Motor[3]						电机4参数	   							0~1000

10~11			Servo[0]						舵机1参数									900+-200
12~13			Servo[1]						舵机2参数									100~1800
14~15			Servo[2]						舵机3参数									400~1800
16~17			Servo[3]						舵机4参数										0~300

18~19		B_Motor_Control			无刷电机参数								0~1000							为0，电机停止

21				Bump_State					气泵控制								0：停止		1：启用

20				verify							数据校验
*/

#include "stm32f10x.h"                  // Device header
#include "Control.h"
#include "ADC.h"
#include "LED.h"
#include "Key.h"

struct NRF24L01_Send_Data Send_Data;	//发送文件
u8 Control_State=0;										//控制选择状态字节		0：电机控制		1：舵机控制
u8 Motor_State=0;											//无刷电机状态字节		0：停止				1：运行

u8 Motor_Space=0;											//电机区间						0~12
u16 Motor_Speed[3];										//横向,纵向,旋转速度	0~1000
u8 Motor_Turn_State=0;								//旋转状态						0:无旋转	1:右转	2:左转

/*
摇杆位置						Motor_Space
死区内									0
	轴1										1
	轴2										2
	轴3										3
	轴4										4
	区间1									5
	区间2									6
	区间3									7
	区间4									8
	区间5									9
	区间6									10
	区间7									11
	区间8									12
*/

/** @brief	数据防溢出处理		锁定范围0~1000
	* @brief	若未溢出，返回原值
  * @param	Input		输入数据
  * @retval	OutPut	输入数据
  **/
u16 Data_Overflow_Control(u16 Input)
{	
	u16 OutPut;
	if(Input>=1000)				OutPut=1000;
	else if(Input>=10000)	OutPut=0;
	else									OutPut=Input;
	return OutPut;
}


/** @brief	电机原地正反转控制	
  * @param	AD_Value							AD值
  **/
void Motor_Stop_Turn_Set(u16 AD_Value)
{
	if(AD_Value>=2300)
	{	Send_Data.Motor_State=0X0C;}		//电机1,2正转,3,4反转
	else if (AD_Value<=1800)
	{	Send_Data.Motor_State=0X03;}		//电机1,2反转,3,4正转
}

/** @brief	电机同速数据处理
	* @param	AD_Value			AD数据
  * @retval	处理后的数据	范围0~1000
  **/
u16 Motor_Union_Convert(u16 AD_Value)
{	
	u16 Data;
	float Convert;
	if(AD_Value>=2300)
	{	
		AD_Value-=2300;
		Convert=(float)AD_Value/1500;
		Data=1000*Convert;
		return Data_Overflow_Control(Data);
	}
	if(AD_Value<=1800)
	{	
		AD_Value-=300;
		if(AD_Value>=10000)AD_Value=0;		//防反向溢出
		Convert=(float)AD_Value/1500;
		Convert=1-Convert;
		Data=1000*Convert;
		return Data_Overflow_Control(Data);
	}
	return 0;
}


/** @brief	电机旋转双速控制
  * @param	Speed1		加速值		0→1000↑
  * @param	Speed2		减速值		0→1000↓
  * @retval	Out_Speed	最终速度	0~1000
  **/
u16 Motor_Trun_Speed_Control(u16 Speed1,u16 Speed2)
{
	u16 Out_Speed=Speed1;
	
	Out_Speed-=(Speed1*((float)Speed2/1000));
	
	return Out_Speed;
}

/** @brief	电机区间平移双速控制
  * @param	Speed1		加速值		0→1000↑
  * @param	Speed2		减速值		0→1000↓
  * @retval	Out_Speed	最终速度	0~1000
  **/
u16 Motor_Space_Speed_Control(u16 Speed1,u16 Speed2)
{
	Speed1-=Speed2;
	
	return Speed1;
}
/*
											 轴1
												↑        			
					↖			  		|4096        ↗
						↖					|          ↗
							↖	      |        ↗
								↖  区间1|区间2 ↗
									↖		|    ↗
						区间8		↖	|  ↗		区间3
											↖|↗
 轴4←———————————————————*————————————————————→轴2
		0									↙|↘								4096
						区间7		↙	|  ↘		区间4
									↙		|    ↘
								↙  区间6|区间5  ↘
							↙				|        ↘
						↙					| 				 ↘
					↙						|0  				 ↘
												↓              
											 轴3
*/

/** @brief	电机区间设置
  **/
void Motor_Space_Set(void)
{
	//死区
	if(AD_Final_Value[0]>=1800&&AD_Final_Value[0]<=2300&&
		 AD_Final_Value[1]>=1800&&AD_Final_Value[1]<=2300)
	{Motor_Space=0;return;}//死区
		
	
	//轴
	if(AD_Final_Value[0]>=1800&&AD_Final_Value[0]<=2300&&
		 AD_Final_Value[1]>=2300)
	{Motor_Space=1;return;}//轴1
	
	if(AD_Final_Value[0]>=1800&&AD_Final_Value[0]<=2300&&
		AD_Final_Value[1]<=1800)
	{Motor_Space=3;return;}//轴3
	
	if(AD_Final_Value[0]>=2300&&
		 AD_Final_Value[1]>=1800&&AD_Final_Value[1]<=2300)
	{Motor_Space=2;return;}//轴2
	
	if(AD_Final_Value[0]<=1800&&
		 AD_Final_Value[1]>=1800&&AD_Final_Value[1]<=2300)
	{Motor_Space=4;return;}//轴4
	
	
	//区间
	//第一象限
	if(AD_Final_Value[0]>=2300&&AD_Final_Value[1]>=2300)
	{
		if(AD_Final_Value[0]<=AD_Final_Value[1])
			Motor_Space=6;									//区间2
		else if(AD_Final_Value[0]>=AD_Final_Value[1])
			Motor_Space=7;									//区间3
	return;
	}
	
	//第三象限
	if(AD_Final_Value[0]<=1800&&AD_Final_Value[1]<=1800)
	{
		if(AD_Final_Value[0]<=AD_Final_Value[1])
			Motor_Space=11;									//区间7
		else if(AD_Final_Value[0]>=AD_Final_Value[1])
			Motor_Space=10;									//区间6
	return;
	}
	
	//第二象限
	if(AD_Final_Value[0]<=1800&&AD_Final_Value[1]>=2300)
	{
		if((-AD_Final_Value[0]+1800)<=AD_Final_Value[1]-2300)
			Motor_Space=5;									//区间1
		else
			Motor_Space=12;									//区间8
	return;
	}
	
	//第四象限
	if(AD_Final_Value[0]>=2300&&AD_Final_Value[1]<=1800)
	{
		if(AD_Final_Value[0]-2300>=(-AD_Final_Value[1]+1800))
			Motor_Space=8;									//区间4
		else
			Motor_Space=9;									//区间5
	return;
	}
}

/** @brief	电机速度设置
  **/
void Motor_Speed_Set(void)
{
	float Convert;
	
	for(u8 i=0;i<3;i++)
	{
		Motor_Speed[i]=AD_Final_Value[i];
		if(Motor_Speed[i]>=2300)
		{	
			Motor_Speed[i]-=2300;
			Convert=(float)Motor_Speed[i]/1500;
			Motor_Speed[i]=1000*Convert;
			Motor_Speed[i]=Data_Overflow_Control(Motor_Speed[i]);
		}
		else if(Motor_Speed[i]<=1800)
		{	
			Motor_Speed[i]-=300;
			if(Motor_Speed[i]>=10000)Motor_Speed[i]=0;			//防反向溢出
			Convert=(float)Motor_Speed[i]/1500;
			Convert=1-Convert;
			Motor_Speed[i]=1000*Convert;
			Motor_Speed[i]=Data_Overflow_Control(Motor_Speed[i]);
		}
		else
			Motor_Speed[i]=0;
	}
}

/** @brief	电机旋转状态设置
  **/
void Motor_Turn_Set(void)
{
	if(AD_Final_Value[2]>=2300)
		Motor_Turn_State=1;						//右转
	else if(AD_Final_Value[2]<=1800)
		Motor_Turn_State=2;						//左转
	else
		Motor_Turn_State=0;						//无旋转
}

/** @brief	电机Y轴运动控制
  **/
void Motor_Y_Move_Control(void)
{
	if(Motor_Turn_State==1)								//右转
	{
		Send_Data.Motor[0]=Motor_Speed[1];
		Send_Data.Motor[1]=Motor_Speed[1];
		Send_Data.Motor[2]=Motor_Trun_Speed_Control(Motor_Speed[1],Motor_Speed[2]);
		Send_Data.Motor[3]=Motor_Trun_Speed_Control(Motor_Speed[1],Motor_Speed[2]);
	}
	else if(Motor_Turn_State==2)					//左转
	{
		Send_Data.Motor[0]=Motor_Trun_Speed_Control(Motor_Speed[1],Motor_Speed[2]);
		Send_Data.Motor[1]=Motor_Trun_Speed_Control(Motor_Speed[1],Motor_Speed[2]);
		Send_Data.Motor[2]=Motor_Speed[1];
		Send_Data.Motor[3]=Motor_Speed[1];
	}
	else																	//无旋转转
	{
		Send_Data.Motor[0]=Motor_Speed[1];
		Send_Data.Motor[1]=Motor_Speed[1];
		Send_Data.Motor[2]=Motor_Speed[1];
		Send_Data.Motor[3]=Motor_Speed[1];
	}
}

/** @brief	电机右移运动控制
  **/
void Motor_X_Move_Control(void)
{
	if(Motor_Turn_State==1)								//右转
	{
		Send_Data.Motor[0]=Motor_Trun_Speed_Control(Motor_Speed[0],Motor_Speed[2]);
		Send_Data.Motor[1]=Motor_Speed[0];
		Send_Data.Motor[2]=Motor_Trun_Speed_Control(Motor_Speed[0],Motor_Speed[2]);
		Send_Data.Motor[3]=Motor_Speed[0];
	}
	else if(Motor_Turn_State==2)					//左转
	{
		Send_Data.Motor[0]=Motor_Speed[0];
		Send_Data.Motor[1]=Motor_Trun_Speed_Control(Motor_Speed[0],Motor_Speed[2]);
		Send_Data.Motor[2]=Motor_Speed[0];
		Send_Data.Motor[3]=Motor_Trun_Speed_Control(Motor_Speed[0],Motor_Speed[2]);
	}
	else																	//无旋转转
	{
		Send_Data.Motor[0]=Motor_Speed[0];
		Send_Data.Motor[1]=Motor_Speed[0];
		Send_Data.Motor[2]=Motor_Speed[0];
		Send_Data.Motor[3]=Motor_Speed[0];
	}
}

/** @brief	电机区间移动控制
		区1458调速控制为1001				区2367调速控制为0110		(0:保持原速1:调速)
		区1256调速方式为MP1-MP0			区3478调速方式为MP0-MP1
* @param	Space		区间选择
  **/
void Motor_Space_Move_Control(u8 Space)
{
	if(Space==1||Space==4||Space==5||Space==8)//1001调速
	{
		if(Space==1||Space==5)			//区15调速控制						//MP1-MP0调速
		{	Send_Data.Motor[0]=Motor_Space_Speed_Control(Motor_Speed[1],Motor_Speed[0]);
			Send_Data.Motor[3]=Motor_Space_Speed_Control(Motor_Speed[1],Motor_Speed[0]);
		}
		else if(Space==4||Space==8)	//区48调速控制						//MP0-MP1调速
		{	Send_Data.Motor[0]=Motor_Space_Speed_Control(Motor_Speed[0],Motor_Speed[1]);
			Send_Data.Motor[3]=Motor_Space_Speed_Control(Motor_Speed[0],Motor_Speed[1]);
		}
	}
	else																			//0110调速
	{
		if(Space==2||Space==6)//区26										//MP1-MP0调速
		{	Send_Data.Motor[0]=Motor_Speed[1];
			Send_Data.Motor[1]=Motor_Space_Speed_Control(Motor_Speed[1],Motor_Speed[0]);
			Send_Data.Motor[2]=Motor_Space_Speed_Control(Motor_Speed[1],Motor_Speed[0]);
			Send_Data.Motor[3]=Motor_Speed[1];
		}
		else									//区37										//MP0-MP1调速
		{	Send_Data.Motor[0]=Motor_Speed[0];
			Send_Data.Motor[1]=Motor_Space_Speed_Control(Motor_Speed[0],Motor_Speed[1]);
			Send_Data.Motor[2]=Motor_Space_Speed_Control(Motor_Speed[0],Motor_Speed[1]);
			Send_Data.Motor[3]=Motor_Speed[0];
		}
	}
}

/** @brief	电机区间旋转控制
		区1256原速跟随MP1						区3478原速跟随MP0
		区18旋转控制为左0100右0010	区23旋转控制为左1000右0001
		区45旋转控制为左0010右0100	区67旋转控制为左0001右1000
  * @param	Space		区间选择
  **/
void Motor_Space_Turn_Control(u8 Space)
{
	if(Motor_Turn_State==1)			//右转状态
	{
		if(Space==1||Space==2||Space==5||Space==6)//区1256		原速跟随MP1
		{
			switch(Space)
			{
				case 1:	Send_Data.Motor[1]=Motor_Speed[1];
								Send_Data.Motor[2]=Motor_Trun_Speed_Control(Motor_Speed[1],Motor_Speed[2]);
								break;//0010
				case 2:	Send_Data.Motor[0]=Motor_Speed[1];
								Send_Data.Motor[3]=Motor_Trun_Speed_Control(Motor_Speed[1],Motor_Speed[2]);
								break;//0001
				case 5:	Send_Data.Motor[1]=Motor_Trun_Speed_Control(Motor_Speed[1],Motor_Speed[2]);
								Send_Data.Motor[2]=Motor_Speed[1];
								break;//0100
				case 6:	Send_Data.Motor[0]=Motor_Trun_Speed_Control(Motor_Speed[1],Motor_Speed[2]);
								Send_Data.Motor[3]=Motor_Speed[1];
								break;//1000
			}
		}
		else 																			//区3478		原速跟随MP0
		{
			switch(Space)
			{
				case 8:	Send_Data.Motor[1]=Motor_Speed[0];
								Send_Data.Motor[2]=Motor_Trun_Speed_Control(Motor_Speed[0],Motor_Speed[2]);
								break;//0010
				case 3:	Send_Data.Motor[0]=Motor_Speed[0];
								Send_Data.Motor[3]=Motor_Trun_Speed_Control(Motor_Speed[0],Motor_Speed[2]);
								break;//0001
				case 4:	Send_Data.Motor[1]=Motor_Trun_Speed_Control(Motor_Speed[0],Motor_Speed[2]);
								Send_Data.Motor[2]=Motor_Speed[0];
								break;//0100
				case 7:	Send_Data.Motor[0]=Motor_Trun_Speed_Control(Motor_Speed[0],Motor_Speed[2]);
								Send_Data.Motor[3]=Motor_Speed[0];
								break;//1000
			}
		}
	}
	else if(Motor_Turn_State==2)//左转状态
	{
		if(Space==1||Space==2||Space==5||Space==6)//区1256		原速跟随MP1
		{
			switch(Space)
			{
				case 1:	Send_Data.Motor[1]=Motor_Trun_Speed_Control(Motor_Speed[1],Motor_Speed[2]);
								Send_Data.Motor[2]=Motor_Speed[1];
								break;//0100
				case 2:	Send_Data.Motor[0]=Motor_Trun_Speed_Control(Motor_Speed[1],Motor_Speed[2]);
								Send_Data.Motor[3]=Motor_Speed[1];
								break;//1000
				case 5:	Send_Data.Motor[1]=Motor_Speed[1];
								Send_Data.Motor[2]=Motor_Trun_Speed_Control(Motor_Speed[1],Motor_Speed[2]);
								break;//0010
				case 6:	Send_Data.Motor[0]=Motor_Speed[1];
								Send_Data.Motor[3]=Motor_Trun_Speed_Control(Motor_Speed[1],Motor_Speed[2]);
								break;//0001
			}
		}
		else 																			//区3478		原速跟随MP0
		{
			switch(Space)
			{
				case 8:	Send_Data.Motor[1]=Motor_Trun_Speed_Control(Motor_Speed[0],Motor_Speed[2]);
								Send_Data.Motor[2]=Motor_Speed[0];
								break;//0100
				case 3:	Send_Data.Motor[0]=Motor_Trun_Speed_Control(Motor_Speed[0],Motor_Speed[2]);
								Send_Data.Motor[3]=Motor_Speed[0];
								break;//1000
				case 4:	Send_Data.Motor[1]=Motor_Speed[0];
								Send_Data.Motor[2]=Motor_Trun_Speed_Control(Motor_Speed[0],Motor_Speed[2]);
								break;//0010
				case 7:	Send_Data.Motor[0]=Motor_Speed[0];
								Send_Data.Motor[3]=Motor_Trun_Speed_Control(Motor_Speed[0],Motor_Speed[2]);
								break;//0001
			}
		}
	}
	else												//无旋转
	{
		if(Space==1||Space==2||Space==5||Space==6)//区1256		原速跟随MP1
		{
			if(Space==1||Space==5)//区15
			{	Send_Data.Motor[1]=Motor_Speed[1];
				Send_Data.Motor[2]=Motor_Speed[1];
			}
			else									//区26
			{	Send_Data.Motor[0]=Motor_Speed[1];
			  Send_Data.Motor[3]=Motor_Speed[1];
			}
		}
		else 																			//区3478		原速跟随MP0
		{	
			if(Space==4||Space==8)//区48
			{	Send_Data.Motor[1]=Motor_Speed[0];
				Send_Data.Motor[2]=Motor_Speed[0];
			}
			else									//区37
			{	Send_Data.Motor[0]=Motor_Speed[0];
			  Send_Data.Motor[3]=Motor_Speed[0];
			}
		}
	}
}

/** @brief	电机控制数据处理
  **/
void Motor_Control(void)
{
	Send_Data.Control_Select=0;			//电机控制
	Send_Data.Servo[0]=900;					//舵机1参数复位
	
	Motor_Space_Set();							//电机移动区间设置
	Motor_Speed_Set();							//电机运行速度设置
	Motor_Turn_Set();								//电机旋转状态设置 
	
	//各区间控制
	switch(Motor_Space)
	{
		//死区(原地旋转控制)
		case 0:	Motor_Stop_Turn_Set(AD_Final_Value[2]);
						for(u8 i=0;i<4;i++)Send_Data.Motor[i]=Motor_Speed[2];
						break;
		//轴(横向移动控制)
		case 1:	Send_Data.Motor_State=0X00;		//前进
						Motor_Y_Move_Control();
						break;
		case 2:	Send_Data.Motor_State=0X06;		//右移
						Motor_X_Move_Control();
						break;
		case 3:	Send_Data.Motor_State=0X0F;		//后退
						Motor_Y_Move_Control();
						break;
		case 4:	Send_Data.Motor_State=0X09;		//左移
						Motor_X_Move_Control();
						break;
		//区间(全向移动控制)
		case 5:	Send_Data.Motor_State=0X00;
						Motor_Space_Move_Control(1);
						Motor_Space_Turn_Control(1);
						break;
		case 6:	Send_Data.Motor_State=0X00;
						Motor_Space_Move_Control(2);
						Motor_Space_Turn_Control(2);
						break;
		case 7:	Send_Data.Motor_State=0X06;
						Motor_Space_Move_Control(3);
						Motor_Space_Turn_Control(3);
						break;
		case 8:	Send_Data.Motor_State=0X06;
						Motor_Space_Move_Control(4);
						Motor_Space_Turn_Control(4);
						break;
		case 9:	Send_Data.Motor_State=0X0F;
						Motor_Space_Move_Control(5);
						Motor_Space_Turn_Control(5);
						break;
		case 10:Send_Data.Motor_State=0X0F;
						Motor_Space_Move_Control(6);
						Motor_Space_Turn_Control(6);
						break;
		case 11:Send_Data.Motor_State=0X09;
						Motor_Space_Move_Control(7);
						Motor_Space_Turn_Control(7);
						break;
		case 12:Send_Data.Motor_State=0X09;
						Motor_Space_Move_Control(8);
						Motor_Space_Turn_Control(8);
						break;
	}
	
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/** @brief	舵机1角度控制
  * @param	AD_Value				AD原始参数
	* @retval	舵机0角度数据		范围：900+-200
  **/
u16	Servo_0_Set(u16 AD_Value)
{
	u16 Data;
	float Convert;
	if(AD_Value>=2300)
	{
		AD_Value-=2300;
		Convert=(float)AD_Value/1800;
		Data=900-150*Convert;
		return Data;
	}
	else if(AD_Value<=1800)
	{
		Convert=(float)AD_Value/1800;
		Convert=1-Convert;
		Data=150*Convert+900;
		return Data;
	}
	else
	{return 900;}
}

/** @brief	舵机2角度控制
  * @param	AD_Value				AD原始参数
	* @retval	舵机0角度数据		范围：100~1800
  **/
void Servo_1_Set(u16 AD_Value)
{
	s8 Add_Value=0;
	float Convert;
	if(AD_Value>=2300)
	{
		AD_Value-=2300;
		Convert=(float)AD_Value/1800;
		Add_Value+=(50*Convert);
	}
	else if(AD_Value<=1800)
	{
		Convert=(float)AD_Value/1800;
		Convert=1-Convert;
		Add_Value-=(50*Convert);
	}
	Send_Data.Servo[1]+=Add_Value;
	
	//舵机2参数防溢出		100~1800
	if(Send_Data.Servo[1]<=100){Send_Data.Servo[1]=100;}
	if(Send_Data.Servo[1]>=1800){Send_Data.Servo[1]=1800;}
}

/** @brief	舵机3角度控制
  * @param	AD_Value				AD原始参数
	* @retval	舵机0角度数据		范围：400~1800
  **/
void Servo_2_Set(u16 AD_Value)
{
	s8 Add_Value=0;
	float Convert;
	if(AD_Value>=2300)
	{
		AD_Value-=2300;
		Convert=(float)AD_Value/1800;
		Add_Value-=(50*Convert);
	}
	else if(AD_Value<=1800)
	{
		Convert=(float)AD_Value/1800;
		Convert=1-Convert;
		Add_Value+=(50*Convert);
	}
	Send_Data.Servo[2]+=Add_Value;
	
	//舵机3参数防溢出		400~1800
	if(Send_Data.Servo[2]<=400){Send_Data.Servo[2]=400;}
	if(Send_Data.Servo[2]>=1800){Send_Data.Servo[2]=1800;}
}

/** @brief	舵机4角度控制
  * @param	AD_Value				AD原始参数
	* @retval	舵机0角度数据		范围：0~300
  **/
void Servo_3_Set(u16 AD_Value)
{
	s8 Add_Value=0;
	float Convert;
	if(AD_Value>=2300)
	{
		AD_Value-=2300;
		Convert=(float)AD_Value/1800;
		Add_Value+=(50*Convert);
	}
	else if(AD_Value<=1800)
	{
		Convert=(float)AD_Value/1800;
		Convert=1-Convert;
		Add_Value-=(50*Convert);
	}
	Send_Data.Servo[3]+=Add_Value;
	
	//舵机4参数防溢出		0~300
	if(Send_Data.Servo[3]>=10000){Send_Data.Servo[3]=0;}
	if(Send_Data.Servo[3]>=300){Send_Data.Servo[3]=300;}
}

/** @brief	舵机控制数据处理
  **/
void Servo_Control(void)
{
	Send_Data.Control_Select=1;			//舵机控制
	Send_Data.Motor[0]=0;						//电机速度参数复位
	Send_Data.Motor[1]=0;
	Send_Data.Motor[2]=0;
	Send_Data.Motor[3]=0;	
	
	Send_Data.B_Motor_Control=0;		//无刷电机复位
	
	Send_Data.Servo[0]=Servo_0_Set(AD_Final_Value[0]);
	
//	//舵机3参数防溢出		400~1800
//	if(Send_Data.Servo[1]<=100){Send_Data.Servo[1]=400;}
//	if(Send_Data.Servo[1]>=1800){Send_Data.Servo[1]=1800;}
//	//舵机4参数防溢出		0~300
//	if(Send_Data.Servo[1]<=100){Send_Data.Servo[1]=0;}
//	if(Send_Data.Servo[1]>=1800){Send_Data.Servo[1]=300;}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/** @brief	无刷电机控制
  **/
void B_Motor_Control(void)
{
	u16 Value;
	float Convert;
	
	if(Get_B_Motor_State()==1||Control_State==1)
	{	Motor_State=0;
		R_GLED_State(1);
	}
	if(Get_B_Motor_State()==0&&AD_Final_Value[4]<=20)
	{	Motor_State=1;
		R_GLED_State(0);
	}
	
	if(Motor_State==1)
	{
		Convert=(float)AD_Final_Value[4]/4096;
		Value=Convert*1000;
	}
	else
	{Value=0;}
	
	Send_Data.B_Motor_Control=Value;				//无刷电机控制
}

/** @brief	无刷电机停止
  **/
void B_Motor_Stop(void)
{
	Motor_State=0;
	
	R_GLED_State(1);
	
	Send_Data.B_Motor_Control=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/** @brief	电机控制参数复位
  **/
void Motor_Control_RESET(void)
{
	Send_Data.Motor[0]=0;
	Send_Data.Motor[1]=0;
	Send_Data.Motor[2]=0;
	Send_Data.Motor[3]=0;
}

/** @brief	气泵控制
  **/
void Air_Bump_Control()
{
	if(Get_Air_Pump_State()==1)
	{	Send_Data.Bump_State=1;
		Motor_Control_RESET();
	}
	else
		Send_Data.Bump_State=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
	
/** @brief	总控制
  **/
void Control(void)
{
	Compute_AD_Final_Value();							//指数加权移动平均滤波器算法////计算平均AD值
	Control_State=TEST_Motor_Control();		//获取控制状态
	
	switch(Control_State)
	{
		case 0:	Motor_Control();			//电机控制
						B_Motor_Control();		//无刷电机控制
						L_RLED_State(0);
						L_GLED_State(1);				
						break;
		case 1:	Servo_Control();			//舵机控制
						B_Motor_Stop();				//无刷电机停止
						L_RLED_State(1);
						L_GLED_State(0);			
						break;
	}
	Air_Bump_Control();							//气泵控制
}

