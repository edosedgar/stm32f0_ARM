FSM_STATE_DEF(DUMMY,                    dummy)
FSM_STATE_DEF(ERROR,                    error)
FSM_STATE_DEF(GLOBAL_INIT,              global_init)
FSM_STATE_DEF(PWM_INIT,                 pwm_init)
FSM_STATE_DEF(BTN_INIT,                 btn_init)
FSM_STATE_DEF(ADC_INIT,                 adc_init)
FSM_STATE_DEF(TERM_INIT,                term_init)
FSM_STATE_DEF(TERM_MAIN,                term_main)
FSM_STATE_DEF(TERM_RESPOND,             term_respond)
/*
 * Terminal command set for FSM
 */
FSM_STATE_DEF(TERM_CMD_START,           term_cmd_start)
FSM_STATE_DEF(PWM_HANDLER,              pwm_handler)
FSM_STATE_DEF(BTN_HANDLER,              btn_handler)
FSM_STATE_DEF(ADC_HANDLER,              adc_handler)
