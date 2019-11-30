/*************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * All Rights Reserved.
 * Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*************************************************************************/
#ifndef AOV_TASK_H
#define AOV_TASK_H

#ifdef INSTALL_AOV

#include "aov.h"
#include "opmgr/opmgr_for_ops.h"
#include "panic/panic.h"
#include "pl_timers/pl_timers.h"
#include "sched_oxygen/sched_oxygen.h"
#include "subserv/subserv.h"
#include "utils/utils_fsm.h"

#ifdef AOV_INSTALL_DUTY_CYCLE
/** Default duty cycle periods in microseconds */
#define AOVSM_DEFAULT_LP_WORK_US (30000)
#define AOVSM_DEFAULT_LP_IDLE_US (70000)

/** AOV duty cycle parameters */
typedef struct AOV_DUTY_PARAMS_STRUCT
{
    unsigned work_us; /** trigger detection on for work_us microseconds */
    unsigned idle_us; /** idle for idle_us microseconds */

} AOV_DUTY_PARAMS;
#endif

// The final operator in the AOV graph won't have a source terminal

extern const struct aovsm_fsm_struct
{
    utils_fsmtype base;
    struct
    {
        utils_fsmevent_type configure_trigger;
        utils_fsmevent_type lp_activate;
        utils_fsmevent_type op_trigger_positive;
        utils_fsmevent_type use_custom_clock;
        utils_fsmevent_type use_default_clock;
        utils_fsmevent_type clk_transition_success;
        utils_fsmevent_type clk_transition_failure;
        utils_fsmevent_type lp_deactivate;
#ifdef AOV_INSTALL_DUTY_CYCLE
        utils_fsmevent_type duty_finish;
        utils_fsmevent_type duty_begin;
#endif
        utils_fsmevent_type stop;
    } events;

    struct
    {
        utils_fsmstate idle;
        utils_fsmstate error;

        // active
        utils_fsmstate am_changing_to_custom_clk;
        utils_fsmstate am_changing_to_default_clk;
        utils_fsmstate am_in_default_clock;
        utils_fsmstate am_in_custom_clock;
        utils_fsmstate am_lp_activate_pending_clk_transition;

        // lp-->lp_custom-->active
        utils_fsmstate entering_lp;
        utils_fsmstate in_lp;
#ifdef AOV_INSTALL_DUTY_CYCLE
        utils_fsmstate duty_finished;
#endif
        utils_fsmstate entering_lp_custom;
        utils_fsmstate in_lp_custom;
        utils_fsmstate back_to_lp;
        utils_fsmstate entering_active_on_trigger;
        utils_fsmstate client_wake_notified;
        utils_fsmstate client_registered;
        utils_fsmstate entering_active_on_request;
        utils_fsmstate lp_deactivate_pending_clk_transition;
    } states;
} aovsm_fsm;

extern struct aovsm_data_struct
{
    unsigned wake_op_id;            // external op id of the wake-on operator
    AOV_OP   wake_aov_op;           // vad, vtd
    unsigned client_id;             // accmd client id
    unsigned graph_op_id;           // aov graph operator id
    unsigned lp_count;              // number of times we have entered lp post a request to enter lp.
                                    // reset to 0 on client registration
#ifdef AOV_INSTALL_DUTY_CYCLE
    AOV_DUTY_PARAMS duty_params;    // duty cycle parameters
    tTimerId duty_timer_id;         // work/idle timer
#endif
    unsigned negative_vtd_triggers; // count of negative vtd triggers (in lp mode) from boot
} aovsm_data;

#ifdef INSTALL_AOV_TASK_TEST
// so the test can poke around
extern utils_fsm aovsm;
#endif

/**
 * Panics with panicid if cond is FALSE
 */
#define AOV_PANIC_IF_FALSE(cond, panicid)  ((cond) ? ((void)0) : panic_diatribe(panicid, 0))

#endif // INSTALL_AOV

#endif // AOV_TASK_H
