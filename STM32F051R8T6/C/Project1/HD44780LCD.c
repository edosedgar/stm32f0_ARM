
#include <stm32f0xx_gpio.h>
#include <stm32f0xx.h>

#define LCD_E	GPIO_Pin_4
#define LCD_RW	GPIO_Pin_5
#define LCD_RS  GPIO_Pin_6
#define LCDPort GPIOB
//===========================================
void Delay(volatile uint32_t nCount);
void InitLCD(void);
void WriteData(uint8_t Data);
void WriteCommand(uint8_t Command);
void ResetControllerLcd(void);
void SetCursor(uint8_t x,uint8_t y);
void WriteChar(uint8_t Char);
void WriteText(char *Text);
//===========================================

void Delay(volatile uint32_t nCount)
{
    for (; nCount > 0; nCount--);
};

void WriteData(uint8_t Data)
{
	GPIO_WriteBit(LCDPort,LCD_RS,Bit_SET);
    (LCDPort->ODR)=((LCDPort->ODR&0xFFF0)+(Data>>4));
    GPIO_WriteBit(LCDPort,LCD_E ,Bit_SET);
    Delay(10);
    GPIO_WriteBit(LCDPort,LCD_E ,Bit_RESET);
    (LCDPort->ODR)=((LCDPort->ODR&0xFFF0)+(Data&0x0F));
    GPIO_WriteBit(LCDPort,LCD_E ,Bit_SET);
    Delay(10);
    GPIO_WriteBit(LCDPort,LCD_E ,Bit_RESET);
    GPIO_WriteBit(LCDPort,LCD_RS,Bit_RESET);
    Delay(300);
};

void WriteCommand(uint8_t Command)
{
	GPIO_WriteBit(LCDPort,LCD_RS,Bit_RESET);
    (LCDPort->ODR)=((LCDPort->ODR&0xFFF0)+(Command>>4));
    GPIO_WriteBit(LCDPort,LCD_E ,Bit_SET);
    Delay(10);
    GPIO_WriteBit(LCDPort,LCD_E ,Bit_RESET);
    (LCDPort->ODR)=((LCDPort->ODR&0xFFF0)+(Command&0x0F));
    GPIO_WriteBit(LCDPort,LCD_E ,Bit_SET);
    Delay(10);
    GPIO_WriteBit(LCDPort,LCD_E ,Bit_RESET);
    Delay(300);
};

void SetCursor(uint8_t x,uint8_t y)
{
	uint8_t Buffer;
	Buffer=(y-1)*0x40+(x-1);
	WriteCommand(0x80+Buffer);
};

void InitLCD(void)
{
	Delay(300000);
	GPIO_WriteBit(LCDPort,LCD_RW,Bit_RESET);
	GPIO_WriteBit(LCDPort,LCD_RS,Bit_RESET);
	GPIO_WriteBit(LCDPort,LCD_E ,Bit_SET);
    (LCDPort->ODR)=((LCDPort->ODR&0xFFF0)+0x02);
    GPIO_WriteBit(LCDPort,LCD_E ,Bit_RESET);
	Delay(300);
    WriteCommand(0x2C);
    WriteCommand(0x0C);
    WriteCommand(0x06);
};

void WriteChar(uint8_t Char)
{
	WriteData(Char);
};

void WriteText(char *Text)
{
	while (*Text)
	{
		WriteChar(*Text++);
	}
};
