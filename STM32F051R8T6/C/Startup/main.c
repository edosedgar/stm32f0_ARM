
#include <stm32f0xx_gpio.h>
#include <GLCD240x128.c>
#include <stm32f0xx.h>
#include <InitPeripheral.c>

int main(void)
{
	GPIO_Configuration();
    InitLCD();
	RCC_Configuration();
	uint8_t Pos=60;
    while (1)
    {
    	/*ClrScr();
    	Pos+=1;
    	DrawRectangle(Pos,Pos,242-2*Pos,130-2*Pos,clBlack);
    	DrawRectangle(Pos-20,Pos-20,242-2*(Pos-20),130-2*(Pos-20),clBlack);
    	DrawRectangle(Pos-40,Pos-40,242-2*(Pos-40),130-2*(Pos-40),clBlack);
    	ShowForm(165,5,70,80);
    	DrawRectangle(1,1,240,128,clBlack);
    	RefreshLCD();
    	if (Pos==64) {Pos=44;};*/
    	DrawFillRectangle(1,1,240,128,clWhite);
    	RefreshLCD();
    	Delay(500);
    	DrawFillRectangle(1,1,240,128,clBlack);
    	RefreshLCD();
    	Delay(500);
    }
}
