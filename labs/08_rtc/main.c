#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rtc.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_exti.h"

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

static void rtc_config(void)
{
    LL_RCC_LSI_Enable();
    while (LL_RCC_LSI_IsReady() != 1);

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    LL_PWR_EnableBkUpAccess();

    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSI);
    LL_RCC_EnableRTC();
    /*
     * Set up time and date
     */
    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_EnableInitMode(RTC);
    while (!LL_RTC_IsActiveFlag_INIT(RTC));

    LL_RTC_SetAsynchPrescaler(RTC, 0x7F);
    LL_RTC_SetSynchPrescaler(RTC, 0x0137);

    LL_RTC_SetHourFormat(RTC, LL_RTC_HOURFORMAT_24HOUR);
    LL_RTC_DATE_Config(RTC, LL_RTC_WEEKDAY_FRIDAY, 1,
                       LL_RTC_MONTH_MARCH, 2019);
    LL_RTC_TIME_Config(RTC, LL_RTC_TIME_FORMAT_AM_OR_24, 11, 22, 00);

    LL_RTC_DisableInitMode(RTC);
    LL_RTC_EnableWriteProtection(RTC);

    /*
     * Set up alarm
     */
    LL_RTC_DisableWriteProtection(RTC);

    LL_RTC_ALMA_Disable(RTC);
    while (!LL_RTC_IsActiveFlag_ALRAW(RTC));
    LL_RTC_ALMA_SetMask(RTC, LL_RTC_ALMA_MASK_ALL);
    //LL_RTC_ALMA_ConfigTime(RTC, LL_RTC_ALMA_TIME_FORMAT_AM, 00, 00, 00);
    LL_RTC_ALMA_Enable(RTC);
    LL_RTC_EnableIT_ALRA(RTC);

    LL_RTC_EnableWriteProtection(RTC);

    NVIC_EnableIRQ(RTC_IRQn);
    NVIC_SetPriority(RTC_IRQn, 0);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_17);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_17);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_17);
    return;
}

void RTC_IRQHandler(void)
{
    LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8);

    LL_RTC_ClearFlag_ALRA(RTC);
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_17);
}
/*
 * Here we call configure all peripherals we need and
 * start blinking upon current mode
 */
int main(void)
{
    uint32_t date = 0;
    uint32_t time = 0;

    rcc_config();
    gpio_config();
    rtc_config();

    while (1) {
        if (LL_RTC_IsActiveFlag_RS(RTC)) {
            time = LL_RTC_TIME_Get(RTC);
            date = LL_RTC_DATE_Get(RTC);
        }
    }
    return 0;
}
