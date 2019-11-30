/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  ipc_sched.h
 * \ingroup ipc
 *
 * Scheduler header for inter-processor communications config <br>
 * Header file for ipc background
 *
 */

#ifndef IPC_SCHED_H
#define IPC_SCHED_H

#define IPC_SCHED_TASK(m)

#define IPC_BG_INT(m)                        \
    BG_INT(m, (ipc_rx, ipc_rx_bg))


#endif /* IPC_SCHED_H */
