
#include "stm32f0xx.h"
#include <stm32f0xx_gpio.h>
#include <LCD240x320.c>
#include <InitPeripheral.c>
#include <Voltmetr.c>

int main(void)
{
	RCC_Configuration();
	GPIO_Configuration();
	//ADC_Configuration();
	//TIM_Configuration();
	SPI_Configuration();
	InitLCD();
	uint16_t Vertical,Vertical2;

	DrawFillRectangle(1,1,240,320,0x0000);
	//DrawRectangle(1,1,240,320,3,RED);
	for (Vertical=0;Vertical<=39;Vertical++)
	{
		DrawText(1,Vertical*8+1,"This is test sentences, you look this =)",1,1,BLUE,YELLOW);
	}
	uint32_t Packet=1;
	DrawFillRectangle(1,1,240,320,YELLOW);
    while (1)
    {
    	Packet=Packet+1;
    	//SPI2->DR=Packet;
    	//while (!(SPI2->SR & SPI_SR_RXNE));
    	//Packet=SPI2->DR;
    	//Delay(100);
    		//DrawText(10,50,"HELLO",5,5,Packet,YELLOW);
    	//LCDWriteRegister(0x41,Vertical-=1);
    	for (Vertical=1;Vertical<=320;Vertical++)
    	{
    		DrawFillRectangle(1,Vertical,240,1,Vertical*Packet);
    	}
    	for (Vertical=1;Vertical<=240;Vertical++)
    	{
    		DrawFillRectangle(Vertical,1,1,320,Vertical*Packet);
    	}

    }
}
