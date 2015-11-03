
#include <Font6x8.h>
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>

#define CS   GPIO_Pin_4
#define LCDPort  GPIOA
//================Color table================
#define clRed     0xE0
#define clGreen   0x1C
#define clBlue    0x03
#define clWhite   0xFF
#define clBlack   0x00
#define clYellow  0xFC
#define clMagenta 0xE3
#define clCyan    0x1F
//===========================================

//---------------------------------
void SendToLCD(uint32_t Packet);
void InitLCD();
void DrawFillRectangle(uint32_t X,uint32_t Y,uint32_t Width,uint32_t Height,uint32_t Color);
void DrawRectangle(uint32_t X,uint32_t Y,uint32_t Width,uint32_t Height,uint32_t WidthLine,uint32_t Color);
void DrawText(uint32_t X,uint32_t Y,char* Text,uint32_t ScaleX,uint32_t ScaleY,uint32_t Color);
void DrawSymbol(uint32_t X,uint32_t Y,char Symbol,uint32_t ScaleX,uint32_t ScaleY,uint32_t Color);


void SendToLCD(uint32_t Packet)
{
	LCDPort->ODR=0;
	//Transmission 9-bit--------------
	SPI1->DR=Packet;
	while (!(SPI1->SR & SPI_SR_RXNE));
	Packet=SPI1->DR;
	//--------------------------------
	LCDPort->ODR=CS;
}

void InitLCD()
{
	// Software reset
	SendToLCD(0x0001);
	//Booster voltage on
	SendToLCD(0x0003);
	//Sleep out
	SendToLCD(0x0011);
	//Normal mode on
	SendToLCD(0x0013);
	//Display inversion off
	SendToLCD(0x0020);
	//Set Contrast
	SendToLCD(0x0025);
	SendToLCD(0x3F);
	//Interface pixel format
	SendToLCD(0x003A);
	SendToLCD(0x0102);
	//Memory data access control
	SendToLCD(0x0036);
	SendToLCD(0x120);
	//Tearing line on
	SendToLCD(0x0035);
	//Display on
	SendToLCD(0x0029);
}

void DrawFillRectangle(uint32_t X,uint32_t Y,uint32_t Width,uint32_t Height,uint32_t Color)
{
	//Column address set
	SendToLCD(0x2A);
	SendToLCD(0x100+Y);
	SendToLCD(0x100+Y+Height-1);
	//Row address set
	SendToLCD(0x2B);
	SendToLCD(0x100+X);
	SendToLCD(0x100+X+Width-1);
	//Memory write
	uint32_t i,CountPixels;
	SendToLCD(0x2C);
	CountPixels=Height*Width;
	for (i=1;i<=CountPixels;i++)
	{
		SendToLCD(0x0100+Color);//3:3:2
	}
}

void DrawRectangle(uint32_t X,uint32_t Y,uint32_t Width,uint32_t Height,uint32_t WidthLine,uint32_t Color)
{

}

void DrawSymbol(uint32_t X,uint32_t Y,char Symbol,uint32_t ScaleX,uint32_t ScaleY,uint32_t Color)
{

}

void DrawText(uint32_t X,uint32_t Y,char* Text,uint32_t ScaleX,uint32_t ScaleY,uint32_t Color)
{

}
