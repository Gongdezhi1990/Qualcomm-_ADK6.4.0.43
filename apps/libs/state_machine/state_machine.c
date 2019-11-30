/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    state_machine.c

DESCRIPTION
    Source file for prototype state patterns.
*/

#include <hydra_macros.h>
#include <panic.h>

#include "state_machine.h"


/* Default transition for any unhandled event that is not handled
   by a custom default state or fsm event handler. */
static sm_transition_t global_default_transition = { 0, 0, smPanicAction };


/*
  @brief Validate the contents of an fsm.

  Performs sanity checks on a newly initialised fsm. Things like:
  * Is the current state valid?

  Possible future checks:
  * Are there any unreachable states?
    * i.e. a transition table row exists but no transitions go to that state.
  * ...
*/
static void smValidateFsm(sm_fsm_t *fsm)
{
    int i;

    /* Check current state is one that actually exists in the
       state transition table. */
    for (i = 0; i < fsm->state_count; i++)
    {
        if (fsm->state_transition_table[i].state == fsm->cur_state)
            break;
    }

    PanicFalse(i < fsm->state_count);
}


void smInitialiseFsm(sm_fsm_t *fsm, const sm_state_t *initial_state,
                     const sm_state_transitions_t *state_transitions, int state_count,
                     const sm_transition_t *default_transition,
                     void *user_ctx)
{
    PanicNull(fsm);
    PanicNull((void *)initial_state);
    PanicNull((void *)state_transitions);

    memset(fsm, 0, sizeof(*fsm));

    fsm->cur_state = initial_state;
    fsm->state_count = state_count;
    fsm->state_transition_table = state_transitions;
    fsm->default_transition = default_transition;
    fsm->user_ctx = user_ctx;

    smValidateFsm(fsm);
}

static const sm_state_transitions_t *getTable(const sm_fsm_t *fsm, const sm_state_t *state)
{
    int i;
    const sm_state_transitions_t *table = 0;

    for (i = 0; i < fsm->state_count; i++)
    {
        if (fsm->state_transition_table[i].state == state)
        {
            table = &fsm->state_transition_table[i];
            break;
        }
    }

    return table;
}

static const sm_transition_t *getTransition(const sm_state_transitions_t *table, sm_event_t event)
{
    int i;
    const sm_transition_t *transition = 0;

    for (i = 0; i < table->transition_count; i++)
    {
        if (table->transitions[i].event == event)
        {
            transition = &table->transitions[i];
            break;
        }
    }

    return transition;
}

static sm_event_t processTransition(sm_fsm_t *fsm, const sm_transition_t *transition)
{
    sm_event_t event = FSM_EVENT_NONE;
    const sm_state_t *end_state = 0;

    /* Perform the transition:
        Call exit function (if it exists)
        Call action function (if it exists)
            Store the event it returns
        Call entry function (if it exists)

        If end_state is 0 then this is an internal transition to the same
        state. Don't call the entry or exit functions for internal transitions.

        Return the event returned by the action function, or FSM_EVENT_NONE
        if there is no action function.
    */
    end_state = transition->end_state;

    if (end_state && fsm->cur_state->exit)
    {
        fsm->cur_state->exit(fsm);
    }

    if (transition->action)
    {
        event = transition->action(fsm);
    }

    if (end_state)
    {
        fsm->cur_state = end_state;

        if (end_state->entry)
        {
            end_state->entry(fsm);
        }
    }

    return event;
}

void smProcessEvent(sm_fsm_t *fsm, sm_event_t event)
{
    const sm_state_transitions_t *table = 0;
    const sm_transition_t *transition = 0;
    sm_event_t next_event = event;

    PanicNull(fsm);
    PanicNull((void *)fsm->cur_state);
    PanicNull((void *)fsm->state_transition_table);

    while (FSM_EVENT_NONE != next_event)
    {
        table = getTable(fsm, fsm->cur_state);

        /* It is fatal in just about all circumstances if we couldn't
           find a transition table for the current state. */
        PanicNull((void *)table);

        transition = getTransition(table, next_event);
        if (!transition)
        {
            /* Use the state default transition if it exists.
               If not, use the fsm default transition if it exists.
               If not, fall-back to the global default transition. */
            transition = table->default_transition ? table->default_transition
                            : (fsm->default_transition ? fsm->default_transition : &global_default_transition);
        }

        /* Process the next_event returned by the transition immediately
           if it is not FSM_EVENT_NONE. */
        next_event = processTransition(fsm, transition);
    }
}

sm_event_t smPanicAction(const sm_fsm_t *fsm)
{
    UNUSED(fsm);

    Panic();

    return FSM_EVENT_NONE;
}

sm_event_t smIgnoreAction(const sm_fsm_t *fsm)
{
    UNUSED(fsm);

    return FSM_EVENT_NONE;
}
