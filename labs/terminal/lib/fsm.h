#ifndef _H_FSM_H_
#define _H_FSM_H_

#include <stdint.h>

#define MAX_SHADOW_STATE 5

/*
 * Generate enum of states available in
 * fsm_defs.h
 * Note: All commands start with 'FSM_' prefix
 */
#define FSM_STATE_DEF(state_aliase, state_handler) \
        FSM_##state_aliase,
enum {
        LOWER_BOUND_CASE,
        #include "fsm_states_defs.h"
        UPPER_BOUND_CASE
};
#undef FSM_STATE_DEF

/*
 * Declare list of handlers prototypes
 * Note: all handler function names start
 * with 'fsm_' prefix
 */
#define FSM_STATE_DEF(state_aliase, state_handler) \
void fsm_##state_handler(void *args);
#include "fsm_states_defs.h"
#undef FSM_STATE_DEF

#define IS_STATE_ILL(state) \
        ((state <= LOWER_BOUND_CASE) || state >= UPPER_BOUND_CASE)

/*
 * Extern the array of handlers
 */
extern void (* const fsm_states_handlers[])(void *);

typedef struct {
        uint32_t state;
        uint32_t sleep_state;
        uint32_t shadow_state[MAX_SHADOW_STATE];
        void *data[UPPER_BOUND_CASE];
        void (* const *states_handlers)(void *);
} fsm_ctrl_t;

/*
 * Main set of fsm methods
 */

/*
 * Initialization of finite state machine
 */
void fsm_init(void);

/*
 * Runner of states in fsm
 */
void fsm_run_state(void);

/*
 * Main finite state machine manager
 */
void fsm_state_mng(void);

/*
 * Add shadow state
 */
int fsm_add_shadow_state(uint32_t shadow_state);

/*
 * Set state to be called
 * Note: normal states cannot call shadow states
 *       directly, only through
 *       fsm_add_shadow_state
 *       If you do so the function ends up with error -1
 */
int fsm_set_state(uint32_t state);

/*
 * The function sets data of the state specified
 */
void fsm_set_data(uint32_t state, void *state_data);

/*
 * The function returns data of the state specified
 */
void *fsm_get_data(uint32_t state);

#endif
