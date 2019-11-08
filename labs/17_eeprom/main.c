/*
 * The example shows how to call EEPROM testing
 * For further investigations goto "i2c_eeprom.c"
 */

#include <stdlib.h>

#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"

#include "i2c_eeprom.h"

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
static void rcc_config() {
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
	while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

	/* Set APB1 prescaler */
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

	/* Update CMSIS variable (which can be updated also
	* through SystemCoreClockUpdate function) */
	SystemCoreClock = 48000000;
}

static void gpio_config(void) {
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
}


static void i2c_config(void) {
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	
	// SCL - GPIOB6
	LL_GPIO_SetPinMode(			GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinSpeed(		GPIOB, LL_GPIO_PIN_6, LL_GPIO_SPEED_FREQ_HIGH);
	LL_GPIO_SetPinOutputType(	GPIOB, LL_GPIO_PIN_6, LL_GPIO_OUTPUT_OPENDRAIN);
	LL_GPIO_SetPinPull(			GPIOB, LL_GPIO_PIN_6, LL_GPIO_PULL_UP);
	LL_GPIO_SetAFPin_0_7(		GPIOB, LL_GPIO_PIN_6, LL_GPIO_AF_1);

	// SDA - GPIOB7
	LL_GPIO_SetPinMode(			GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinSpeed(		GPIOB, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);
	LL_GPIO_SetPinOutputType(	GPIOB, LL_GPIO_PIN_7, LL_GPIO_OUTPUT_OPENDRAIN);
	LL_GPIO_SetPinPull(			GPIOB, LL_GPIO_PIN_7, LL_GPIO_PULL_UP);
	LL_GPIO_SetAFPin_0_7(		GPIOB, LL_GPIO_PIN_7, LL_GPIO_AF_1);
	
	
	LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_SYSCLK);
	LL_I2C_Disable(I2C1);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
	LL_I2C_DisableAnalogFilter(I2C1);
	LL_I2C_SetDigitalFilter(I2C1, 1);

	LL_I2C_SetTiming(I2C1, 0x50330309);
	LL_I2C_DisableClockStretching(I2C1);
	LL_I2C_SetMasterAddressingMode(I2C1, LL_I2C_ADDRESSING_MODE_7BIT);
	LL_I2C_SetMode(I2C1, LL_I2C_MODE_I2C);
	LL_I2C_Enable(I2C1);
}

int main(void) {
	rcc_config();
	gpio_config();
	i2c_config(); // EE_Test won't do this for us
	
	if(EE_Test() == 0)
		LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_9); // The green one is good
	else
		LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8); // The blue one means errors
	
	while (1)
		;
	
	return 0;
}
