#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"

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
static void
rcc_config(void) {
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
        tick++;
        if (tick == 1000) {
                LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8);
                tick = 0;
        }
}

__attribute__((naked)) static void
delay(void) {
        asm ("push {r7, lr}");
        asm ("ldr r6, [pc, #8]");
        asm ("sub r6, #1");
        asm ("cmp r6, #0");
        asm ("bne delay+0x4");
        asm ("pop {r7, pc}");
        asm (".word 0x5b8d80"); //6000000
        //asm (".word 0x927c00"); //9600000
}

int
main(void) {
        rcc_config();

        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);

        while (1) {
                delay();
                LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_9);
        }
        return 0;
}
