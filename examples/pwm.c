#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_tim.h"

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
static void rcc_config(void)
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

        /* Set systick to 1ms */
        SysTick_Config(48000000/1000);

        /* Update CMSIS variable (which can be updated also
         * through SystemCoreClockUpdate function) */
        SystemCoreClock = 48000000;
        return;
}

static void gpio_config(void)
{
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);

        /*
         * Set PWM pin
         */
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_8_15(GPIOC, LL_GPIO_PIN_8, LL_GPIO_AF_1);
        LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_PUSHPULL);
        return;
}

static void tim3_config(void)
{
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
        
        LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH3);
        
        LL_TIM_SetCounterMode(TIM3, LL_TIM_COUNTERMODE_UP);
        LL_TIM_SetAutoReload(TIM3, 48000);
        LL_TIM_SetPrescaler(TIM3, 1);

        LL_TIM_OC_SetMode(TIM3, LL_TIM_CHANNEL_CH3, LL_TIM_OCMODE_PWM1);
        LL_TIM_OC_EnableFast(TIM3, LL_TIM_CHANNEL_CH3);
        LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH3);
        LL_TIM_OC_SetCompareCH3(TIM3, 48000 * 0.1);

        LL_TIM_GenerateEvent_UPDATE(TIM3);

        LL_TIM_EnableCounter(TIM3);

        return;
}

void HardFault_Handler(void)
{
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_9);
        while (1);
}

const int period = 2000;
const int slice = 2000/6;
int current_pwm = 0;
void SysTick_Handler(void)
{
        static int tick = 0;
        tick++;
        if (tick == period) {
                tick = 0;
        }
        current_pwm = tick < slice ? tick :
                      (tick >= slice) && (tick < 2 * slice) ? 2 * slice - tick :
                      (tick >= 2 * slice) && (tick < 3 * slice) ? tick - 2 * slice :
                      (tick >= 3 * slice) && (tick < 6 * slice) ? (6 * slice - tick)/3 :
                      0;
        LL_TIM_OC_SetCompareCH3(TIM3, 48000 * current_pwm / slice);

}


int main(void)
{
        rcc_config();
        gpio_config();
        tim3_config();

        while (1);
        return 0;
}
