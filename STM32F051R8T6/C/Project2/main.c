
#include "stm32f0xx.h"
#include <stm32f0xx_gpio.h>
#include <NokiaLCD.c>
#include <InitPeripheral.c>
#include <Voltmetr.c>

void Delay(volatile uint32_t nCount);

int main(void)
{
	RCC_Configuration();
	GPIO_Configuration();
	ADC_Configuration();
	TIM_Configuration();
	SPI_Configuration();
	InitLCD();
	DrawFillRectangle(1,1,130,130,clBlack);
	uint32_t Position=1;
	uint32_t Position2=1;
	uint32_t Flag;
    while (1)
    {
    	if (((GPIOA->IDR)&GPIO_Pin_2)==0x0000) {DrawFillRectangle(1,1,130,130,clBlack);;Position2-=10;Flag=1;};
    	if (((GPIOA->IDR)&GPIO_Pin_9)==0x0000) {DrawFillRectangle(1,1,130,130,clBlack);;Position2+=10;Flag=1;};
    	if (((GPIOA->IDR)&GPIO_Pin_8)==0x0000) {DrawFillRectangle(1,1,130,130,clBlack);;Position+=10;Flag=1;};
    	if (((GPIOA->IDR)&GPIO_Pin_1)==0x0000) {DrawFillRectangle(1,1,130,130,clBlack);;Position-=10;Flag=1;};
    	DrawFillRectangle(Position2,Position,30,30,clBlue|(0x08));
    	if (Flag==1) {Delay(700000);Flag=0;};
    }
}

void Delay(volatile uint32_t nCount)
{
    for (; nCount > 0; nCount--);
};




