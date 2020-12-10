/*
 * Is an example of different low power modes configuration of stm32
 * See lecture for more information and read the description in readme
 * To notice the difference, connect amperemeter to MCU power rails
 */

#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_rtc.h"

#include "stm32f0xx_ll_gpio.h"

// set delay for entering low power mode in milliseconds
#define DELAY 3000 // 3 sec

// select one of this modes

#define LOW_FREQ
//#define SLEEP
//#define STOP
//#define STANDBY

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
    while (LL_RCC_HSI_IsReady() != 1)
        ;

    /* Main PLL configuration and activation */
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2,
                                LL_RCC_PLL_MUL_12);

    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1)
        ;

    /* Sysclk activation on the main PLL */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
        ;

    /* Set APB1 prescaler */
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    /* Update CMSIS variable (which can be updated also
     * through SystemCoreClockUpdate function) */
    SystemCoreClock = 48000000;
}

static void exti_config()
{
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_0);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_0);

    //LL_EXTI_EnableEvent_0_31(LL_EXTI_LINE_0); // for sleep mode, using __WFE

    NVIC_EnableIRQ(EXTI0_1_IRQn);
    NVIC_SetPriority(EXTI0_1_IRQn, 0);
}

static void timers_config(void)
{
    /*
     * Setup timer
     */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    LL_TIM_SetPrescaler(TIM2, 47999);
    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
    LL_TIM_OC_SetCompareCH1(TIM2, DELAY);
    LL_TIM_GenerateEvent_UPDATE(TIM2);
    LL_TIM_EnableIT_CC1(TIM2);

    LL_TIM_EnableCounter(TIM2);
    /*
     * Setup NVIC
     */
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, 2);
    return;
}

void EXTI0_1_IRQHandler()
{
#ifdef LOW_FREQ
    // reinit RCC
    rcc_config();
#elif defined(SLEEP)
    // we are in Run mode
#elif defined(STOP)
    // we are in Run mode
    rcc_config();
#elif defined(STANDBY)
    // we lost all data
    // so after wake up we will be in main like after reset but with accordinly set bit
#endif

    // let timer go
    LL_TIM_GenerateEvent_UPDATE(TIM2);
    LL_TIM_EnableCounter(TIM2);

    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
}

void TIM2_IRQHandler(void)
{
    // stop timer // we don't need them more in low power mode
    LL_TIM_DisableCounter(TIM2);
    // set counter 0
    LL_TIM_GenerateEvent_UPDATE(TIM2);

#ifdef LOW_FREQ
    // set 512 prescaler
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_512);
#elif defined(SLEEP)
    // enable clock for PWR
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    // select sleep after exiting last interrupt
    LL_LPM_EnableSleepOnExit();
    // enable events and interrups for waking up
    LL_LPM_EnableEventOnPend();
    // enable sleep mode
    LL_LPM_EnableSleep();

    // going to sleep // waiting for interrupt mode
    __WFI();

    // try __WFE();
    // don't forget to enable event in EXTI and disable interrupt
#elif defined(STOP)
    // enable clock for PWR
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    // enable deep sleep modes
    LL_LPM_EnableDeepSleep();
    // select low power voltage regulator
    LL_PWR_SetPowerMode(LL_PWR_MODE_STOP_LPREGU);

    // going to sleep // waiting for interrupt mode
    __WFI();
#elif defined(STANDBY)
    // enable clock for PWR
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    // select deep sleep
    LL_LPM_EnableDeepSleep();
    // select power mode
    LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
    // enable wakeup pin PA0 to wake up
    LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN1);
    // clear was in standby mode flag
    LL_PWR_ClearFlag_WU();
    // clear RTC alarm flar
    LL_RTC_ClearFlag_ALRA(RTC);
    // go to standby
    __WFI();
#endif

    LL_TIM_ClearFlag_CC1(TIM2);
}

static void gpio_config()
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA | LL_AHB1_GRP1_PERIPH_GPIOB |
                             LL_AHB1_GRP1_PERIPH_GPIOC | LL_AHB1_GRP1_PERIPH_GPIOD |
                             LL_AHB1_GRP1_PERIPH_GPIOF);

    GPIOA->MODER = 0xFFFFFFFF;
    GPIOB->MODER = 0xFFFFFFFF;
    GPIOC->MODER = 0xFFFFFFFF;
    GPIOD->MODER = 0xFFFFFFFF;
    GPIOF->MODER = 0xFFFFFFFF;

    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);

    LL_AHB1_GRP1_DisableClock(LL_AHB1_GRP1_PERIPH_GPIOA | LL_AHB1_GRP1_PERIPH_GPIOB |
                              LL_AHB1_GRP1_PERIPH_GPIOC | LL_AHB1_GRP1_PERIPH_GPIOD |
                              LL_AHB1_GRP1_PERIPH_GPIOF);
}

int main(void)
{
    // Entering standart mode 48MHz
    rcc_config();
    exti_config();
    gpio_config();

    timers_config();

    while (1)
        ;
    return 0;
}
