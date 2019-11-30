/*************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * All Rights Reserved.
 * Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*************************************************************************/

#ifdef INSTALL_AOV
#ifndef AOV_TASK_H
#define AOV_TASK_H

#include "aov.h"
#include "opmgr/opmgr_for_ops.h"
#include "panic/panic.h"
#include "pl_timers/pl_timers.h"
#include "sched_oxygen/sched_oxygen.h"
#include "subserv/subserv.h"
#include "utils/utils_fsm.h"

#define AOVSM_ID (0x1E)

// The final operator in the AOV graph won't have a source terminal
// The total number of endpoints in the AOV graph would be 2 * num_ops_in_aov_graph
#define AOV_EP_MAX (AOV_OP_MAX * 2)

extern const struct aovsm_fsm_struct
{
    utils_fsmtype base;
    struct
    {
        utils_fsmevent_type configure_trigger;
        utils_fsmevent_type lp_activate;
        utils_fsmevent_type op_trigger_positive;
        utils_fsmevent_type op_trigger_negative;
        utils_fsmevent_type clk_transition_success;
        utils_fsmevent_type clk_transition_failure;
        utils_fsmevent_type op_response_success;
        utils_fsmevent_type op_response_failure;
        utils_fsmevent_type lp_deactivate;
        utils_fsmevent_type duty_finish;
        utils_fsmevent_type duty_begin;
        utils_fsmevent_type vad_start_success;
        utils_fsmevent_type vad_start_failure;
        utils_fsmevent_type vad_stop_success;
        utils_fsmevent_type vad_stop_failure;
        utils_fsmevent_type stop;
    } events;

    struct
    {
        utils_fsmstate idle;
        utils_fsmstate error;

        // active
        utils_fsmstate am_configuring_vad_for_proc;
        utils_fsmstate am_configuring_vtd_for_purge;
        utils_fsmstate am_configuring_vci_for_purge;
        utils_fsmstate am_wait_for_vad_trigger;
        utils_fsmstate am_configuring_vtd_for_proc;
        utils_fsmstate am_wait_for_vtd_trigger;
        utils_fsmstate am_configuring_vci_for_proc;
        utils_fsmstate am_wait_for_vci_trigger;
        utils_fsmstate am_client_notified;
        utils_fsmstate am_lp_activate_pending_op_response;
        utils_fsmstate am_stop_pending_op_response;

        // lp-->lp_wake_up-->active
        utils_fsmstate set_up_lp_and_configuring_vad_for_proc;
        utils_fsmstate configuring_vtd_for_purge;
        utils_fsmstate configuring_vci_for_purge;
        utils_fsmstate entering_lp;
        utils_fsmstate in_lp;
        utils_fsmstate stopping_vad;
        utils_fsmstate duty_finished;
        utils_fsmstate starting_vad;
        utils_fsmstate entering_lp_wake_up;
        utils_fsmstate configuring_vtd_for_proc;
        utils_fsmstate in_lp_wake_up;
        utils_fsmstate entering_active_on_trigger;
        utils_fsmstate configuring_vci_for_proc;
        utils_fsmstate wait_for_vci_trigger;
        utils_fsmstate client_wake_notified;
        utils_fsmstate client_registered;
        utils_fsmstate entering_active_on_request;
        utils_fsmstate lp_deactivate_pending_clk_transition;
        utils_fsmstate lp_deactivate_pending_op_response;
        utils_fsmstate lp_deactivate_pending_vad_stop_start;
        utils_fsmstate lp_deactivate_pending_vad_start;
        utils_fsmstate vad_start_pending_vad_stop;
        utils_fsmstate leave_lp_pending_vad_start;
    } states;
} aovsm_fsm;

typedef struct aovsm_graph_struct
{
    struct ENDPOINT * eps[AOV_EP_MAX];   // endpoint list
    unsigned ops[AOV_OP_MAX];            // external op ids of the aov operators
} aovsm_graph;

extern struct aovsm_data_struct
{
    unsigned wake_op_id;            // external op id of the wake-on operator
    AOV_OP   wake_aov_op;           // vad, vtd or vci
    unsigned client_id;             // accmd client id
    aovsm_graph graph;              // aov graph
    unsigned lp_count;              // number of times we have entered lp post a request to enter lp.
                                    // reset to 0 on client registration
    AOV_DUTY_PARAMS duty_params;    // duty cycle parameters
    tTimerId duty_timer_id;         // work/idle timer
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

#endif // AOV_TASK_H
#endif // INSTALL_AOV
