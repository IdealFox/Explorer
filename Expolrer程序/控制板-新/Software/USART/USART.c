#include "stm32f10x.h"                  // Device header
#include "USART.h"


/** @brief	USART1_GPIO初始化 ,PA9,PA10	
  **/
void USART1_GPIO_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed	=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);						//PA9,TX,复用推挽
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_10;
	GPIO_Init(GPIOA,&GPIO_InitStruct);						//PA10,RX,上拉输入
}

/** @brief	USART1初始化
  **/
void USART1_Init(void)
{
	USART1_GPIO_Init();			//USART1_GPIO初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);				//使能USART1
	
	//USART初始化结构体
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate							=115200;													//波特率
	USART_InitStruct.USART_HardwareFlowControl	=USART_HardwareFlowControl_None;	//流控制，无
	USART_InitStruct.USART_Mode									=USART_Mode_Tx|USART_Mode_Rx;			//收发模式
	USART_InitStruct.USART_Parity								=USART_Parity_No;									//无校验
	USART_InitStruct.USART_StopBits							=USART_StopBits_1;								//停止位长度，1
	USART_InitStruct.USART_WordLength						=USART_WordLength_8b;							//8位数据
	USART_Init(USART1,&USART_InitStruct);
	
	USART_Cmd(USART1,ENABLE);									//开启USART1
}

/** @brief	USART1中断初始化
	* @param	Priority			抢占优先级
	* @param	SubPriority		响应优先级
  **/
void USART1_Interrupt_Init(u8 Priority,u8 SubPriority)
{
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);				//开启USART1接收中断
	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel										=USART1_IRQn;		//NVIC通道
	NVIC_InitStruct.NVIC_IRQChannelCmd								=ENABLE;				//启用中断
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	=Priority;			//抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority				=SubPriority;		//响应优先级
	NVIC_Init(&NVIC_InitStruct);																			//初始化NVIC
}

/** @brief	USART1发送8位数据
  * @param	Byte	发送的数据，1字节，char
  **/
 void USART1_SendByte(u8 Byte)
{
	USART_SendData(USART1,Byte);																	//写入数据
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);		//等待发送标志为清零
}

/** @brief	USART1发送数组
	* @param	Arrary	数组名
	* @param	Length	数组长度
  **/
void USART1_SendArray(u8 *Array,u16 Length)
{
	for(u8 i=0;i<Length;i++)
	{
		USART1_SendByte(Array[i]);
	}
}

/** @brief	USART1发送字符串
  * @param	String	字符串
  **/
void USART1_SendString(u8 *String)
{
	for(u8 i=0;String[i]!='\0';i++)
	{	USART1_SendByte(String[i]);}		//依次发送字符，直至字符串结束
}

/** @brief	次方运算
  * @param	a	底数
	* @param	b 指数
  * @retval	幂
  **/
u32 USART1_Get_Pow(u32 a,u32 b)
{
	u32 Result = 1;
	while (b--)
	{	Result *= a;}
	return Result;
}

/** @brief	USART1发送数字
	* @param	Number	发送的数字	范围:0~4294967295
	* @param	Length	发送的数字长度
  **/
void USART1_SendNumber(u32 Number,u8 Length)
{
	for(u8 i=0;i<Length;i++)
	{
		USART1_SendByte(Number/USART1_Get_Pow(10,Length-i-1)%10+'0');
	}
}

/** @brief	USART1发送数据包
  * @param	Packet_Heat	包头数据
	* @param	Packet_Tail	包尾数据
	* @param	Packet			实际数据包		数组
	* @param	Length			数据包长度
  **/
void USART1_SendPacket(u8 Packet_Heat,u8 Packet_Tail,u8 *Packet,u8 Length)
{
	USART1_SendByte(Packet_Heat);				//发送包头数据
	for(u8 i=0;i<Length;i++)
	{	USART1_SendByte(Packet[i]);}			//发送实际数据
	USART1_SendByte(Packet_Tail);				//发送包尾数据
}

u8 RX_Data[4];

/** @brief	USART1接收数据包
	* @param	Packet_Heat	包头数据
	* @param	Packet_Tail	包尾数据
  **/
void USART1_GetPacket(u8 Packet_Heat,u8 Packet_Tail)
{
	static u8 RX_State=0,Length=0;
	//状态判断
	switch(RX_State)
	{
		case 0:
			if(USART_ReceiveData(USART1)==Packet_Heat)		//检测到包头
			{	RX_State=1;}																//进入状态1
			break;
		case 1:
			RX_Data[Length]=USART_ReceiveData(USART1);		//依次接收数据
			Length++;
			if(Length>=4)																	//完成数据接收
			{	RX_State=2;}																//进入状态2
			break;
		case 2:
			Length=0;
			if(USART_ReceiveData(USART1)==Packet_Tail)		//检测到包尾
			{	RX_State=0;}																//进入状态0
			break;
	}
}








