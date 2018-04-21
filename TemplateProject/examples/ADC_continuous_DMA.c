#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_adc.h"
#include "stm32f0xx_ll_dma.h"

char ADCBuffer[16] = {0};

void config_RCC(void);
void config_ADC2DMA(void);
void config_IO(void);

int
main(void) {
        config_RCC();
        config_IO();
        config_ADC2DMA();

        /* Being pressed button pulls pin to VCC, hence ADC value is
         * greater than 128. To be concise, it is equal to 255. */
        while (1) {
            if (ADCBuffer[0] > 128)
                LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_8);
            else
                LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_8);
        }
        return 0;
}

void config_IO(void) {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_ANALOG);
        return;
}

void config_ADC2DMA(void) {
        /* Turn on ADC1 as peripheral */
        LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);
        /* Clock selection */
        LL_RCC_HSI14_Enable();
        LL_ADC_SetClock(ADC1, LL_ADC_CLOCK_ASYNC);
        while (LL_RCC_HSI14_IsReady() != 1);
        /* ADC Calibration */
        if (LL_ADC_IsEnabled(ADC1)) {
                LL_ADC_Disable(ADC1);
        }
        while (LL_ADC_IsEnabled(ADC1));
        LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_NONE);
        LL_ADC_StartCalibration(ADC1);
        while (LL_ADC_IsCalibrationOnGoing(ADC1));
        /* ADC Setting */
        LL_ADC_Enable(ADC1);
        LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_8B);
        LL_ADC_SetDataAlignment(ADC1, LL_ADC_DATA_ALIGN_RIGHT);
        LL_ADC_SetLowPowerMode(ADC1, LL_ADC_LP_MODE_NONE);
        LL_ADC_SetSamplingTimeCommonChannels(ADC1,
                                             LL_ADC_SAMPLINGTIME_239CYCLES_5);
        LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
        LL_ADC_REG_SetSequencerChannels(ADC1, LL_ADC_CHANNEL_0);
        LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_CONTINUOUS);
        LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);
        LL_ADC_REG_SetOverrun(ADC1, LL_ADC_REG_OVR_DATA_PRESERVED);
        /* DMA Setting */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
        LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_1,
                                        LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
        LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MODE_CIRCULAR);
        LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_1,
                                LL_DMA_PERIPH_NOINCREMENT);
        LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_1,
                                LL_DMA_MEMORY_INCREMENT);
        LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_BYTE);
        LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_BYTE);
        LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_1,
                                       LL_DMA_PRIORITY_VERYHIGH);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, 16);
        LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)&(ADC1->DR));
        LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)ADCBuffer);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
        /* Enable ADC conversion */
        LL_ADC_REG_StartConversion(ADC1);

        return;
}

/**
  * System Clock Configuration
  * The system Clock is configured as follow :
  *    System Clock source            = PLL (HSI/2)
  *    SYSCLK(Hz)                     = 48000000
  *    HCLK(Hz)                       = 48000000
  *    AHB Prescaler                  = 1
  *    APB1 Prescaler                 = 1
  *    HSI Frequency(Hz)              = 8000000
  *    PLLMUL                         = 12
  *    Flash Latency(WS)              = 1
  */

void
config_RCC() {
        /* Set FLASH latency */
        LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

        /* Enable HSI and wait for activation*/
        LL_RCC_HSI_Enable();
        while (LL_RCC_HSI_IsReady() != 1);

        /* Main PLL configuration and activation */
        LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2,
                                    LL_RCC_PLL_MUL_12);

        LL_RCC_PLL_Enable();
        while (LL_RCC_PLL_IsReady() != 1);

        /* Sysclk activation on the main PLL */
        LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
        LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
        while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

        /* Set APB1 prescaler */
        LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

        /* Set systick to 1ms */
        SysTick_Config(48000000/1000);

        /* Update CMSIS variable (which can be updated also
         * through SystemCoreClockUpdate function) */
        SystemCoreClock = 48000000;
}

void
NMI_Handler(void) {
}

void
HardFault_Handler(void) {
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_9);
        while (1);
}

void
SVC_Handler(void) {
}

void
PendSV_Handler(void) {
}

int tick;
void
SysTick_Handler(void) {
}
