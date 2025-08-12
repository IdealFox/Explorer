#ifndef __NRF24L01_H
#define __NRF24L01_H

//寄存器地址
#define CONFIG				0x00						//配置寄存器
#define EN_AA					0X01						//自动应答使能
#define EN_RXADDR			0X02						//接收使能通道
#define	SETUP_RETR		0X04						//设置自动重发
#define RF_CH					0X05						//通讯通道(频率)配置
#define RF_STEUP			0X06						//射频寄存器
#define STATUS				0X07						//状态寄存器
#define RX_ADDR_P0		0X0A						//接收地址寄存器0
#define RX_ADDR_P1		0x0B						//接收地址寄存器1
#define RX_ADDR_P2    0X0C						//接收地址寄存器2
#define RX_ADDR_P3    0X0D						//接收地址寄存器3
#define RX_ADDR_P4    0X0E						//接收地址寄存器4
#define RX_ADDR_P5    0X0F						//接收地址寄存器5
#define TX_ADDR       0X10						//发送地址寄存器
#define RX_PW_P0      0X11						//数据通道0有效数据宽度寄存器
#define RX_PW_P1			0x12						//数据通道1有效数据宽度寄存器
#define RX_PW_P2      0X13						//数据通道2有效数据宽度寄存器
#define RX_PW_P3      0X14						//数据通道3有效数据宽度寄存器
#define RX_PW_P4      0X15						//数据通道4有效数据宽度寄存器
#define RX_PW_P5      0X16						//数据通道5有效数据宽度寄存器
#define FIFO_STATUS   0X17						//FIFO状态寄存器

//操作指令
#define R_REGISTER 		0X00						//读指定寄存器，低5位为寄存器地址
#define	W_REGISTER		0X20						//写指定寄存器，低5位为寄存器地址
#define R_RX_PAYLOAD	0X61						//读接收寄存器
#define W_TX_PAYLOAD	0XA0						//写发送寄存器
#define FLUSH_TX			0XE1						//清除TX_FIFO寄存器
#define FLUSH_RX			0XE2						//清除RX_FIFO寄存器

//状态寄存器状态判断
#define RX_DR					0X40						//接收数据中断位
#define	TX_DS					0X20						//数据发送完成中断
#define	MAX_RT				0X10						//达到最多次重发中断

extern u8 NRF24L01_State;							//NRF24L01状态字节

void NRF24L01_W_Data(u8 Reg,u8 Byta);	//NRF24L01写指令
u8 NRF24L01_R_Data(u8 Reg);						//NRF24L01读指令

u8 NRF24L01_Read_STATUS(void);				//NRF24L01读STATUS寄存器
u8 NRF24L01_Init(void);								//NRF24L01模块初始化
void NRF24L01_Interrupt_Init(u8 PreemptionPriority,u8 SubPriority);		//NRF24L01外部中断初始化

void NRF24L01_Receive_Data(u8 *Data);	//NRF24L01接收数据
void NRF24L01_Send_Data(u8 *Data);		//NRF24L01发送数据
void NRF24L01_Send_OK(void);					//发送完成操作
void NRF24L01_Send_Error(void);				//发送失败操作


#endif


