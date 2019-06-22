/*
 * This example demonstrates how to configure analog
 * to digital converter
 */

#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_adc.h"
#include "stm32f0xx_ll_dma.h"

#include "xprintf.h"
#include "oled_driver.h"

uint16_t adc_buffer[16] = {0};

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
static void rcc_config()
{
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

    /* Update CMSIS variable (which can be updated also
     * through SystemCoreClockUpdate function) */
    SystemCoreClock = 48000000;
}

/*
 * Clock on GPIOC and set pin with Blue led connected
 */
static void gpio_config(void)
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
    return;
}

/*
 * Configure ADC with DMA
 */
static void adc_config(void)
{
    /*
     * Setting ADC
     */
    /* Turn on ADC1 as peripheral */
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);
    /* Clock selection */
    LL_RCC_HSI14_Enable();
    LL_ADC_SetClock(ADC1, LL_ADC_CLOCK_ASYNC);
    while (LL_RCC_HSI14_IsReady() != 1);
    /* ADC Calibration process */
    if (LL_ADC_IsEnabled(ADC1)) {
        LL_ADC_Disable(ADC1);
    }
    while (LL_ADC_IsEnabled(ADC1));
    LL_ADC_StartCalibration(ADC1);
    while (LL_ADC_IsCalibrationOnGoing(ADC1));
    /* Turn on ADC */
    LL_ADC_Enable(ADC1);
    LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_12B);
    LL_ADC_SetDataAlignment(ADC1, LL_ADC_DATA_ALIGN_RIGHT);
    LL_ADC_SetLowPowerMode(ADC1, LL_ADC_LP_MODE_NONE);
    LL_ADC_SetSamplingTimeCommonChannels(ADC1,
                                         LL_ADC_SAMPLINGTIME_239CYCLES_5);
    LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
    LL_ADC_REG_SetSequencerChannels(ADC1, LL_ADC_CHANNEL_TEMPSENSOR |
                                          LL_ADC_CHANNEL_VREFINT);
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
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_HALFWORD);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_HALFWORD);
    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_1,
                                   LL_DMA_PRIORITY_VERYHIGH);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, 16);
    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)&(ADC1->DR));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)adc_buffer);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    /* Enable interrupt */
    NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    /* Enable ADC conversion */
    LL_ADC_SetCommonPathInternalCh(ADC, LL_ADC_PATH_INTERNAL_TEMPSENSOR |
                                        LL_ADC_PATH_INTERNAL_VREFINT);
    LL_ADC_REG_StartConversion(ADC1);
    return;
}

/*
 * Set callback for out device
 */
static void printf_config(void)
{
    xdev_out(oled_putc);
    return;
}

static float temp = 0;
/*
 * Handle the end of transmission
 */
void DMA1_Channel1_IRQHandler(void)
{
    int i;
    float avg_temp = 0;
    float avg_vdda = 0;
    float ts_cal1 = *TEMPSENSOR_CAL1_ADDR;
    float ts_cal2 = *TEMPSENSOR_CAL2_ADDR;
    float vref_cal = *VREFINT_CAL_ADDR;
    float vdda = 0;

    for (i = 0; i < 16; i+=2) {
        avg_temp += adc_buffer[i];
        avg_vdda += adc_buffer[i+1];
    }
    vdda = VREFINT_CAL_VREF * vref_cal / (avg_vdda / 8);
    temp = (TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP)/(ts_cal2 - ts_cal1) *
           (avg_temp / 8 * vdda / VREFINT_CAL_VREF - ts_cal1);
    temp += 30;

    LL_DMA_ClearFlag_TC1(DMA1);
}

/*
 * Init ADC, DMA, display and show current temperature
 */
int main(void)
{
    rcc_config();
    gpio_config();
    adc_config();
    oled_config();
    printf_config();

    while (1) {
        for (int delay = 0; delay < 100000; delay++);
        oled_set_cursor(0, 3);
        xprintf("   Temperature: %dC", (int)temp);
        oled_update();
    }

    return 0;
}
