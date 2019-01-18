#include <stdlib.h>
#include <string.h>

#include "fsm.h"

#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"

void fsm_led_init(void *args)
{
        /*
         * Setting USER LED PC8 and PC9
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);

        fsm_set_state(FSM_TERM_MAIN);
}

void fsm_led_handler(void *args)
{
        uint8_t *arr = (uint8_t *)args;

        if (arr[0] == 1) {
                LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_9);
        } else
        {
                LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_9);
        }
        arr[0] = 2;
        memcpy(arr + 1, "OK", 2);
        fsm_set_state(FSM_TERM_RESPOND);
}
