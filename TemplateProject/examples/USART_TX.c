#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_exti.h"
#include <string.h>

void SystemClock_Config(void);
void Configure_USART1(void);
void usart_transmit(char *tx);

int
main(void) {
        SystemClock_Config();
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);

        LL_GPIO_WriteOutputPort(GPIOC, 0x0000);
        Configure_USART1();

        while (1);
        return 0;
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
SystemClock_Config() {
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
Configure_USART1(void) {
        /* Enable the peripheral clock of GPIOA */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

        /* Configure SCK Pin connected to pin 23 of CN10 connector */
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_8, LL_GPIO_AF_1);
        LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_8, LL_GPIO_SPEED_FREQ_HIGH);
        LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_8,
                                 LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_8, LL_GPIO_PULL_DOWN);

        /* Configure TX Pin connected to pin 21 of CN10 connector */
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_9, LL_GPIO_AF_1);
        LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_HIGH);
        LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_9,
                                 LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_DOWN);

        /* Configure RX Pin connected to pin 33 of CN10 connector */
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_10, LL_GPIO_AF_1);
        LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_HIGH);
        LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_10,
                                 LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_10, LL_GPIO_PULL_DOWN);

        /* Enable the USART1 peripheral clock */
        LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);

        /* Set clock source */
        LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);

        /* Disable USART prior modifying configuration registers */
        LL_USART_Disable(USART1);

        /* TX/RX direction */
        LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_TX_RX);

        /* 8 data bit, 1 start bit, 1 stop bit, no parity */
        LL_USART_ConfigCharacter(USART1, LL_USART_DATAWIDTH_8B,
                                 LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);

        /* Configure transfer bit order : MSB first */
        LL_USART_SetTransferBitOrder(USART1, LL_USART_BITORDER_LSBFIRST);

        /* Clock configuration : Phase 2 edges, Polarity Low,
         *                       Last Bit Clock output enabled */
        LL_USART_ConfigClock(USART1, LL_USART_PHASE_2EDGE,
                             LL_USART_POLARITY_LOW,
                             LL_USART_LASTCLKPULSE_OUTPUT);

        /* Oversampling by 8 : To be forced to 8 in USART mode */
        LL_USART_SetOverSampling(USART1, LL_USART_OVERSAMPLING_8);

        /* Set Baudrate to 115200 using APB frequency set to 48000000 Hz */
        /* Frequency available for USART peripheral can also be calculated
         * through LL RCC macro */
        /* Ex :
            Periphclk = LL_RCC_GetUSARTClockFreq(Instance);
            In this example, Peripheral Clock is expected to be equal to
            48000000 Hz => equal to SystemCoreClock
        */
        LL_USART_SetBaudRate(USART1, SystemCoreClock,
                             LL_USART_OVERSAMPLING_8, 115200);

        /* Configure peripheral in USART mode for synchronous communication
           (CLK signal delivered by USRAT peripheral) */
        LL_USART_ConfigSyncMode(USART1);

        /* Enable USART1 */
        LL_USART_Enable(USART1);

        /* Polling USART initialisation */
        while (!LL_USART_IsActiveFlag_TEACK(USART1) ||
               !LL_USART_IsActiveFlag_REACK(USART1));
}

void
usart_transmit(char *tx) {
        while (*tx++) {
                LL_USART_TransmitData8(USART1, *tx);
                while (!LL_USART_IsActiveFlag_TC(USART1));
        }
}

void
NMI_Handler(void) {
}

void
HardFault_Handler(void) {
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
                usart_transmit("Hello");
                LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8);
                tick = 0;
        }
}
