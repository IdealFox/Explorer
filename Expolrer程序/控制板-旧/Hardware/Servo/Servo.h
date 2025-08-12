#ifndef __Servo_H
#define __Servo_H

void PWM_Out_Init(u16 Angle1,u16 Angle2,u16 Angle3,u16 Angle4);		//PWM输出初始化

void PWM1_SetAngle(u16 Angle);				//舵机1角度更改
void PWM2_SetAngle(u16 Angle);				//舵机1角度更改
void PWM3_SetAngle(u16 Angle);				//舵机1角度更改
void PWM4_SetAngle(u16 Angle);				//舵机1角度更改


void AD1_PWM(u16 AD_Value);
void AD2_PWM(u16 AD_Value);
void AD3_PWM(u16 AD_Value);
void AD4_PWM(u16 AD_Value);

#endif


