/*
 * This example demonstrates usage of system interrupts
 * After powering on Blue LED is on
 * in a second a green LED is on as well
 */

#include <stdlib.h>

#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"

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
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
    return;
}

/*
 * Just set of commands to waste CPU power for a second
 * (basically it is a simple cycle with a predefined number
 * of loops)
 */
__attribute__((naked)) static void delay(void)
{
    asm ("push {r7, lr}");
    asm ("ldr r6, [pc, #8]");
    asm ("sub r6, #1");
    asm ("cmp r6, #0");
    asm ("bne delay+0x4");
    asm ("pop {r7, pc}");
    asm (".word 0x5b8d80"); //6000000
}

/*
 * This handler will be called in a handler
 * mode (MSP stack)
 */
void unpriviliged_callback(void *data)
{
    (void) data;
    LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8);
}

/*
 * This function stores input parameters in r0 and r1 regs
 * and request SVC interrupt
 */
#pragma GCC diagnostic ignored "-Wunused-parameter"
void service_call(void (*func)(void *), void *args)
{
    //by convention func is in r0 and args is in r1
    asm volatile("svc 0");
}
#pragma GCC diagnostic pop

int main(void)
{
    /*
     * Turn on thread mode
     * (We just took some very far place in RAM)
     */
    __set_CONTROL(0x02);
    __set_PSP(0x20001000);

    rcc_config();
    gpio_config();
    /*
     * Generate SVC interrupt
     */
    service_call(unpriviliged_callback, NULL);
    delay();
    /*
     * Unaligned access -> HardFault
     */
    *((uint32_t *)0x01) = 0;

    while (1);
    return 0;
}
