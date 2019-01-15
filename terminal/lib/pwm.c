#include <stdlib.h>
#include <string.h>

#include "fsm.h"

#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_tim.h"

void fsm_pwm_init(void *args)
{
        (void) args;

        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
        /*
         * Set PWM pin
         */
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_8_15(GPIOC, LL_GPIO_PIN_8, LL_GPIO_AF_1);
        LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_8,
                                 LL_GPIO_OUTPUT_PUSHPULL);
        /*
         * Set timer to PWM mode
         */
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
        fsm_set_state(FSM_TERM_INIT);
        return;
}

void fsm_pwm_handler(void *args)
{
        uint8_t pwm = ((uint8_t *)args)[0];

        if (abs(pwm) <= 100) {
                LL_TIM_OC_SetCompareCH3(TIM3, 48000 * abs(pwm)/100);
                ((uint8_t *)args)[0] = 2;
                memcpy((uint8_t *)args + 1, "OK", 2);
        }
        else {
                ((uint8_t *)args)[0] = 5;
                memcpy((uint8_t *)args + 1, "ERROR", 5);
        }
        fsm_set_state(FSM_TERM_MAIN);
        return;
}
