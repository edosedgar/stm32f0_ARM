/**
  ******************************************************************************
  *
  * 1. This file provides two functions and one global variable to be called from
  *    user application:
  *      - SystemInit(): This function is called at startup just after reset and
  *                      before branch to main program. This call is made inside
  *                      the "startup_stm32f0xx.s" file.
  *
  *      - SystemCoreClock variable: Contains the core clock (HCLK), it can be used
  *                                  by the user application to setup the SysTick
  *                                  timer or configure other parameters.
  *
  *      - SystemCoreClockUpdate(): Updates the variable SystemCoreClock and must
  *                                 be called whenever the core clock is changed
  *                                 during program execution.
  *
  * 2. After each device reset the HSI (8 MHz) is used as system clock source.
  *    Then SystemInit() function is called, in "startup_stm32f0xx.s" file, to
  *    configure the system clock before to branch to main program.
  *
  * 3. This file configures the system clock as follows:
  *=============================================================================
  *                         Supported STM32F0xx device
  *-----------------------------------------------------------------------------
  *        System Clock source                    | HSI
  *-----------------------------------------------------------------------------
  *        SYSCLK(Hz)                             | 8000000
  *-----------------------------------------------------------------------------
  *        HCLK(Hz)                               | 8000000
  *-----------------------------------------------------------------------------
  *        AHB Prescaler                          | 1
  *-----------------------------------------------------------------------------
  *        APB1 Prescaler                         | 1
  *-----------------------------------------------------------------------------
  *=============================================================================
  */

#include "stm32f0xx.h"

#if !defined  (HSE_VALUE)
  #define HSE_VALUE    ((uint32_t)8000000) /*!< Default value of the External oscillator in Hz.
                                                This value can be provided and adapted by the user application. */
#endif /* HSE_VALUE */

#if !defined  (HSI_VALUE)
  #define HSI_VALUE    ((uint32_t)8000000) /*!< Default value of the Internal oscillator in Hz.
                                                This value can be provided and adapted by the user application. */
#endif /* HSI_VALUE */

#if !defined (HSI48_VALUE)
#define HSI48_VALUE    ((uint32_t)48000000) /*!< Default value of the HSI48 Internal oscillator in Hz.
                                                 This value can be provided and adapted by the user application. */
#endif /* HSI48_VALUE */

uint32_t SystemCoreClock = 8000000;

const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};

/**
  * Setup the microcontroller system.
  * Initialize the default HSI clock source, vector table location and the
  * PLL configuration is reset.
  */
void SystemInit(void)
{
        /* Reset the RCC clock configuration to the default reset state */
        /* Set HSION bit */
        RCC->CR |= (uint32_t)0x00000001U;

        /* Reset SW[1:0], HPRE[3:0], PPRE[2:0], ADCPRE and MCOSEL[2:0] bits */
        RCC->CFGR &= (uint32_t)0xF8FFB80CU;

        /* Reset HSEON, CSSON and PLLON bits */
        RCC->CR &= (uint32_t)0xFEF6FFFFU;

        /* Reset HSEBYP bit */
        RCC->CR &= (uint32_t)0xFFFBFFFFU;

        /* Reset PLLSRC, PLLXTPRE and PLLMUL[3:0] bits */
        RCC->CFGR &= (uint32_t)0xFFC0FFFFU;

        /* Reset PREDIV[3:0] bits */
        RCC->CFGR2 &= (uint32_t)0xFFFFFFF0U;

        /* Reset USART1SW[1:0], I2C1SW, CECSW and ADCSW bits */
        RCC->CFGR3 &= (uint32_t)0xFFFFFEACU;

        /* Reset HSI14 bit */
        RCC->CR2 &= (uint32_t)0xFFFFFFFEU;

        /* Disable all interrupts */
        RCC->CIR = 0x00000000U;
}

/**
  * Update SystemCoreClock variable according to Clock Register Values.
  * The SystemCoreClock variable contains the core clock (HCLK), it can
  * be used by the user application to setup the SysTick timer or configure
  * other parameters.
  *
  * Each time the core clock (HCLK) changes, this function must be called
  * to update SystemCoreClock variable value. Otherwise, any configuration
  * based on this variable will be incorrect.
  *
  * - The system frequency computed by this function is not the real
  *   frequency in the chip. It is calculated based on the predefined
  *   constant and the selected clock source:
  *
  *   - If SYSCLK source is HSI, SystemCoreClock will contain the HSI_VALUE(*)
  *
  *   - If SYSCLK source is HSE, SystemCoreClock will contain the HSE_VALUE(**)
  *
  *   - If SYSCLK source is PLL, SystemCoreClock will contain the HSE_VALUE(**)
  *     or HSI_VALUE(*) multiplied/divided by the PLL factors.
  *
  * (*) HSI_VALUE is a constant defined in stm32f0xx_hal.h file (default value
  *     8 MHz) but the real value may vary depending on the variations
  *     in voltage and temperature.
  *
  * (**) HSE_VALUE is a constant defined in stm32f0xx_hal.h file (default value
  *      8 MHz), user has to ensure that HSE_VALUE is same as the real
  *      frequency of the crystal used. Otherwise, this function may
  *      have wrong result.
  *
  * - The result of this function could be not correct when using fractional
  *   value for HSE crystal.
  *
  */
void SystemCoreClockUpdate (void)
{
        uint32_t tmp = 0, pllmull = 0, pllsource = 0, predivfactor = 0;

        /* Get SYSCLK source */
        tmp = RCC->CFGR & RCC_CFGR_SWS;

        switch (tmp) {
        case RCC_CFGR_SWS_HSI:  /* HSI used as system clock */
                SystemCoreClock = HSI_VALUE;
                break;
        case RCC_CFGR_SWS_HSE:  /* HSE used as system clock */
                SystemCoreClock = HSE_VALUE;
                break;
        case RCC_CFGR_SWS_PLL:  /* PLL used as system clock */
                /* Get PLL clock source and multiplication factor */
                pllmull = RCC->CFGR & RCC_CFGR_PLLMUL;
                pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;
                pllmull = ( pllmull >> 18) + 2;
                predivfactor = (RCC->CFGR2 & RCC_CFGR2_PREDIV) + 1;

                if (pllsource == RCC_CFGR_PLLSRC_HSE_PREDIV)
                        /* HSE used as PLL clock source :
                         * SystemCoreClock = HSE/PREDIV * PLLMUL */
                        SystemCoreClock = (HSE_VALUE/predivfactor) * pllmull;
                else
                        /* HSI used as PLL clock source :
                         * SystemCoreClock = HSI/2 * PLLMUL */
                        SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
                break;
        default: /* HSI used as system clock */
                SystemCoreClock = HSI_VALUE;
                break;
        }
        /* Compute HCLK clock frequency */
        /* Get HCLK prescaler */
        tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
        /* HCLK clock frequency */
        SystemCoreClock >>= tmp;
}
