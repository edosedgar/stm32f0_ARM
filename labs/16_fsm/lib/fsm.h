#ifndef _H_FSM_H_
#define _H_FSM_H_

/*
 * Define all states to be used in FSM
 */
enum FSM_STATES {
    STARTUP = 0,
    BUTTON_POLL,
    LED_TURN_ON 
};

/* 
 * Define all possible return codes for each state
 */
enum RET_CODES {
    OK_FSM = 0,
    ERROR_FSM,
    REPEAT_FSM
};

/*
 * Define the structure for transition table
 */
typedef struct {
    enum FSM_STATES src_state;
    enum RET_CODES ret_code;
    enum FSM_STATES dst_state;
} fsm_cell_t;

/*
 * Here all managing happens
 */
void fsm_loop(enum FSM_STATES init_state);

/*
 * Define all subroutines associated with FSM state
 */
enum RET_CODES do_startup(void);
enum RET_CODES do_button_poll(void);
enum RET_CODES do_led_turn(void);

#endif 