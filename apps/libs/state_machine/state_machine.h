/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    state_machine.h

DESCRIPTION
    Data types for defining a table based state machine and functions to
    process events for it.
*/

/*!
@brief  Data types and functions for a VM application state machine.
@section intro INTRODUCTION


*/

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <hydra_macros.h>
#include <stdlib.h>
#include <vmtypes.h>


/*
 * generic finite state machine types
 */

typedef uint16 sm_event_t;

#define FSM_EVENT_NONE ((sm_event_t) 0xFFFF)

/* Forward declaration so it can be used in the function pointer types. */
struct sm_fsm;

/*!
  @brief Definition of the state entry function.

  When moving to a state the entry function will be called (if it exists).

  @param fsm FSM object that is entering this state.
*/
typedef void (*sm_entry_t)(const struct sm_fsm *fsm);

/*!
  @brief Definition of the state exit function.

  When performing a state transition the exit function for the current state
  is called first, before any action or entry function.

  @param fsm FSM object that is exiting this state.
*/
typedef void (*sm_exit_t)(const struct sm_fsm *fsm);

/*!
  @brief Definition of the action function.

  When performing a state transition the action function for the transition
  will be called after the exit function for the current state and before
  the entry function for the new end state.

  @param fsm FSM object that this action is for.
*/
typedef sm_event_t (*sm_action_t)(const struct sm_fsm *fsm);


/*! @brief A FSM state */
typedef struct sm_state
{
    /*! @brief Function to call when entering this state. May be 0. */
    sm_entry_t entry;

    /*! @brief Function to call when exiting this state. May be 0. */
    sm_exit_t exit;
} sm_state_t;


/*! @brief A Transition */
typedef struct sm_transition
{
    /*! @brief The event id that triggers this transition. */
    sm_event_t event;

    /*! @brief The end state after this transition. */
    const sm_state_t *end_state;

    /*! @brief The action to perform during the transition. */
    sm_action_t action;
} sm_transition_t;


/*! @brief A table of all the transitions for a particular state. */
typedef struct sm_state_transitions
{
    /*! @brief State this transition table is for. */
    const struct sm_state *state;

    /*! @brief Number of rows in transitions. */
    int transition_count;

    /*! @brief Array of transitions for handled events. */
    sm_transition_t *transitions;

    /*! @brief Default transition for unhandled events. */
    sm_transition_t *default_transition;
} sm_state_transitions_t;


/*! @brief The main FSM object. */
typedef struct sm_fsm
{
    /*! @brief Current state of the FSM. */
    const sm_state_t *cur_state;

    /*! @brief Number of rows in state_transition_table. */
    int state_count;

    /*! @brief State transition table. Has one row per state. */
    const sm_state_transitions_t *state_transition_table;

    /*! @brief Default transition for unhandled events. */
    const sm_transition_t *default_transition;

    /*! @brief User specific data that exists outside of the FSM. */
    void *user_ctx;
} sm_fsm_t;

/******************************************************************************
 * Helper macros
 *****************************************************************************/

/* Naming conventions */
#define SM_STATE_NAME(name) name##_state
#define SM_STATE_ENTRY_NAME(name) name##_entry
#define SM_STATE_EXIT_NAME(name) name##_exit
#define SM_STATE_TRANSITIONS_NAME(name) name##_transitions

/* Actions */
#define SM_ACTION(name) sm_event_t (name)(const struct sm_fsm *fsm)

/* States */
#define SM_STATE(name) \
const sm_state_t SM_STATE_NAME(name) = { \
    0, \
    0 \
}

#define SM_STATE_WITH_ENTRY_ONLY(name) \
void SM_STATE_ENTRY_NAME(name)(const struct sm_fsm *fsm); \
    \
const sm_state_t SM_STATE_NAME(name) = { \
    SM_STATE_ENTRY_NAME(name), \
    0 \
}

#define SM_STATE_WITH_EXIT_ONLY(name) \
void SM_STATE_EXIT_NAME(name)(const struct sm_fsm *fsm); \
    \
const sm_state_t SM_STATE_NAME(name) = { \
    0, \
    SM_STATE_EXIT_NAME(name), \
}

#define SM_STATE_WITH_ENTRY_EXIT(name) \
void SM_STATE_ENTRY_NAME(name)(const struct sm_fsm *fsm); \
void SM_STATE_EXIT_NAME(name)(const struct sm_fsm *fsm); \
    \
const sm_state_t SM_STATE_NAME(name) = { \
    SM_STATE_ENTRY_NAME(name), \
    SM_STATE_EXIT_NAME(name) \
}

/* Transitions */
#define SM_TRANSITIONS_START(name) sm_transition_t SM_STATE_TRANSITIONS_NAME(name)[] = {

#define SM_TRANSITION(event, end_state, action) { event, &SM_STATE_NAME(end_state), action }
#define SM_TRANSITION_INTERNAL(event, action) { event, 0, action }

#define SM_TRANSITIONS_END() }

/* Transition tables */
#define SM_TRANSITION_TABLE_START(name) const sm_state_transitions_t (name##_table)[] = {

#define SM_TRANSITION_TABLE_ENTRY(name) { &SM_STATE_NAME(name), ARRAY_DIM(SM_STATE_TRANSITIONS_NAME(name)), SM_STATE_TRANSITIONS_NAME(name), 0 }
#define SM_TRANSITION_TABLE_ENTRY_DEFAULT(name) { &SM_STATE_NAME(name), ARRAY_DIM(SM_STATE_TRANSITIONS_NAME(name)), SM_STATE_TRANSITIONS_NAME(name), SM_STATE_TRANSITIONS_NAME(name##_default) }

#define SM_TRANSITION_TABLE_END() }


/******************************************************************************
 * End of helper macros
 *****************************************************************************/

/*!
    @brief Initialise a state machine instance.

    @param fsm The state machine to initialise.
    @param initial_state Initial state of the state machine. Must not be 0
    @param state_transitions Table containing all the transitions per state.
                There must be one entry per state.
    @param state_count Number of states in the transition table.
    @param default_transition Default transition to use for unhandled events
                if the current state does not have its own default transition.
                This can be 0, in which case the library default transition is
                used instead.
    @param user_ctx User data that can be accessed in action functions.
*/
void smInitialiseFsm(sm_fsm_t *fsm, const sm_state_t *initial_state,
                     const sm_state_transitions_t *state_transitions, int state_count,
                     const sm_transition_t *default_transition,
                     void *user_ctx);

/*!
    @brief Process an event for a state machine instance.

    @param fsm The state machine that will handle the event.
    @param event Event to send to the state machine.
*/
void smProcessEvent(sm_fsm_t *fsm, sm_event_t event);


/*!
    @brief An action function that always panics.

    If logging is enabled it will output some info before it panics.

    Note: This is the default transition action for any state machine that
    does not define its own default transition.
 */
sm_event_t smPanicAction(const sm_fsm_t *fsm);

/*!
    @brief An action function that does nothing.

    This will output some info if logging is enabled but otherwise it does
    nothing.
*/
sm_event_t smIgnoreAction(const sm_fsm_t *fsm);


#endif // STATE_MACHINE_H
