/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file ipc_deep_sleep.c
 *
 */

#include "ipc/ipc_private.h"
#include "dorm/dorm.h"


void ipc_send_p1_deep_sleep_msg(bool p1_sleep, uint32 earliest_wake_up_time, uint32 latest_wake_up_time)
{
    IPC_P1_DEEP_SLEEP_MSG_PRIM prim;

    prim.p1_sleep              = p1_sleep;
    prim.earliest_wake_up_time = earliest_wake_up_time;
    prim.latest_wake_up_time   = latest_wake_up_time;

    ipc_send(IPC_SIGNAL_ID_P1_DEEP_SLEEP_MSG, (const void *)&prim, sizeof(prim));
}

