/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file opmgr_sched.h
 * \ingroup opmgr
 *
 * Task definitions for opmgr
 */
#ifndef OPMGR_SCHED_H
#define OPMGR_SCHED_H

#define OPMGR_SCHED_TASK(m)                                            \
    SCHED_TASK_START(m, (opmgr, opmgr_task_init,                       \
                         opmgr_task_handler, RUNLEVEL_BASIC))          \
    SCHED_TASK_QUEUE(m, (opmgr, OPMGR_TASK_QUEUE_ID))                  \
    SCHED_TASK_END(m, (opmgr))

#define OPMGR_BG_INT(m)

#endif /* OPMGR_SCHED_H */
