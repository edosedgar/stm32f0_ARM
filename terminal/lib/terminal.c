#include <stdlib.h>
#include <string.h>

#include "fsm.h"
#include "terminal.h"

#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_gpio.h"

/*
 * Private control structure
 */
static term_ctrl_t term_ctrl;

/*
 * Private function for sending response
 */
static void comm_send_msg(uint8_t *buff, int len)
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

/*
 * Init all periphs related to terminal communication, like
 * UART and all static structures related
 */
void fsm_term_init(void *args)
{
        (void) args;
        /*
         * Reset all flags
         */
        term_ctrl.flags = 0x00;
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
        fsm_set_state(FSM_TERM_MAIN);
        return;
}

/*
 * Here all commands should be processed
 * and then corresponding handler called
 * Example: if some command is requested then fsm_set_state to its
 * corresponding handler
 */

void fsm_term_main(void *args)
{
        (void) args;
        uint8_t command_code;

        if (is_term_flag_set(term_ctrl, RX_COMPLETE)) {
                term_clr_flag(term_ctrl, RX_COMPLETE);
                command_code = term_ctrl.channel[0];
                if (IS_COMMAND_VALID(command_code) &&
                    fsm_states_handlers[command_code] != NULL) {
                        memcpy(term_ctrl.params, &term_ctrl.channel[1],
                               TERM_CMD_LENGTH - 1);
                        fsm_set_data(command_code + FSM_TERM_CMD_START,
                                     (void *)term_ctrl.params);
                        fsm_set_state(command_code + FSM_TERM_CMD_START);
                    }
        }
        return;
}

void fsm_term_respond(void *args)
{
        uint8_t len = term_ctrl.params[0];
        uint8_t *buff = &term_ctrl.params[1];
        
        comm_send_msg(buff, len);
        fsm_set_state(FSM_TERM_MAIN);
        return;
}

/*
 * Dummy function
 */
void fsm_term_cmd_start(void *args)
{
        (void) args;

        return;
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
                term_set_flag(term_ctrl, RX_COMPLETE);
        }
}
