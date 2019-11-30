/*************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * All Rights Reserved.
 * Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*************************************************************************/

#ifndef AOV_TASK_SCHED_H
#define AOV_TASK_SCHED_H

#define AOV_TASK_SCHED_TASK(m)                                        \
    SCHED_TASK_START(m, (aov_task, aov_task_init,      \
                         aov_task_msg_handler, RUNLEVEL_BASIC))      \
    SCHED_TASK_QUEUE(m, (aov_task, AOV_TASK_QUEUE_ID)) \
    SCHED_TASK_END(m, (aov_task))

#define AOV_TASK_BG_INT(m)

#endif /* AOV_SCHED_H */
