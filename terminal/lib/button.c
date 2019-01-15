#include <string.h>

#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "fsm.h"

void fsm_btn_init(void *args)
{
        (void) args;

        /*
         * Setting button PIN A0
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
        LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_NO);
        fsm_set_state(FSM_PWM_INIT);
        return;
}

void fsm_btn_handler(void *args)
{
        if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0)) {
                ((uint8_t *)args)[0] = 4;
                memcpy(((uint8_t *)args) + 1, "HIGH", 4);
        }
        else {
                ((uint8_t *)args)[0] = 3;
                memcpy(((uint8_t *)args) + 1, "LOW", 3);
        }
        
        fsm_set_data(FSM_TERM_RESPOND, args);
        fsm_set_state(FSM_TERM_RESPOND);
        return;
}
