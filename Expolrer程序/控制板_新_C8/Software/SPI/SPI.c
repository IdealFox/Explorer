#include "stm32f10x.h"                  // Device header
#include "SPI.h"
#include "Delay.h"

/*
	引脚定义			GPIO模式
	SCK			PA5			PP
	MISO		PA6			IPU
	MOSI		PA7			PP
*/


/** @brief	SPI1_GPIO初始化
  **/
void SPI1_GPIO_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_AF_PP;				//复用推挽
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_5|GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Speed	=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode		=GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_InitStruct.GPIO_Pin		=GPIO_Pin_6;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}

/** @brief	SPI1初始化
  **/
void SPI1_Init(void)
{
	SPI1_GPIO_Init();											//GPIO初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);												//使能SPI1
	
	SPI_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.SPI_Mode							=SPI_Mode_Master;										//主机模式
	GPIO_InitStruct.SPI_Direction					=SPI_Direction_2Lines_FullDuplex;		//全双工模式
	GPIO_InitStruct.SPI_DataSize					=SPI_DataSize_8b;										//8位数据
	GPIO_InitStruct.SPI_FirstBit					=SPI_FirstBit_MSB;									//高位先行
	GPIO_InitStruct.SPI_BaudRatePrescaler	=SPI_BaudRatePrescaler_4	;					//波特率预分频器
	GPIO_InitStruct.SPI_CPOL							=SPI_CPOL_Low;											//时钟模式
	GPIO_InitStruct.SPI_CPHA							=SPI_CPHA_1Edge;										//数据传输模式
	GPIO_InitStruct.SPI_NSS								=SPI_NSS_Soft;											//片选模式
	GPIO_InitStruct.SPI_CRCPolynomial			=7;																	//CRC!验
	SPI_Init(SPI1,&GPIO_InitStruct);
	
	SPI_Cmd(SPI1,ENABLE);							//开启SPI1
}

/** @brief	SPI1交换字节
  * @param	ByteSend	发送的字节
  * @retval	接收的字节
  **/
u8 SPI1_SwapByte(u8 ByteSend)
{
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)!=SET);		//等待发送标志位为空
	SPI_I2S_SendData(SPI1,ByteSend);														//发送数据
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)!=SET);	//等待接收标志位为空
	return SPI_I2S_ReceiveData(SPI1);														//接收数据
}




 





