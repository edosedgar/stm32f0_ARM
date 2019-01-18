#include "fsm.h"
#include <stdlib.h>

/*
 * Main control structure for finite state machine
 */
static fsm_ctrl_t fsm_ctrl;

/*
 * Array of cases for finite state machine
 */
#define FSM_STATE_DEF(state_aliase, state_handler) \
        [FSM_##state_aliase] = fsm_##state_handler,
void (* const fsm_states_handlers[])(void *) = {
        #include "fsm_states_defs.h"
};
#undef FSM_STATE_DEF

/*
 * General states handlers implementations
 * To add new command and corresponding handler just
 * add new entry to fsm_states_def.h and implementation
 * below.
 * Note: You are not obliged to implement handlers in this file,
 *       handlers might be implemented whereever upon condition
 *       that 'fsm.h' is included.
 */

/*
 * Dummy state (passthrough)
 */
void fsm_dummy(void *args)
{
        (void) args;

        fsm_set_state(FSM_GLOBAL_INIT);
        return;
}

/*
 * Implementations of fsm methods
 */
void fsm_init(void)
{
        int i = 0;

        fsm_ctrl.state = FSM_DUMMY;
        fsm_ctrl.sleep_state = LOWER_BOUND_CASE;
        /*
         * Turn off all shadow states
         */
        for (i = 0; i < MAX_SHADOW_STATE; i++)
                fsm_ctrl.shadow_state[i] = LOWER_BOUND_CASE;
        fsm_ctrl.states_handlers = fsm_states_handlers;
        return;
}

void fsm_run_state(void)
{
        void *state_data = NULL;
        int state = 0;

        if (IS_STATE_ILL(fsm_ctrl.state))
                fsm_ctrl.state = FSM_ERROR;
        state = fsm_ctrl.state;
        state_data = fsm_ctrl.data[state];
        fsm_ctrl.states_handlers[state](state_data);
        return;
}

void fast_swap(uint32_t *a, uint32_t *b)
{
        *a = *a ^ *b;
        *b = *b ^ *a;
        *a = *a ^ *b;
        return;
}

int is_state_shadow(fsm_ctrl_t *fsm_ctrl, uint32_t state)
{
        int i = 0;

        for (i = 0; i < MAX_SHADOW_STATE; i++) {
                if (fsm_ctrl->shadow_state[i] == state)
                        return i;
        }
        return -1;
}

void fsm_state_mng(void)
{
        int next_shadow = 0;

        /*
         * Handle first call
         */
        if (fsm_ctrl.shadow_state[0] == LOWER_BOUND_CASE)
                return;
        if (fsm_ctrl.sleep_state == LOWER_BOUND_CASE)
                fsm_ctrl.sleep_state = fsm_ctrl.shadow_state[0];
                fast_swap(&(fsm_ctrl.sleep_state), &(fsm_ctrl.state));
                return;
        /*
         * When current state is not shadow just swap
         */
        if ((next_shadow = is_state_shadow(&fsm_ctrl, fsm_ctrl.state)) == -1)
                fast_swap(&(fsm_ctrl.sleep_state), &(fsm_ctrl.state));
                return;
        /*
         * When current state is shadow take next shadow task and swap
         */
        next_shadow %= MAX_SHADOW_STATE;
        if (fsm_ctrl.shadow_state[next_shadow] != LOWER_BOUND_CASE)
                fsm_ctrl.state = fsm_ctrl.shadow_state[next_shadow];
        else
                fsm_ctrl.state = fsm_ctrl.shadow_state[0];
        fast_swap(&(fsm_ctrl.sleep_state), &(fsm_ctrl.state));
        return;
}

int fsm_add_shadow_state(uint32_t shadow_state)
{
        int next_free = 0;

        /*
         * Already added
         */
        if (is_state_shadow(&fsm_ctrl, shadow_state) != -1)
                return -1;
        /*
         * No memory
         */
        if ((next_free = is_state_shadow(&fsm_ctrl, LOWER_BOUND_CASE)) == -1)
                return -1;
        /*
         * Check if valid shadow_state
         */
        if (IS_STATE_ILL(shadow_state))
                return -1;
        fsm_ctrl.shadow_state[next_free] = shadow_state;
        return 0;
}

int fsm_set_state(uint32_t state)
{
        int shadow_index = -1;
        int next_shadow_index = -1;

        shadow_index = is_state_shadow(&fsm_ctrl, fsm_ctrl.state);
        next_shadow_index = is_state_shadow(&fsm_ctrl, state);
        if ((shadow_index == -1) && (next_shadow_index == -1)) {
                fsm_ctrl.state = state;
                return 0;
        }
        if (shadow_index != -1) {
                fsm_ctrl.shadow_state[shadow_index] = state;
                return 0;
        }
        return -1;
}

void fsm_set_data(uint32_t state, void *state_data)
{
        fsm_ctrl.data[state] = state_data;
        return;
}

void *fsm_get_data(uint32_t state)
{
        if (IS_STATE_ILL(state))
                return NULL;
        return fsm_ctrl.data[state];
}
