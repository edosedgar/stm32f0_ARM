#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_adc.h"

#include <string.h>
#include <stdlib.h>

/*
 * Terminal control structure
 */

typedef struct {
        uint8_t *channel;
        uint8_t rx_complete;
        uint8_t *response;
} term_ctrl_t;

static term_ctrl_t term_ctrl;

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
        /*
         * Set PWM pin
         */
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_8_15(GPIOC, LL_GPIO_PIN_8, LL_GPIO_AF_1);
        LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_PUSHPULL);
        /*
         * Set output pin
         */
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
        /*
         * Setting button PIN A0
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
        LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_NO);
        /*
         * Setting analog AIN1
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_1, LL_GPIO_MODE_ANALOG);

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

static void adc_config(void)
{
        /*
         * Setting ADC
         */
        /* Turn on ADC1 as peripheral */
        LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);
        /* Clock selection */
        LL_RCC_HSI14_Enable();
        LL_ADC_SetClock(ADC1, LL_ADC_CLOCK_ASYNC);
        while (LL_RCC_HSI14_IsReady() != 1);
        /* ADC Calibration */
        if (LL_ADC_IsEnabled(ADC1)) {
                LL_ADC_Disable(ADC1);
        }
        while (LL_ADC_IsEnabled(ADC1));
        LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_NONE);
        LL_ADC_StartCalibration(ADC1);
        while (LL_ADC_IsCalibrationOnGoing(ADC1));
        /* ADC Setting */
        LL_ADC_Enable(ADC1);
        LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_8B);
        LL_ADC_SetDataAlignment(ADC1, LL_ADC_DATA_ALIGN_RIGHT);
        LL_ADC_SetLowPowerMode(ADC1, LL_ADC_LP_MODE_NONE);
        LL_ADC_SetSamplingTimeCommonChannels(ADC1,
                                             LL_ADC_SAMPLINGTIME_239CYCLES_5);
        LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
        LL_ADC_REG_SetSequencerChannels(ADC1, LL_ADC_CHANNEL_1);
        LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_CONTINUOUS);
        LL_ADC_REG_SetOverrun(ADC1, LL_ADC_REG_OVR_DATA_PRESERVED);
        /* Enable ADC conversion */
        LL_ADC_REG_StartConversion(ADC1);

        return;
}

static void uart_dma_config(void)
{
        /*
         * Setting USART pins
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
        /*
         * USART TX
         */
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_9, LL_GPIO_AF_1);
        LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_HIGH);
        /*
         * USART RX
         */
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_10, LL_GPIO_AF_1);
        LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_HIGH);
        /*
         * USART Set clock source
         */
        LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);
        LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
        /*
         * USART Setting
         */
        LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_TX_RX);
        LL_USART_SetParity(USART1, LL_USART_PARITY_NONE);
        LL_USART_SetDataWidth(USART1, LL_USART_DATAWIDTH_8B);
        LL_USART_SetStopBitsLength(USART1, LL_USART_STOPBITS_1);
        LL_USART_SetTransferBitOrder(USART1, LL_USART_BITORDER_LSBFIRST);
        LL_USART_SetBaudRate(USART1, SystemCoreClock,
                             LL_USART_OVERSAMPLING_16, 115200);
        LL_USART_EnableDMAReq_RX(USART1);

        /*
         * DMA configuration
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
        LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3,
                               (uint32_t)&((USART1)->RDR), 
                               (uint32_t)(term_ctrl.channel),
                               LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, 10);
        LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3,
                                LL_DMA_MEMORY_INCREMENT);
        LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_CIRCULAR);
        LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
        /*
         * NVIC configuration
         */
        NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1);
        NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
        /*
         * USART and DMA turn on
         */
        LL_USART_Enable(USART1);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

        return;
}

void comm_send_msg(uint8_t *buff, int len)
{
        int i = 0;

        LL_USART_ClearFlag_TC(USART1);
        while (len--) {
                while (!LL_USART_IsActiveFlag_TXE(USART1));
                LL_USART_TransmitData8(USART1, buff[i++]);
        }
        while (!LL_USART_IsActiveFlag_TC(USART1));
        return;
}

void HardFault_Handler(void)
{
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_9);
        while (1);
}

void SysTick_Handler(void)
{
        static int tick = 0;
        tick++;
        if (tick == 1000) {
                tick = 0;
        }
}

/*
 * DMA hardware interrupt handler
 */
void DMA1_Channel2_3_IRQHandler(void)
{
        if (LL_DMA_IsActiveFlag_TC3(DMA1)) {
                LL_DMA_ClearFlag_TC3(DMA1);
                LL_DMA_ClearFlag_HT3(DMA1);
                LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
                term_ctrl.rx_complete = 1;
        }
}

/*
 * Terminal commands implementation
 */
int set_pwm(term_ctrl_t *t_ctrl)
{
        uint8_t pvm = t_ctrl->channel[1];
        if (abs(pvm) <= 100) {
                LL_TIM_OC_SetCompareCH3(TIM3, 48000 * abs(pvm)/100);
                memcpy(t_ctrl->response, "OK", 2);
                return 2;
        }
        else {
                memcpy(t_ctrl->response, "ERROR", 5);
                return 5;
        }

        return 0;
}

int read_btn_pin(term_ctrl_t *t_ctrl)
{
        if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0)) {
                memcpy(t_ctrl->response, "HIGH", 4);
                return 4;
        }
        else {
                memcpy(t_ctrl->response, "LOW", 3);
                return 3;
        }
}

int read_adc(term_ctrl_t *t_ctrl)
{
        uint8_t val = LL_ADC_REG_ReadConversionData8(ADC1);
        memcpy(t_ctrl->response, &val, 1);
        return 1;
}

int main(void)
{
        int resp_len = 0;
        uint8_t rx_buff[10];
        uint8_t tx_buff[10];
        term_ctrl.channel = rx_buff;
        term_ctrl.response = tx_buff;

        rcc_config();
        gpio_config();
        tim3_config();
        adc_config();
        uart_dma_config();

        while (1) {
                if (term_ctrl.rx_complete == 1)
                {
                        term_ctrl.rx_complete = 0;
                        switch (term_ctrl.channel[0]) {
                        case 1:
                                resp_len = set_pwm(&term_ctrl);
                                comm_send_msg(term_ctrl.response, resp_len);
                                break;
                        case 2:
                                resp_len = read_btn_pin(&term_ctrl);
                                comm_send_msg(term_ctrl.response, resp_len);
                                break;
                        case 3:
                                resp_len = read_adc(&term_ctrl);
                                comm_send_msg(term_ctrl.response, resp_len);
                                break;
                        default:
                                break;
                        }
                }
        }
        return 0;
}
