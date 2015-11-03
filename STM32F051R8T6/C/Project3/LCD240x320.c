
#include <Font6x8.h>
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>

#define LCD_CS   GPIO_Pin_4	 //Chip enable
#define LCD_REST GPIO_Pin_3	 //Hardware reset
#define LCD_RS 	 GPIO_Pin_0  //Data or Command
#define LCD_WR   GPIO_Pin_1  //Write strobe
#define LCDPort   GPIOC
#define LCDPort2  GPIOB
#define ClrPort	  BRR
#define SetPort	  BSRR
#define SystemFont Font6x8
#define BitIsSet(reg, bit)        ((reg & (1<<bit)) != 0)
#define BitIsClear(reg, bit)      ((reg & (1<<bit)) == 0)

//================Color table================
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000
#define BLUE         	 0x001F
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40
#define BRRED 			 0XFC07
#define GRAY  			 0X8430
#define DARKBLUE      	 0X01CF
#define LIGHTBLUE      	 0X7D7C
#define GRAYBLUE       	 0X5458
#define LIGHTGREEN     	 0X841F
#define LGRAY 			 0XC618
#define LGRAYBLUE        0XA651
#define LBBLUE           0X2B12
//===========================================

//---------------------------------
void LCDWriteCommand(uint8_t Command);
void LCDWriteData(uint16_t Data);
void LCDWriteRegister(uint8_t Register,uint16_t Data);
void InitLCD();
void DrawFillRectangle(uint8_t X,uint16_t Y,uint8_t Width,uint16_t Height,uint16_t Color);
void DrawRectangle(uint8_t X,uint16_t Y,uint8_t Width,uint16_t Height,uint16_t WidthLine,uint16_t Color);
void DrawText(uint8_t X,uint16_t Y,char* Text,uint8_t ScaleX,uint8_t ScaleY,uint16_t Color,uint16_t BackgroundColor);
void DrawSymbol(uint8_t X,uint16_t Y,char Symbol,uint8_t ScaleX,uint8_t ScaleY,uint16_t Color,uint16_t BackgroundColor);
void DrawLine(uint8_t x1,uint16_t y1,uint8_t x2,uint16_t y2,uint16_t Color);
void Delay(uint32_t nCount);
#define DrawPoint(x,y,color) DrawFillRectangle(x,y,1,1,color);
//---------------------------------

void LCDWriteRegister(uint8_t Register,uint16_t Data)
{
	LCDWriteCommand(Register);
	LCDWriteData(Data);
}

void LCDWriteCommand(uint8_t Command)
{
	LCDPort2->ClrPort = LCD_CS;
	LCDPort2->ClrPort = LCD_RS;
	LCDPort->ODR=Command;
	LCDPort2->ClrPort = LCD_WR;
	Delay(1);
    LCDPort2->SetPort = LCD_WR;
    LCDPort2->SetPort = LCD_RS;
    LCDPort2->SetPort = LCD_CS;
}

void LCDWriteData(uint16_t Data)
{
	LCDPort2->ClrPort = LCD_CS;
	LCDPort2->SetPort = LCD_RS;
	LCDPort->ODR = Data;
	LCDPort2->ClrPort = LCD_WR;
	Delay(1);
    LCDPort2->SetPort = LCD_WR;
    LCDPort2->SetPort = LCD_CS;
}

void DrawFillRectangle(uint8_t X,uint16_t Y,uint8_t Width,uint16_t Height,uint16_t Color)
{
	LCDWriteCommand(0x4E); //x
	LCDWriteData(X-1);
	LCDWriteCommand(0x4F); //y
	LCDWriteData(Y-1);
	LCDWriteCommand(0x44); // конечный и начальный адрес по горизонтали x
	LCDWriteData((Width+X-2)*256+X-1);
	LCDWriteCommand(0x45); //начальный адрес по вертикали y
	LCDWriteData(Y-1);
	LCDWriteCommand(0x46); //конечный адрес по вертикали y
	LCDWriteData(Height+Y-2);
    LCDWriteCommand(0x22);
    LCDPort2->ClrPort = LCD_CS;
    asm ("MOV R0,%0\n\t" :: "r" (&(LCDPort2->BRR))); //Reset
    asm ("MOV R1,%0\n\t" :: "r" (&(LCDPort2->BSRR)));//Set
    asm ("MOV R4,%0\n\t" :: "r" (LCD_WR));
    asm ("MOV R5,%0\n\t" :: "r" (Color));
    asm ("MOV R6,%0\n\t" :: "r" (&(LCDPort->ODR)));
    asm ("MOV R3,%0\n\t" :: "r" (Width*Height));
    asm ("SendPackets:");
		asm ("STR R5,[R6]"); //LCDPort->ODR=Color;
		asm ("STR R4,[R0]"); //GPIO_WriteBit(LCDPort2,LCD_WR,Bit_RESET);
		asm ("STR R4,[R1]"); //GPIO_WriteBit(LCDPort2,LCD_WR,Bit_SET);
	asm ("SUB R3,R3,#1");
	asm ("BNE SendPackets");
	LCDPort2->SetPort = LCD_CS;
}

void InitLCD()
{
    LCDPort2->SetPort = LCD_CS;
    LCDPort2->SetPort = LCD_REST;
	//Standard initialization
	LCDWriteRegister(0x07, 0x0021); //Display control
	LCDWriteRegister(0x00, 0x0001); //Oscillation Start
	LCDWriteRegister(0x07, 0x0023); //Display control
	LCDWriteRegister(0x10, 0x0000); //Sleep mode
    Delay(100);
    LCDWriteRegister(0x07, 0x0633); //Display control
    LCDWriteRegister(0x11, 0x6070); //Entry Mode
    LCDWriteRegister(0x02, 0x0600); //LCD drive AC control
    //==========================
    LCDWriteRegister(0x0C, 0x0007); //Power control (2)
    LCDWriteRegister(0x0D, 0x000F); //Power control (3)
    LCDWriteRegister(0x0E, 0x2200); //Power control (4)
    LCDWriteRegister(0x1E, 0x009F); //Power control (5)

    LCDWriteRegister(0x01, 0x3B3F); //Driver output control
    LCDWriteRegister(0x0B, 0x8000); //Frame cycle control
    LCDWriteRegister(0x25, 0xE000); //Frame Frequency

    LCDWriteRegister(0x28, 0x0006);
    LCDWriteRegister(0x2F, 0x12BE);
    LCDWriteRegister(0x12, 0x6CEB);
    DrawFillRectangle(1,1,240,320,BLACK);
}

void DrawRectangle(uint8_t X,uint16_t Y,uint8_t Width,uint16_t Height,uint16_t WidthLine,uint16_t Color)
{
	DrawFillRectangle(X,Y,Width,WidthLine,Color);
	DrawFillRectangle(X,Y,WidthLine,Height,Color);
	DrawFillRectangle(X,Y+Height-WidthLine,Width,WidthLine,Color);
	DrawFillRectangle(X+Width-WidthLine,Y,WidthLine,Height,Color);
}

void DrawSymbol(uint8_t X,uint16_t Y,char Symbol,uint8_t ScaleX,uint8_t ScaleY,uint16_t Color,uint16_t BackgroundColor)
{
	uint16_t WindowX,WindowY;
	WindowX=6*ScaleX;
	WindowY=8*ScaleY;
	LCDWriteCommand(0x4E); //x
	LCDWriteData(X-1);
	LCDWriteCommand(0x4F); //y
	LCDWriteData(Y-1);
	LCDWriteCommand(0x44); // конечный и начальный адрес по горизонтали x
	LCDWriteData((WindowX+X-2)*256+X-1);
	LCDWriteCommand(0x45); //начальный адрес по вертикали y
	LCDWriteData(Y-1);
	LCDWriteCommand(0x46); //конечный адрес по вертикали y
	LCDWriteData(WindowY+Y-2);
    LCDWriteCommand(0x22);
    LCDPort2->ClrPort = LCD_CS;
	uint16_t i,j;
	for (j=0;j<WindowY;j++)
	{
		for (i=0;i<WindowX;i++)
		{
			if (BitIsSet(SystemFont[Symbol*6+i/ScaleX],j/ScaleY)==1)
			{
				LCDPort->ODR=Color;
			} else
			{
				LCDPort->ODR=BackgroundColor;
			}
			LCDPort2->ClrPort = LCD_WR;
			LCDPort2->SetPort = LCD_WR;
		}
	}
    LCDPort2->SetPort = LCD_CS;
}

void DrawText(uint8_t X,uint16_t Y,char* Text,uint8_t ScaleX,uint8_t ScaleY,uint16_t Color,uint16_t BackgroundColor)
{
	uint16_t Position=X;
	while (*Text)
	{
		DrawSymbol(Position,Y,*Text++,ScaleX,ScaleY,Color,BackgroundColor);
		Position=Position+6*ScaleX;
	}
}

void DrawLine(uint8_t x1,uint16_t y1,uint8_t x2,uint16_t y2,uint16_t Color)
{
	uint8_t deltaX=abs(x2-x1);
	uint16_t deltaY=abs(y2-y1);
	uint8_t signX=x1<x2?1:-1;
	uint8_t signY=y1<y2?1:-1;
	int16_t error=deltaX-deltaY;
    DrawPoint(x2,y2,Color);
    while(x1!=x2||y1!=y2)
    {
        DrawPoint(x1,y1,Color);
        int16_t error2=error*2;
        if(error2>-deltaY)
        {
            error-=deltaY;
            x1+=signX;
        }
        if(error2<deltaX)
        {
            error+=deltaX;
            y1+=signY;
        }
    }
}

void Delay(uint32_t nCount)
{
    for (; nCount > 0; nCount--);
};

