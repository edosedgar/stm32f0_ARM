#include <stm32f0xx_gpio.h>
#include <stm32f0xx_tim.h>
#include <stm32f0xx.h>
#include <InitPeripheral.c>
#include <SinSound.c>

void TIM6_DAC_IRQHandler(void);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM14_IRQHandler(void);

void Delay(volatile uint32_t nCount)
{
    for (; nCount > 0; nCount--);
};

//**************************************************************************************
int main(void)
{
	RCC_Configuration();
	GPIO_Configuration();
	ClearSoundBuffers();
	TIM_Configuration();
	uint8_t i=0;
	while (1)
    {
		OutFrequence(261,0,500,750);
		OutFrequence(277,0,500,750);
		OutFrequence(500,1,2000,750);
		OutFrequence(293,0,500,750);
		OutFrequence(2489,0,500,750);
		OutFrequence(2637,0,500,750);
		OutFrequence(2794,0,500,750);
		OutFrequence(2960,0,500,750);
		OutFrequence(3136,0,500,750);
		OutFrequence(3332,0,500,750);
		OutFrequence(3440,0,500,750);
		OutFrequence(3729,0,500,750);
		OutFrequence(3951,0,500,750);
    };
}

//**************************************************************************************
void TIM2_IRQHandler(void)
{
	//Render Channel1
	SoundRender(0);
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}
//**************************************************************************************
void TIM3_IRQHandler(void)
{
	//Render Channel2
	SoundRender(1);
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}
//**************************************************************************************
void TIM6_DAC_IRQHandler(void)
{
	DelayTimeRender();
	TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
}
//**************************************************************************************
