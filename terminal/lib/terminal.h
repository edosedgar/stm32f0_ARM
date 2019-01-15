#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <stdint.h>

#define TERM_CH_LENGTH 255
#define TERM_CMD_LENGTH 10

/*
 * Checking input command code
 */
#define IS_COMMAND_VALID(com)   (((com) > LOWER_BOUND_CASE) && \
                                ((com) < UPPER_BOUND_CASE - \
                                        FSM_TERM_CMD_START))

/*
 * Flags for terminal
 */
#define RX_COMPLETE 0x01


#define is_term_flag_set(term_ctrl, bit) \
        (term_ctrl.flags & bit)

#define term_set_flag(term_ctrl, bit)  \
        term_ctrl.flags |= bit

#define term_clr_flag(term_ctrl, bit) \
        term_ctrl.flags &= (~bit)

typedef struct {
        uint8_t channel[TERM_CH_LENGTH];
        uint8_t params[TERM_CMD_LENGTH];
        uint8_t flags;
} term_ctrl_t;

#endif
