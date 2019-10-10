#include "fsm.h"

#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"

/*
 * Associate all state codes with subroutines
 */
enum RET_CODES (*state_executer[])(void) = {
    [STARTUP] = do_startup,
    [BUTTON_POLL] = do_button_poll,
    [LED_TURN_ON] = do_led_turn
};

/*
 * Transition table
 * src_state | return code | dst_state
 * WARNING: avoid dublicatins
 */
fsm_cell_t fsm_table[] = {
    {STARTUP, OK_FSM, BUTTON_POLL},
    {BUTTON_POLL, OK_FSM, LED_TURN_ON},
    {BUTTON_POLL, REPEAT_FSM, BUTTON_POLL},
    {LED_TURN_ON, OK_FSM, BUTTON_POLL}
};

void fsm_loop(enum FSM_STATES init_state) {
    int state = init_state;
    enum RET_CODES ret_code;
    int iter;
    while (1) {
        /*
         * Run current state and get a return code
         */
        ret_code = state_executer[state]();
        /*
         * Find the entry with a given state and return code, and
         * extract destination state.
         * If there is no entry with such return code then we start it over
         */
        for (iter = 0; iter < sizeof(fsm_table); iter++) {
            if (fsm_table[iter].src_state == state &&
                fsm_table[iter].ret_code == ret_code) {
                    state = fsm_table[iter].dst_state;
                    break;
                }
        }
    }
}
/*
 * ========================================================
 * ============ Implement state executers =================
 * ========================================================
 */

enum RET_CODES do_startup(void) {
    /*
     * Init two default LEDs
     */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
    /*
     * Init port for USER button
     */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
    return OK_FSM;
}

enum RET_CODES do_button_poll(void) {
    /*
     * poll button, if it is pressed then return OK,
     * otherwise REPEAT
     */
    if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0)) {
        return OK_FSM;
    } else {
        return REPEAT_FSM;
    }
}

enum RET_CODES do_led_turn(void) {
    LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8);
    /*
     * turn on the LED, then come back to polling
     */
    return OK_FSM;
}