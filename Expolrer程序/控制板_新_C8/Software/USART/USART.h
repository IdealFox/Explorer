#ifndef __USART_H
#define __USART_H

//串口初始化
void USART1_Init(void);																	//USART1初始化
void USART1_Interrupt_Init(u8 Priority,u8 SubPriority);	//USART1中断初始化

//数据收发
void USART1_SendByte(u8 Byte);													//USART1发送8位数据
void USART1_SendArray(u8 *Array,u16 Length);						//USART1发送数组
void USART1_SendString(u8 *String);											//USART1发送字符串
void USART1_SendNumber(u32 Number,u8 Length);						//USART1发送数字

extern u8 RX_Data[4];
//数据包收发
void USART1_SendPacket(u8 Packet_Heat,u8 Packet_Tail,u8 *Packet,u8 Length);		//USART1发送数据包
void USART1_GetPacket(u8 Packet_Heat,u8 Packet_Tail);													//USART1接收数据包

#endif


