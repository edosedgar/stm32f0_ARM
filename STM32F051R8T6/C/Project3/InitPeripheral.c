
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_flash.h>
#include <stm32f0xx_adc.h>
#include <stm32f0xx_tim.h>
#include <stm32f0xx_spi.h>
//===============================================
void RCC_Configuration(void);
void ADC_Configuration(void);
void GPIO_Configuration(void);
void TIM_Configuration(void);
void SPI_Configuration(void);
//===============================================
void RCC_Configuration(void)
{
	FLASH_PrefetchBufferCmd(ENABLE);
	FLASH_SetLatency(FLASH_Latency_1);
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    if (RCC_WaitForHSEStartUp()==SUCCESS)
    {
    	RCC_HCLKConfig(RCC_SYSCLK_Div1);
    	RCC_PCLKConfig(RCC_HCLK_Div1);
    	RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div4);
    	RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);
    	RCC_PREDIV1Config(RCC_PREDIV1_Div1);
    	RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_6);
    	RCC_PLLCmd(ENABLE);
    	/* Wait till PLL is ready */
    	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}
    	/* Select PLL as system clock source */
    	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    	/* Wait till PLL is used as system clock source */
    	while (RCC_GetSYSCLKSource() != 0x08) {}
    	RCC_HSICmd(DISABLE);
    }
};

void TIM_Configuration(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	/* TIM3 Peripheral clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_DeInit(TIM3);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 0x0000;
	TIM_TimeBaseStructure.TIM_Prescaler = 0x0000;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x00;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* TIM3 TRGO selection */
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
};

void ADC_Configuration(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	ADC_DeInit(ADC1);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	/* Configure the ADC1 in continuous mode with a resolution equal to 12 bits*/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Falling;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* Convert the ADC1 Channel 11 with 239.5 Cycles as sampling time */
	ADC_ChannelConfig(ADC1, ADC_Channel_0 , ADC_SampleTime_239_5Cycles);
    /* ADC Calibration */
	ADC_GetCalibrationFactor(ADC1);

	/* Enable the auto delay feature */
	ADC_WaitModeCmd(ADC1, ENABLE);

	/* Enable the Auto power off mode */
	ADC_AutoPowerOffCmd(ADC1, ENABLE);

	/* Enable ADCperipheral[PerIdx] */
	ADC_Cmd(ADC1, ENABLE);

	/* Wait the ADCEN flag */
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN));

	/* ADC1 regular Software Start Conversion */
	ADC_StartOfConversion(ADC1);
};

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin   = 0xFFFF;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin   = 0xB01F;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
    //GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    //GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14;
    //GPIO_Init(GPIOB, &GPIO_InitStructure);
};

void SPI_Configuration(void)
{

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_High;
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;
	SPI_Init(SPI2,&SPI_InitStructure);
	SPI_Cmd(SPI2, ENABLE);
}
