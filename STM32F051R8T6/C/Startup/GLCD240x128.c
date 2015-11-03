
#include "stm32f0xx.h"
#include <stm32f0xx_gpio.h>

#define LCD_RES GPIO_Pin_8
#define LCD_CS  GPIO_Pin_9
#define LCD_E	GPIO_Pin_12
#define LCD_RW	GPIO_Pin_11
#define LCD_RS  GPIO_Pin_10
#define LCDPort GPIOB
#define clBlack 1
#define clWhite 0

uint8_t LCDBuffer[3840];

//===============================================================================================
void Delay(volatile uint32_t nCount);
void InitLCD(void);
void WriteData(uint8_t Data);
void WriteCommand(uint8_t Command);
void ResetControllerLcd(void);
void RefreshLCD(void);
void SetPixel(uint16_t x,uint16_t y,uint16_t color);
void DrawRectangle(uint16_t x1,uint16_t y1,uint16_t width,uint16_t height,uint16_t color);
void DrawFillRectangle(uint16_t x1,uint16_t y1,uint16_t width,uint16_t height,uint16_t color);
void ShowForm(uint16_t x1,uint16_t y1,uint16_t width,uint16_t height);
void ClrScr(void);
//===============================================================================================

void Delay(volatile uint32_t nCount)
{
    for (; nCount > 0; nCount--);
}

void WriteData(uint8_t Data)
{
	//GPIO_WriteBit(LCDPort,LCD_RW,Bit_RESET);
	GPIO_WriteBit(LCDPort,LCD_RS,Bit_RESET);
	GPIO_WriteBit(LCDPort,LCD_E ,Bit_SET);
    LCDPort->ODR=((LCDPort->ODR&0xFF00)+Data);
    GPIO_WriteBit(LCDPort,LCD_E ,Bit_RESET);
    GPIO_WriteBit(LCDPort,LCD_CS,Bit_SET);
}

void WriteCommand(uint8_t Command)
{
	GPIO_WriteBit(LCDPort,LCD_CS,Bit_RESET);
	//GPIO_WriteBit(LCDPort,LCD_RW,Bit_RESET);
	GPIO_WriteBit(LCDPort,LCD_RS,Bit_SET);
	GPIO_WriteBit(LCDPort,LCD_E ,Bit_SET);
	LCDPort->ODR=((LCDPort->ODR&0xFF00)+Command);
    GPIO_WriteBit(LCDPort,LCD_E ,Bit_RESET);
}

void ResetControllerLcd(void)
{
	GPIO_WriteBit(LCDPort,LCD_RES,Bit_RESET);
	Delay(10000);
	GPIO_WriteBit(LCDPort,LCD_RES,Bit_SET);
	GPIO_WriteBit(LCDPort,LCD_CS,Bit_SET);
}

void InitLCD(void)
{
	ResetControllerLcd();
	//1)Mode Control
    WriteCommand(0b00000000);
    WriteData(0b00110010);
    //2)Set Character Pitch
    WriteCommand(0b00000001);
    WriteData(0b00000111);
    //3)Set Number of Characters
    WriteCommand(0b00000010);
    WriteData(0b00011101);
    //4)Set Number of Time Divisions (Inverse of Display Duty Ratio)
    WriteCommand(0b00000011);
    WriteData(0b01111111);
    //5)Set Cursor Position
    WriteCommand(0b00000100);
    WriteData(0b00000000);
    //6)Set Display Start Low Order Address
    //low order address
    WriteCommand(0b00001000);
    WriteData(0b00000000);
    //high order address
    WriteCommand(0b00001001);
    WriteData(0b00000000);
}

void RefreshLCD(void)
{
    //low order address cursor
    WriteCommand(0b00001010);
    WriteData(0b00000000);
    //high order address cursor
    WriteCommand(0b00001011);
    WriteData(0b00000000);
    uint16_t i;
	for (i=0;i<3840;i++)
	{
    	GPIO_WriteBit(LCDPort,LCD_CS,Bit_RESET);
    	GPIO_WriteBit(LCDPort,LCD_RS,Bit_SET);
    	GPIO_WriteBit(LCDPort,LCD_E ,Bit_SET);
    	LCDPort->ODR=((LCDPort->ODR&0xFF00)+0x0C);
        GPIO_WriteBit(LCDPort,LCD_E ,Bit_RESET);

    	GPIO_WriteBit(LCDPort,LCD_RS,Bit_RESET);
    	GPIO_WriteBit(LCDPort,LCD_E ,Bit_SET);
        LCDPort->ODR=((LCDPort->ODR&0xFF00)+LCDBuffer[i]);
        GPIO_WriteBit(LCDPort,LCD_E ,Bit_RESET);
        GPIO_WriteBit(LCDPort,LCD_CS,Bit_SET);
	}
};

void SetPixel(uint16_t x,uint16_t y,uint16_t color)
{
	uint16_t AdrByte;
	uint16_t AdrBit;
	AdrByte=(30*(y-1))+((x-1) / 8);
	AdrBit=((x-1) % 8);
	switch (color)
	{
		case clWhite:
		LCDBuffer[AdrByte]&=(~(1<<AdrBit));
		break;
		case clBlack:
		LCDBuffer[AdrByte]|=(1<<AdrBit);
		break;
	};
};

void DrawRectangle(uint16_t x1,uint16_t y1,uint16_t width,uint16_t height,uint16_t color)
{
	uint16_t i;
	for (i=x1;i<x1+width-1;i++)
	{
		SetPixel(i,y1,color);
	};
    for (i=y1;i<y1+height-1;i++)
	{
		SetPixel(x1+width-1,i,color);
	};
    for (i=x1+width-1;i>x1;i--)
	{
	 	SetPixel(i,y1+height-1,color);
	};
    for (i=y1+height-1;i>y1;i--)
	{
		SetPixel(x1,i,color);
	};
};

void DrawFillRectangle(uint16_t x1,uint16_t y1,uint16_t width,uint16_t height,uint16_t color)
{
	uint16_t i;
	uint16_t j;
	for (i=y1;i<y1+height;i++)
	{
		for (j=x1;j<x1+width;j++)
		{
			SetPixel(j,i,color);
		}
	}
}

void ClrScr(void)
{
	uint16_t i;
	for (i=0;i<3840;i++)
	{
		LCDBuffer[i]=0;
	}
}

void ShowForm(uint16_t x1,uint16_t y1,uint16_t width,uint16_t height)
{
	DrawRectangle(x1,y1,width,height,0);
	DrawRectangle(x1+1,y1+1,width-2,height-2,1);
	DrawRectangle(x1+2,y1+2,width-4,height-4,1);
	DrawRectangle(x1+3,y1+3,width-6,height-6,0);
	DrawRectangle(x1+4,y1+8,width-8,height-12,1);
	DrawFillRectangle(x1+4,y1+4,width-8,3,1);
	DrawRectangle(x1+4,y1+7,width-8,1,0);
	DrawFillRectangle(x1+5,y1+9,width-10,height-14,0);
}
