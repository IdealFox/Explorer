/************************************************
**引脚定义
**从机				主机		模式		复用功能	
**CSN					PA4			PP								
**SCK					PA5			AF_PP		SPI1_SCK
**MISO				PA6			IPU			SPI1_MISO
**MOSI				PA7			AF_PP		SPI1_MOSI	
**CE					PB5			PP
**IRQ					PB1			IPU
************************************************/
/************************************************
**状态定义				NRF24L01_State
**接收到数据						1
**接收数据中						2
**接收完成							3
**写入发送数据中				4
**发送数据中						5
**数据发送完成					6
**数据发送失败					7
**状态异常							8
************************************************/
#include "stm32f10x.h"              // Device header
#include "NRF24L01.h"
#include "SPI.h"

u8 NRF24L01_State;									//NRF24L01状态字节


const u8 NRF34L01_TX_Address[5]={0XF0,0XF0,0XF0,0XF0,0XF0}; //发送地址
const u8 NRF34L01_RX_Address[5]={0XF0,0XF0,0XF0,0XF0,0XF0};	//接收地址

#define NRF_24L01_Channel				0X00								//通讯通道

#define NRF24L01_CS(X)		GPIO_WriteBit(GPIOA,GPIO_Pin_4,(BitAction)(X))
#define NRF24L01_CE(X)		GPIO_WriteBit(GPIOB,GPIO_Pin_5,(BitAction)(X))

/** @brief	NRF24L01模块GPIO初始化
  **/
void NRF24L01_GPIO_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	//CSN引脚初始化
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Speed	=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//IRQ引脚初始化
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_1;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	//CE引脚初始化
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_5;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_SET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_5,Bit_SET);
}

/** @brief	NRF24L01外部中断初始化		端口：PB1
  * @param	PreemptionPriority		抢占优先级
  * @param	SubPriority						响应优先级
  **/
void NRF24L01_Interrupt_Init(u8 PreemptionPriority,u8 SubPriority)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);							//使能AFIO
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource1);			//GPIO通道选择，PB1
	
	//外部中断配置
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line			=EXTI_Line1;											//通道1
	EXTI_InitStruct.EXTI_LineCmd	=ENABLE;													//使能通道
	EXTI_InitStruct.EXTI_Mode			=EXTI_Mode_Interrupt;							//中断模式
	EXTI_InitStruct.EXTI_Trigger	=EXTI_Trigger_Falling;						//下降沿触发
	EXTI_Init(&EXTI_InitStruct);
	
	//NVIC配置
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel										=EXTI1_IRQn;						//外部中断通道1
	NVIC_InitStruct.NVIC_IRQChannelCmd								=ENABLE;								//使能中断
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	=PreemptionPriority;		//抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority				=SubPriority;						//响应优先级
	NVIC_Init(&NVIC_InitStruct);
}

/** @brief	NRF24L01写指令
  * @param	Reg				指令
  * @param	Byta			数据
  **/
void NRF24L01_W_Data(u8 Reg,u8 Byta)
{
	NRF24L01_CS(0);
	SPI1_SwapByte(Reg);
	SPI1_SwapByte(Byta);
	NRF24L01_CS(1);
}

/** @brief	NRF24L01读指令
  * @param	Reg				指令
  * @param	Byta			数据
  * @retval	读到的数据
  **/
u8 NRF24L01_R_Data(u8 Reg)
{
	u8 Data;
	NRF24L01_CS(0);
	SPI1_SwapByte(Reg);
	Data=SPI1_SwapByte(0XFF);
	NRF24L01_CS(1);
	return Data;
}

/** @brief	NRF24L01写多字节
  * @param	Reg				指令
  * @param	Byta			数据地址
  * @param	Length		数据长度
	* @warn		数据长度不得超过数据本身长度
  **/
void NRF24L01_W_Data_More(u8 Reg,u8 *Byte,u8 Length)
{
	NRF24L01_CS(0);
	SPI1_SwapByte(Reg);
	for(u8 i=0;i<Length;i++)
	{	SPI1_SwapByte(Byte[i]);}
	NRF24L01_CS(1);
}

/** @brief	NRF24L01读多字节
  * @param	Reg				指令
  * @param	Byta			数据存放地址
  * @param	Length		读取数据长度
	* @warn		数据长度不得超过数据本身长度
  **/
void NRF24L01_R_Data_More(u8 Reg,u8 *Byte,u8 Length)
{
	NRF24L01_CS(0);
	SPI1_SwapByte(Reg);
	for(u8 i=0;i<Length;i++)
	{	Byte[i]=SPI1_SwapByte(0XFF);}
	NRF24L01_CS(1);
}

/*	本源文件内调用函数
**	void NRF24L01_GPIO_Init(void)						NRF24L01模块GPIO初始化
**	void NRF24L01_W_Data(u8 Reg,u8 Byta)		NRF24L01写单字节
**	u8 NRF24L01_R_Data(u8 Reg)							NRF24L01读单字节
**	void NRF24L01_W_Data_More(u8 Reg,u8 *Byte,u8 Length)				NRF24L01写多字节
**	void NRF24L01_R_Data_More(u8 Reg,u8 *Byte,u8 Length)				NRF24L01读多字节
*/

/** @brief	NRF24L01读STATUS寄存器
  * @retval	读到的值
  **/
u8 NRF24L01_Read_STATUS(void)
{	
	u8 Byte;
	NRF24L01_CS(0);
	Byte=SPI1_SwapByte(0XFF);
	NRF24L01_CS(1);
	return Byte;
}

/** @brief	NRF24L01模块初始化,数据接收
	* @retval	0:初始化成功		1:初始化失败
  **/
u8 NRF24L01_Init(void)
{	
	u8 Test_Addr,Status;
	NRF24L01_GPIO_Init();															//GPIO初始化
	
	Status=NRF24L01_Read_STATUS();										//获取STATUS参数
	NRF24L01_W_Data(W_REGISTER|STATUS,Status);				//清除STATUS标志位
	
	NRF24L01_CE(0);																		//待机模式
	
	NRF24L01_W_Data(W_REGISTER|RF_CH,NRF_24L01_Channel);										//配置通讯通道(频率)
	NRF24L01_W_Data_More(W_REGISTER|TX_ADDR,(u8*)NRF34L01_TX_Address,5);		//配置发送地址
	NRF24L01_W_Data_More(W_REGISTER|RX_ADDR_P0,(u8*)NRF34L01_RX_Address,5);	//配置接收地址，通道0

	NRF24L01_W_Data(W_REGISTER|CONFIG,0X0F);					//配置寄存器，接收模式
	NRF24L01_W_Data(W_REGISTER|EN_AA,0X01);						//通道0开启自动应答
	NRF24L01_W_Data(W_REGISTER|RX_PW_P0,32);					//数据包长度，32字节	
	NRF24L01_W_Data(W_REGISTER|EN_RXADDR,0X01);		 		//开启接收通道0
	NRF24L01_W_Data(W_REGISTER|SETUP_RETR,0X1A);			//配置自动重发,间隔500us,10次
	NRF24L01_W_Data(W_REGISTER|RF_STEUP,0X07);				//射频功率
	NRF24L01_W_Data(FLUSH_RX,0XFF);										//清除接收数据缓存器
	NRF24L01_W_Data(FLUSH_TX,0XFF);										//清除发射数据缓存器
	
	NRF24L01_CE(1);																		//取消待机
	
	Test_Addr=NRF24L01_R_Data(RF_CH);									//获取NRF24L01通道参数
	if((Test_Addr&0X8F)!=NRF_24L01_Channel)						//验证初始化是否成功
	{	return 1;}
	
	return 0;
}


/** @brief	NRF24L01接收数据
  * @param	Data		接收数据存放地址
	* @retval	0:数据接收成功		1:数据接收失败
	* @warn		数据存放地址长度需为32字节
  **/
void NRF24L01_Receive_Data(u8 *Data)
{
	NRF24L01_State=2;																			//接收数据中
	NRF24L01_R_Data_More(R_RX_PAYLOAD,Data,32);						//接收数据
	NRF24L01_W_Data(FLUSH_RX,0XFF);												//清除数据缓存器
	NRF24L01_State=3;																			//接收完成
}

/** @brief	NRF24L01发送数据
  * @param	Data		发送数据存放地址
  * @retval	
	* @warn		数据存放地址长度需为32字节
  **/
void NRF24L01_Send_Data(u8 *Data)
{ 
	NRF24L01_State=4;																			//写入发送数据中
	NRF24L01_W_Data_More(W_TX_PAYLOAD,Data,32);						//写入发送数据
	NRF24L01_CE(0);	
	NRF24L01_W_Data(W_REGISTER|CONFIG,0X0E);							//切换状态为发送模式
	NRF24L01_CE(1);
	NRF24L01_State=5;																			//发送数据中
}

/** @brief	发送完成操作
  **/
void NRF24L01_Send_OK(void)
{
	NRF24L01_W_Data(FLUSH_TX,0XFF);						//清除发送数据缓存器
	NRF24L01_CE(0);	
	NRF24L01_W_Data(W_REGISTER|CONFIG,0X0F);	//切换状态为接收模式
	NRF24L01_CE(1);
	NRF24L01_State=6;													//数据发送完成
}

/** @brief	发送失败操作
  **/
void NRF24L01_Send_Error(void)
{
	NRF24L01_W_Data(FLUSH_TX,0XFF);						//清除发送数据缓存器
	NRF24L01_CE(0);	
	NRF24L01_W_Data(W_REGISTER|CONFIG,0X0F);	//切换状态为接收模式
	NRF24L01_CE(1);
	NRF24L01_State=7;													//数据发送失败
}


/*		中断函数
{	
	EXTI_ClearITPendingBit(EXTI_Line1);								//清除中断标志位
	u8 Status,State;
	Status=NRF24L01_Read_STATUS();										//获取STATUS参数
	NRF24L01_W_Data(W_REGISTER|STATUS,Status);				//清除STATUS标志位
	
	if(Status==0XFF||Status==0X00){	State=0;}					//状态0，出现异常
	else if(Status&RX_DR){	State=1;NRF24L01_State=1;}//状态1，接收完成中断
	else if(Status&TX_DS){	State=2;}									//状态2，发送完成中断
	else if(Status&MAX_RT){	State=3;}									//状态3，最大重发中断
	
	
	switch(State)																			//执行各状态对应指令
	{	
		case 0:	NRF24L01_State=8;												//状态异常
						break;
		case 1:	NRF24L01_Receive_Data(NRF34L01_RX_Data);//接收数据
						break;
		case 2: NRF24L01_Send_OK();											//发送完成操作
						break;
		case 3:	NRF24L01_Send_Error();									//发送失败操作
						break;
	}
}
*/


