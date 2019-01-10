#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"

/*
 * Global button click counter
 */
uint8_t btn_clicks = 0;

/*
 * Global time counter
 */
 int time_tick = 0;

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

static void rcc_init()
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
}

static void gpio_init(void)
{
        /*
         * Setting button PIN A0
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
        LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_NO);

        /*
         * Setting USER LED PC8 and PC9
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
}

void
SysTick_Handler(void) {
        time_tick++;
        if (time_tick == 1000) {
                if (btn_clicks % 2) {
                       LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8); 
                }
                time_tick = 0;
        }
}

/*
 * Delay in ms
 */
void delay_ms(int time_delay)
{
        int time_start = 1000 - time_tick > time_delay ? time_tick :
                                                         time_tick - 1000;
        while (time_tick - time_start < time_delay);

        return;
}

int main(void)
{
        rcc_init();
        gpio_init();
        uint8_t btn_was_pressed = 0;

        while (1) {
                if (btn_was_pressed && 
                    !LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0)) {
                        delay_ms(50);
                        if(!LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0))
                                btn_clicks++;
                }
                btn_was_pressed = LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0);

        }
        return 0;
}
