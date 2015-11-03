#include <stm32f0xx_gpio.h>
#include <Voltmetr.c>
#include <HD44780LCD.c>
#include <InitPeripheral.c>

#include <stm32f0xx.h>

int main(void)
{
	RCC_Configuration();
	GPIO_Configuration();
	ADC_Configuration();
	TIM_Configuration();
	uint32_t Voltage,AverageSample,i;
	uint32_t Deviation=100000;
    while (1)    {

    	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_SET);
    	Delay(Deviation);
    	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET);

    	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_SET);
    	Delay(Deviation);
    	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET);

    	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_SET);
    	Delay(Deviation);
    	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_RESET);
    	if (Deviation>15000) {Deviation-=Deviation/100;} else {Deviation-=1;};
    }
}
