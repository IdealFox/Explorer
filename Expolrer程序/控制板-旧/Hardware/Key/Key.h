#ifndef __Key_H
#define __Key_H

void Key_Init(void);							//按键初始化

void Key_Loop(void);							//按键循环检测，由定时器中断调用
u8 Get_KeyNum(void);							//获取键码值

#endif


