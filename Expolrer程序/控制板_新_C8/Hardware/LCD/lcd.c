/************************************************
**å¼•è„šå®šä¹‰
**ä»Žæœº				ä¸»æœº		æ¨¡å¼		å¤ç”¨åŠŸèƒ½
**SCL					PA5			AF_PP		SPI1_SCK				SPIæ—¶é’Ÿ
**SDA					PA7			AF_PP		SPI1_MOSI				SPIæ•°æ®
**RES					PA11		PP											å¤ä½
**DC					PA12		PP											æ•°æ®/å‘½ä»¤é€‰æ‹©
**CS					PB11		PP											SPIç‰‡é€‰
**BL					æ— 															èƒŒå…‰æŽ§åˆ¶
************************************************/
#include "stm32f10x.h"                  // Device header
#include "lcd.h"
#include "stdlib.h"
#include "lcdfont.h"  	 
#include "Delay.h"
#include "SPI.h"

/** @brief	LCDæ¨¡å—GPIOåˆå§‹åŒ–
  **/
void LCD_GPIO_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ä½¿èƒ½GPIOA
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ä½¿èƒ½GPIOB
	
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);									//PA11,PA12
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);									//PB11
 	
 	GPIO_SetBits(GPIOA,GPIO_Pin_11|GPIO_Pin_12);
	GPIO_SetBits(GPIOB,GPIO_Pin_11);
}

//å¿«æ·å®å®šä¹‰
#define LCD_RES(X)		GPIO_WriteBit(GPIOA,GPIO_Pin_11,(BitAction)(X))
#define LCD_DC(X)			GPIO_WriteBit(GPIOA,GPIO_Pin_12,(BitAction)(X))
#define LCD_CS(X)			GPIO_WriteBit(GPIOB,GPIO_Pin_11,(BitAction)(X))

/** @brief	SPIå‘é€æ•°æ®
  * @param	dat		å‘é€çš„æ•°æ®
  **/
void LCD_Writ_Bus(u8 dat) 
{	
	LCD_CS(0);
	SPI1_SwapByte(dat);
  LCD_CS(1);	
}

/** @brief	SPIå‘é€æ•°æ®		8ä½
  * @param	dat		å‘é€çš„æ•°æ®
  **/
void LCD_WR_DATA8(u8 dat)
{
	LCD_Writ_Bus(dat);
}

/** @brief	SPIå‘é€æ•°æ®		16ä½
  * @param	dat		å‘é€çš„æ•°æ®
  **/
void LCD_WR_DATA(u16 dat)
{
	LCD_Writ_Bus(dat>>8);
	LCD_Writ_Bus(dat);
}

/** @brief	LCDå†™å…¥å‘½ä»¤
  * @param	dat		å†™å…¥çš„å‘½ä»¤
  **/
void LCD_WR_REG(u8 dat)
{
	LCD_DC(0);//å†™å‘½ä»¤æ¨¡å¼
	LCD_Writ_Bus(dat);
	LCD_DC(1);//å†™æ•°æ®æ¨¡å¼
}


/** @brief	è®¾ç½®èµ·å§‹å’Œç»“æŸåœ°å€
  * @param	x1			åˆ—èµ·å§‹
  * @param	x2			åˆ—ç»“æŸ
  * @param	y1			è¡Œèµ·å§‹
  * @param	y2			è¡Œç»“æŸ
  **/
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2)
{
	if(USE_HORIZONTAL==0)
	{
		LCD_WR_REG(0x2a);//åˆ—åœ°å€è®¾ç½®
		LCD_WR_DATA(x1+2);
		LCD_WR_DATA(x2+2);
		LCD_WR_REG(0x2b);//è¡Œåœ°å€è®¾ç½®
		LCD_WR_DATA(y1+1);
		LCD_WR_DATA(y2+1);
		LCD_WR_REG(0x2c);//å­˜å‚¨å™¨å†™
	}
	else if(USE_HORIZONTAL==1)
	{
		LCD_WR_REG(0x2a);
		LCD_WR_DATA(x1+2);
		LCD_WR_DATA(x2+2);
		LCD_WR_REG(0x2b);
		LCD_WR_DATA(y1+1);
		LCD_WR_DATA(y2+1);
		LCD_WR_REG(0x2c);
	}
	else if(USE_HORIZONTAL==2)
	{
		LCD_WR_REG(0x2a);
		LCD_WR_DATA(x1+1);
		LCD_WR_DATA(x2+1);
		LCD_WR_REG(0x2b);
		LCD_WR_DATA(y1+2);
		LCD_WR_DATA(y2+2);
		LCD_WR_REG(0x2c);
	}
	else
	{
		LCD_WR_REG(0x2a);
		LCD_WR_DATA(x1+1);
		LCD_WR_DATA(x2+1);
		LCD_WR_REG(0x2b);
		LCD_WR_DATA(y1+2);
		LCD_WR_DATA(y2+2);
		LCD_WR_REG(0x2c);
	}
}

/** @brief	LCDæ¨¡å—åˆå§‹åŒ–
  **/
void LCD_Init(void)
{
	LCD_GPIO_Init();//GPIOåˆå§‹åŒ–
	
	LCD_RES(0);//LCDå¤ä½
	Delay_ms(100);
	LCD_RES(1);
	Delay_ms(100);
	
	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x11); //Sleep out 
	Delay_ms(120);              //Delay 120ms 
	//------------------------------------ST7735S Frame Rate-----------------------------------------// 
	LCD_WR_REG(0xB1); 
	LCD_WR_DATA8(0x05); 
	LCD_WR_DATA8(0x3C); 
	LCD_WR_DATA8(0x3C); 
	LCD_WR_REG(0xB2); 
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C); 
	LCD_WR_DATA8(0x3C); 
	LCD_WR_REG(0xB3); 
	LCD_WR_DATA8(0x05); 
	LCD_WR_DATA8(0x3C); 
	LCD_WR_DATA8(0x3C); 
	LCD_WR_DATA8(0x05); 
	LCD_WR_DATA8(0x3C); 
	LCD_WR_DATA8(0x3C); 
	//------------------------------------End ST7735S Frame Rate---------------------------------// 
	LCD_WR_REG(0xB4); //Dot inversion 
	LCD_WR_DATA8(0x03); 
	//------------------------------------ST7735S Power Sequence---------------------------------// 
	LCD_WR_REG(0xC0); 
	LCD_WR_DATA8(0x28); 
	LCD_WR_DATA8(0x08); 
	LCD_WR_DATA8(0x04); 
	LCD_WR_REG(0xC1); 
	LCD_WR_DATA8(0XC0); 
	LCD_WR_REG(0xC2); 
	LCD_WR_DATA8(0x0D); 
	LCD_WR_DATA8(0x00); 
	LCD_WR_REG(0xC3); 
	LCD_WR_DATA8(0x8D); 
	LCD_WR_DATA8(0x2A); 
	LCD_WR_REG(0xC4); 
	LCD_WR_DATA8(0x8D); 
	LCD_WR_DATA8(0xEE); 
	//---------------------------------End ST7735S Power Sequence-------------------------------------// 
	LCD_WR_REG(0xC5); //VCOM 
	LCD_WR_DATA8(0x1A); 
	LCD_WR_REG(0x36); //MX, MY, RGB mode 
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
	else LCD_WR_DATA8(0xA0); 
	//------------------------------------ST7735S Gamma Sequence---------------------------------// 
	LCD_WR_REG(0xE0); 
	LCD_WR_DATA8(0x04); 
	LCD_WR_DATA8(0x22); 
	LCD_WR_DATA8(0x07); 
	LCD_WR_DATA8(0x0A); 
	LCD_WR_DATA8(0x2E); 
	LCD_WR_DATA8(0x30); 
	LCD_WR_DATA8(0x25); 
	LCD_WR_DATA8(0x2A); 
	LCD_WR_DATA8(0x28); 
	LCD_WR_DATA8(0x26); 
	LCD_WR_DATA8(0x2E); 
	LCD_WR_DATA8(0x3A); 
	LCD_WR_DATA8(0x00); 
	LCD_WR_DATA8(0x01); 
	LCD_WR_DATA8(0x03); 
	LCD_WR_DATA8(0x13); 
	LCD_WR_REG(0xE1); 
	LCD_WR_DATA8(0x04); 
	LCD_WR_DATA8(0x16); 
	LCD_WR_DATA8(0x06); 
	LCD_WR_DATA8(0x0D); 
	LCD_WR_DATA8(0x2D); 
	LCD_WR_DATA8(0x26); 
	LCD_WR_DATA8(0x23); 
	LCD_WR_DATA8(0x27); 
	LCD_WR_DATA8(0x27); 
	LCD_WR_DATA8(0x25); 
	LCD_WR_DATA8(0x2D); 
	LCD_WR_DATA8(0x3B); 
	LCD_WR_DATA8(0x00); 
	LCD_WR_DATA8(0x01); 
	LCD_WR_DATA8(0x04); 
	LCD_WR_DATA8(0x13); 
	//------------------------------------End ST7735S Gamma Sequence-----------------------------// 
	LCD_WR_REG(0x3A); //65k mode 
	LCD_WR_DATA8(0x05); 
	LCD_WR_REG(0x29); //Display on 
} 



/** @brief	åœ¨æŒ‡å®šåŒºåŸŸå¡«å……é¢œè‰²
  * @param	xsta,ysta		èµ·å§‹åæ ‡
  * @param	xend,yend		ç»ˆæ­¢åæ ‡
  * @param	color				å¡«å……é¢œè‰²
  **/
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
	u16 i,j; 
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//è®¾ç½®æ˜¾ç¤ºèŒƒå›´
	for(i=ysta;i<yend;i++)
	{													   	 	
		for(j=xsta;j<xend;j++)
		{
			LCD_WR_DATA(color);
		}
	} 					  	    
}

/******************************************************************************
      º¯ÊýËµÃ÷£ºÔÚÖ¸¶¨Î»ÖÃ»­µã
      Èë¿ÚÊý¾Ý£ºx,y »­µã×ø±ê
                color µãµÄÑÕÉ«
      ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_Address_Set(x,y,x,y);//ÉèÖÃ¹â±êÎ»ÖÃ 
	LCD_WR_DATA(color);
} 


/******************************************************************************
      º¯ÊýËµÃ÷£º»­Ïß
      Èë¿ÚÊý¾Ý£ºx1,y1   ÆðÊ¼×ø±ê
                x2,y2   ÖÕÖ¹×ø±ê
                color   ÏßµÄÑÕÉ«
      ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //¼ÆËã×ø±êÔöÁ¿ 
	delta_y=y2-y1;
	uRow=x1;//»­ÏßÆðµã×ø±ê
	uCol=y1;
	if(delta_x>0)incx=1; //ÉèÖÃµ¥²½·½Ïò 
	else if (delta_x==0)incx=0;//´¹Ö±Ïß 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//Ë®Æ½Ïß 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //Ñ¡È¡»ù±¾ÔöÁ¿×ø±êÖá 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);//»­µã
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}


/******************************************************************************
      º¯ÊýËµÃ÷£º»­¾ØÐÎ
      Èë¿ÚÊý¾Ý£ºx1,y1   ÆðÊ¼×ø±ê
                x2,y2   ÖÕÖ¹×ø±ê
                color   ¾ØÐÎµÄÑÕÉ«
      ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}


/******************************************************************************
      º¯ÊýËµÃ÷£º»­Ô²
      Èë¿ÚÊý¾Ý£ºx0,y0   Ô²ÐÄ×ø±ê
                r       °ë¾¶
                color   Ô²µÄÑÕÉ«
      ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color)
{
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,color);             //3           
		LCD_DrawPoint(x0+b,y0-a,color);             //0           
		LCD_DrawPoint(x0-a,y0+b,color);             //1                
		LCD_DrawPoint(x0-a,y0-b,color);             //2             
		LCD_DrawPoint(x0+b,y0+a,color);             //4               
		LCD_DrawPoint(x0+a,y0-b,color);             //5
		LCD_DrawPoint(x0+a,y0+b,color);             //6 
		LCD_DrawPoint(x0-b,y0+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r))//ÅÐ¶ÏÒª»­µÄµãÊÇ·ñ¹ýÔ¶
		{
			b--;
		}
	}
}

/******************************************************************************
      º¯ÊýËµÃ÷£ºÏÔÊ¾ºº×Ö´®
      Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
                *s ÒªÏÔÊ¾µÄºº×Ö´®
                fc ×ÖµÄÑÕÉ«
                bc ×ÖµÄ±³¾°É«
                sizey ×ÖºÅ ¿ÉÑ¡ 16 24 32
                mode:  0·Çµþ¼ÓÄ£Ê½  1µþ¼ÓÄ£Ê½
      ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void LCD_ShowChinese(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	while(*s!=0)
	{
		if(sizey==12) LCD_ShowChinese12x12(x,y,s,fc,bc,sizey,mode);
		else if(sizey==16) LCD_ShowChinese16x16(x,y,s,fc,bc,sizey,mode);
		else if(sizey==24) LCD_ShowChinese24x24(x,y,s,fc,bc,sizey,mode);
		else if(sizey==32) LCD_ShowChinese32x32(x,y,s,fc,bc,sizey,mode);
		else return;
		s+=2;
		x+=sizey;
	}
}

/******************************************************************************
      º¯ÊýËµÃ÷£ºÏÔÊ¾µ¥¸ö12x12ºº×Ö
      Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
                *s ÒªÏÔÊ¾µÄºº×Ö
                fc ×ÖµÄÑÕÉ«
                bc ×ÖµÄ±³¾°É«
                sizey ×ÖºÅ
                mode:  0·Çµþ¼ÓÄ£Ê½  1µþ¼ÓÄ£Ê½
      ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void LCD_ShowChinese12x12(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j,m=0;
	u16 k;
	u16 HZnum;//ºº×ÖÊýÄ¿
	u16 TypefaceNum;//Ò»¸ö×Ö·ûËùÕ¼×Ö½Ú´óÐ¡
	u16 x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	                         
	HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);	//Í³¼Æºº×ÖÊýÄ¿
	for(k=0;k<HZnum;k++) 
	{
		if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//·Çµþ¼Ó·½Ê½
					{
						if(tfont12[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//µþ¼Ó·½Ê½
					{
						if(tfont12[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//»­Ò»¸öµã
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //²éÕÒµ½¶ÔÓ¦µãÕó×Ö¿âÁ¢¼´ÍË³ö£¬·ÀÖ¹¶à¸öºº×ÖÖØ¸´È¡Ä£´øÀ´Ó°Ïì
	}
} 

/******************************************************************************
      º¯ÊýËµÃ÷£ºÏÔÊ¾µ¥¸ö16x16ºº×Ö
      Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
                *s ÒªÏÔÊ¾µÄºº×Ö
                fc ×ÖµÄÑÕÉ«
                bc ×ÖµÄ±³¾°É«
                sizey ×ÖºÅ
                mode:  0·Çµþ¼ÓÄ£Ê½  1µþ¼ÓÄ£Ê½
      ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void LCD_ShowChinese16x16(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j,m=0;
	u16 k;
	u16 HZnum;//ºº×ÖÊýÄ¿
	u16 TypefaceNum;//Ò»¸ö×Ö·ûËùÕ¼×Ö½Ú´óÐ¡
	u16 x0=x;
  TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//Í³¼Æºº×ÖÊýÄ¿
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//·Çµþ¼Ó·½Ê½
					{
						if(tfont16[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//µþ¼Ó·½Ê½
					{
						if(tfont16[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//»­Ò»¸öµã
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //²éÕÒµ½¶ÔÓ¦µãÕó×Ö¿âÁ¢¼´ÍË³ö£¬·ÀÖ¹¶à¸öºº×ÖÖØ¸´È¡Ä£´øÀ´Ó°Ïì
	}
} 


/******************************************************************************
      º¯ÊýËµÃ÷£ºÏÔÊ¾µ¥¸ö24x24ºº×Ö
      Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
                *s ÒªÏÔÊ¾µÄºº×Ö
                fc ×ÖµÄÑÕÉ«
                bc ×ÖµÄ±³¾°É«
                sizey ×ÖºÅ
                mode:  0·Çµþ¼ÓÄ£Ê½  1µþ¼ÓÄ£Ê½
      ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void LCD_ShowChinese24x24(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j,m=0;
	u16 k;
	u16 HZnum;//ºº×ÖÊýÄ¿
	u16 TypefaceNum;//Ò»¸ö×Ö·ûËùÕ¼×Ö½Ú´óÐ¡
	u16 x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//Í³¼Æºº×ÖÊýÄ¿
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//·Çµþ¼Ó·½Ê½
					{
						if(tfont24[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//µþ¼Ó·½Ê½
					{
						if(tfont24[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//»­Ò»¸öµã
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //²éÕÒµ½¶ÔÓ¦µãÕó×Ö¿âÁ¢¼´ÍË³ö£¬·ÀÖ¹¶à¸öºº×ÖÖØ¸´È¡Ä£´øÀ´Ó°Ïì
	}
} 

/******************************************************************************
      º¯ÊýËµÃ÷£ºÏÔÊ¾µ¥¸ö32x32ºº×Ö
      Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
                *s ÒªÏÔÊ¾µÄºº×Ö
                fc ×ÖµÄÑÕÉ«
                bc ×ÖµÄ±³¾°É«
                sizey ×ÖºÅ
                mode:  0·Çµþ¼ÓÄ£Ê½  1µþ¼ÓÄ£Ê½
      ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void LCD_ShowChinese32x32(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j,m=0;
	u16 k;
	u16 HZnum;//ºº×ÖÊýÄ¿
	u16 TypefaceNum;//Ò»¸ö×Ö·ûËùÕ¼×Ö½Ú´óÐ¡
	u16 x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//Í³¼Æºº×ÖÊýÄ¿
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//·Çµþ¼Ó·½Ê½
					{
						if(tfont32[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//µþ¼Ó·½Ê½
					{
						if(tfont32[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//»­Ò»¸öµã
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //²éÕÒµ½¶ÔÓ¦µãÕó×Ö¿âÁ¢¼´ÍË³ö£¬·ÀÖ¹¶à¸öºº×ÖÖØ¸´È¡Ä£´øÀ´Ó°Ïì
	}
}


/** @brief	æ˜¾ç¤ºå•ä¸ªå­—ç¬¦
	* @param	x,y			æ˜¾ç¤ºåæ ‡
	* @param	num			æ˜¾ç¤ºçš„å­—ç¬¦
	* @param	fc			æ˜¾ç¤ºé¢œè‰²
	* @param	bc			æ˜¾ç¤ºèƒŒæ™¯è‰²
  * @param	sizey		å­—å·
	* @param	mode		0:å åŠ æ¨¡å¼	1:éžå åŠ æ¨¡å¼
  **/
void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 temp,sizex,t,m=0;
	u16 i,TypefaceNum;//ä¸€ä¸ªå­—ç¬¦æ‰€å å­—èŠ‚å¤§å°
	u16 x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //å¾—åˆ°åç§»åŽçš„å€¼
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //è®¾ç½®å…‰æ ‡ä½ç½®
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==12)temp=ascii_1206[num][i];		       //è°ƒç”¨6x12å­—ä½“
		else if(sizey==16)temp=ascii_1608[num][i];		 //è°ƒç”¨8x16å­—ä½“
		else if(sizey==24)temp=ascii_2412[num][i];		 //è°ƒç”¨12x24å­—ä½“
		else if(sizey==32)temp=ascii_3216[num][i];		 //è°ƒç”¨16x32å­—ä½“
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//éžå åŠ æ¨¡å¼
			{
				if(temp&(0x01<<t))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//å åŠ æ¨¡å¼
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//ç”»ä¸€ä¸ªç‚¹
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}


/** @brief	æ˜¾ç¤ºå­—ç¬¦ä¸²
	* @param	x,y			æ˜¾ç¤ºåæ ‡
	* @param	*p			æ˜¾ç¤ºçš„å­—ç¬¦ä¸²
	* @param	fc			æ˜¾ç¤ºé¢œè‰²
	* @param	bc			æ˜¾ç¤ºèƒŒæ™¯è‰²
  * @param	sizey		å­—å·
	* @param	mode		0:å åŠ æ¨¡å¼	1:éžå åŠ æ¨¡å¼
  * @retval	
  **/
void LCD_ShowString(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 sizey,u8 mode)
{         
	while(*p!='\0')
	{       
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}  
}


/** @brief	è®¡ç®—å¹‚
  * @param	m		åº•æ•°
  * @param	n		æŒ‡æ•°
  * @retval	ç»“æžœ
  **/
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;
	return result;
}


/** @brief	æ˜¾ç¤ºæ•´æ•°
	* @param	x,y			æ˜¾ç¤ºåæ ‡
	* @param	num			æ˜¾ç¤ºçš„æ•°
	* @param	len			æ˜¾ç¤ºé•¿åº¦
	* @param	fc			æ˜¾ç¤ºé¢œè‰²
	* @param	bc			æ˜¾ç¤ºèƒŒæ™¯è‰²
  * @param	sizey		å­—å·
  **/
void LCD_ShowIntNum(u16 x,u16 y,u16 num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp;
	u8 enshow=0;
	u8 sizex=sizey/2;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
} 


/** @brief	æ˜¾ç¤ºå°æ•°
	* @param	x,y			æ˜¾ç¤ºåæ ‡
	* @param	num			æ˜¾ç¤ºçš„æ•°
	* @param	len			æ˜¾ç¤ºé•¿åº¦
	* @param	fc			æ˜¾ç¤ºé¢œè‰²
	* @param	bc			æ˜¾ç¤ºèƒŒæ™¯è‰²
  * @param	sizey		å­—å·
  **/
void LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp,sizex;
	u16 num1;
	sizex=sizey/2;
	num1=num*100;
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;
		if(t==(len-2))
		{
			LCD_ShowChar(x+(len-2)*sizex,y,'.',fc,bc,sizey,0);
			t++;
			len+=1;
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}


/** @brief	æ˜¾ç¤ºå›¾ç‰‡
	* @param	x,y			èµ·ç‚¹åæ ‡
	* @param	length	å›¾ç‰‡é•¿åº¦
	* @param	width		å›¾ç‰‡å®½åº¦
	* @param	pic[]		å›¾ç‰‡æ•°ç»„
  **/
void LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[])
{
	u16 i,j;
	u32 k=0;
	LCD_Address_Set(x,y,x+length-1,y+width-1);
	for(i=0;i<length;i++)
	{
		for(j=0;j<width;j++)
		{
			LCD_WR_DATA8(pic[k*2]);
			LCD_WR_DATA8(pic[k*2+1]);
			k++;
		}
	}			
}

/** @brief	LCDæ¸…å±
  **/
void  LCD_Clear(void)
{
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
}

