/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/**
 * \file
 * Command interpreter for accessing the mib
 */
#ifndef MIBCMD_SCHED_H
#define MIBCMD_SCHED_H

#ifdef MIBCMD_USES_SCHED_TASK

#define MIBCMD_SCHED_TASK(m)                                            \
    SCHED_TASK_START(m, (mibcmd, init_mibcmd,                           \
                         mibcmd_task_handler, RUNLEVEL_BASIC))          \
    SCHED_TASK_QUEUE(m, (mibcmd, mibcmd_qid))                           \
    SCHED_TASK_END(m, (mibcmd))

#else /* MIBCMD_USES_SCHED_TASK */

#define MIBCMD_SCHED_TASK(m)

#endif /* MIBCMD_USES_SCHED_TASK */

#define MIBCMD_BG_INT(m)

#endif /* MIBCMD_SCHED_H */
