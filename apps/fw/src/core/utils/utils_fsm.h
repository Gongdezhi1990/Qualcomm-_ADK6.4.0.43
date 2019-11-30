/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Table-Driven Finite State Machine (interface).
 */

#ifndef UTILS_FSM_H
#define UTILS_FSM_H

/*****************************************************************************
 * Interface Dependencies
 ****************************************************************************/

#include "utils/utils_event.h"
#include "hydra/hydra_macros.h" /* NULL! */

/*****************************************************************************
 * Public Type Declarations
 ****************************************************************************/

/**
 * Table-Driven Extended State Machine Type.
 *
 * Features:-
 * - Sparse
 * - Ordered Transition Guards. N.B. The first transition in list
 * with valid guard (or null guard) will be taken.
 * - State Entry & Exit actions.
 * - Internal & External Transitions.
 * - Typesafe (no enums)
 * - Stand-alone.
 * - State Change signalling
 *
 * The engine is const-data-greedy. It is designed for ease of
 * hand-coding UML-like FSMs.
 *
 * It is expected that some FSM's initially coded to use this engine
 * will be reversed into a generator to target different engines according to
 * specific product optimisation requirements.
 *
 * Todo:-
 * - Extend CCL fsm to support compatible model and
 * generate compatible tables.
 * - Extend CCL fsm to generate alternative implementations (e.g. "code
 * switch").
 * - Consider adding submachines and unifying with SME/wifi engine.
 */
typedef struct utils_fsmtype utils_fsmtype;

/** Table-Driven Extended State Machine Instance (Abstract Base). */
typedef struct utils_fsm utils_fsm;

/** Event (Abstract Base) */
typedef struct utils_fsmevent utils_fsmevent;

/** State (Type) */
typedef struct utils_fsmstate utils_fsmstate;

/** Initial action. */
typedef void (*utils_fsminit)(utils_fsm *fsm);

/** State entry action */
typedef void (*utils_fsmentry)(utils_fsm *fsm);

/** State leave action */
typedef void (*utils_fsmexit)(utils_fsm *fsm);

/** Event Type */
typedef struct utils_fsmevent_type utils_fsmevent_type;

/** Transition */
typedef struct utils_fsmtransition utils_fsmtransition;

/** Transition Guard */
typedef bool (*utils_fsmguard)(const utils_fsm *fsm, const utils_fsmevent *event);

/** Transition Action/Effect */
typedef void (*utils_fsmaction)(utils_fsm *fsm, const utils_fsmevent *event);

/** FSM initialisation request event */
typedef struct utils_fsm_init_req utils_fsm_init_req;

/*****************************************************************************
 * Public Type Definitions
 ****************************************************************************/

struct utils_fsm
{
    /** \protected Debug name (storage not linked) */
    const char *name;

    /** \protected Type. */
    const utils_fsmtype *type;

    /** \protected Current State. */
    const utils_fsmstate *state;

    /** State Changed Event. */
    utils_Event state_changed_event;
};

struct utils_fsmtype
{
    /** \protected Debug name (storage not linked) */
    const char *name;

    /** \protected Initial state. */
    const utils_fsmstate *initial_state;

    /** \protected Initial action (or NULL). */
    utils_fsminit init;
};

struct utils_fsmstate
{
    /** \protected Debug name (storage not linked) */
    const char *name;

    /** \protected State entry action (or NULL) */
    utils_fsmentry enter;

    /** \protected State leave action (or NULL) */
    utils_fsmexit leave;

    /** \protected Table of transitions from this State. */
    const utils_fsmtransition *transitions;
    size_t num_transitions;
};

struct utils_fsmtransition
{
    /** \protected Triggering event type */
    const utils_fsmevent_type *trigger;

    /** \protected Guard condition or NULL to indicate unconditional */
    utils_fsmguard guard;

    /** \protected Action/effect or NULL to indicate no action */
    utils_fsmaction action;

    /** \protected Destination state or NULL to indicate internal transition */
    const utils_fsmstate *to;
};

struct utils_fsmevent_type
{
    /** \protected Debug name (storage not linked) */
    const char *name;
};

struct utils_fsmevent
{
    /** \protected Thie Event's Type. */
    const utils_fsmevent_type *type;
};

struct utils_fsm_init_req
{
    utils_fsmevent base;
};

/*****************************************************************************
 * Public Data Declarations
 ****************************************************************************/

/**
 * Reserved init event type.
 *
 * Triggers entry of FSM's "initial" state when applied to FSM.
 *
 * Must be applied to FSM after utils_fsm_init and before any other events.
 *
 * By defining an init event the handling can be done in FSM's normal event
 * delivery context - rather than the initiliaser's context.
 */
extern const utils_fsmevent_type utils_fsm_init_req_type;

/*****************************************************************************
 * Public Function Declarations
 ****************************************************************************/

/**
 * Declare logable string for use with utils_fsm.
 *
 * This is a wrapper on HYDRA_LOG_STRING_DECL that ensures there is at least
 * and empty string (1char) even when log is not installed.
 *
 * If the a few words storage waste is a concern then should not be
 * targetting this FSM!
 */
#if defined(INSTALL_HYDRA_LOG)
/* use the hydra log decl */
#define UTILS_FSM_STRING(label, text) HYDRA_LOG_STRING(label, text)
#else /* !defined(INSTALL_HYDRA_LOG) */
/* delcare a dummy null string */
#define UTILS_FSM_STRING(label, text) static const char label[] = ""
#endif /* !defined(INSTALL_HYDRA_LOG) */

/**
 * Initialise machine instance.
 *
 * N.B. Entry of initial state does not happen till event of
 * type utils_fsm_init_event_type is applied to the FSM.
 *
 * \public \memberof utils_fsm
 */
extern void utils_fsm_init(utils_fsm *fsm);

/**
 * Get FSM's (debug) name.
 *
 * \note The string storage is NOT linked. The address can be used as arg.
 * to debug messages.
 *
 * \public memberof utils_fsm
 *
 * \note
 * Implemented as a macro
 */
#ifdef DOXYGEN_ONLY
extern const char *utils_fsm_name(const utils_fsm *fsm);
#else /* DOXYGEN_ONLY */
#define utils_fsm_name(fsm) \
    ((fsm)->name)
#endif /* DOXYGEN_ONLY */

/**
 * Get FSM's current state.
 *
 * \public \memberof utils_fsm
 *
 * \note
 * Implemented as a macro
 */
#ifdef DOXYGEN_ONLY
extern const utils_fsmstate *utils_fsm_state(const utils_fsm *fsm);
#else /* DOXYGEN_ONLY */
#define utils_fsm_state(fsm) \
    ((fsm)->state)
#endif /* DOXYGEN_ONLY */

/**
 * Get ptr to this FSM's state changed event.
 *
 * \public memberof utils_fsm
 *
 * \note
 * Implemented as a macro
 */
#ifdef DOXYGEN_ONLY
extern utils_Event *utils_fsm_state_changed_event(const utils_fsm *fsm);
#else /* DOXYGEN_ONLY */
#define utils_fsm_state_changed_event(fsm) \
    (&(fsm)->state_changed_event)
#endif /* DOXYGEN_ONLY */

/**
 * Apply event to machine
 *
 * \public \memberof utils_fsm
 */
extern void utils_fsm_apply_event(utils_fsm *fsm, const utils_fsmevent *e);

/**
 * Initialise an initialisation request.
 *
 * \public memberof utils_fsm_init_req
 */
#define utils_fsm_init_req_init(r) \
    ((void)((r)->base.type = &utils_fsm_init_req_type))

/**
 * Is this an external transition (c.f. internal)
 *
 * \public \memberof utils_fsmtransition
 *
 * \note
 * Implemented as a macro
 */
#ifdef DOXYGEN_ONLY
extern bool utils_fsmtransition_is_external(const utils_fsmtransition *tx);
#else /* DOXYGEN_ONLY */
#define utils_fsmtransition_is_external(tx) \
    (NULL != (tx)->to)
#endif /* DOXYGEN_ONLY */

/*****************************************************************************
 * Public Function Definitions (== Macros)
 ****************************************************************************/

#endif /* UTILS_FSM_H */
