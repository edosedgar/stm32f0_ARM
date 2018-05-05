#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_spi.h"

static void
gpio_config(void) {
        /*
         * Setting clock
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
        /*
         * Setting LED pins
         */
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
        return;
}

static void
spi_config(void) {
        /*
         * Init GPIO
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
        //SPI_MOSI
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_7, LL_GPIO_AF_0);
        LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);
        //SPI_MISO
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_6, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_6, LL_GPIO_AF_0);
        //SPI_SCK
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_5, LL_GPIO_AF_0);
        LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_5, LL_GPIO_SPEED_FREQ_HIGH);
        //SPI_CS
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_4, LL_GPIO_AF_0);
        LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_4, LL_GPIO_SPEED_FREQ_HIGH);
        /*
         * Init SPI
         */
        LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SPI1);
        LL_SPI_SetMode(SPI1, LL_SPI_MODE_MASTER);
        LL_SPI_SetBaudRatePrescaler(SPI1, LL_SPI_BAUDRATEPRESCALER_DIV8);
        LL_SPI_SetTransferBitOrder(SPI1, LL_SPI_MSB_FIRST);
        LL_SPI_SetDataWidth(SPI1, LL_SPI_DATAWIDTH_8BIT);
        LL_SPI_SetNSSMode(SPI1, LL_SPI_NSS_HARD_OUTPUT);
        LL_SPI_EnableNSSPulseMgt(SPI1);
        LL_SPI_Enable(SPI1);
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
static void
rcc_config() {
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

void
SysTick_Handler(void) {
}

static inline void
sendSPI(uint8_t value) {
        LL_SPI_TransmitData8(SPI1, value);
        while (!LL_SPI_IsActiveFlag_TXE(SPI1));
        return;
}

static inline uint8_t
recvSPI() {
        uint8_t byte;

        while (!LL_SPI_IsActiveFlag_RXNE(SPI1))
        byte = LL_SPI_ReceiveData8(SPI1);
        return byte;
}

int
main(void) {
        uint8_t byte = 0x00;

        rcc_config();
        gpio_config();
        spi_config();

        while (1) {
                byte = recvSPI();
                sendSPI(byte);
        }
        return 0;
}
