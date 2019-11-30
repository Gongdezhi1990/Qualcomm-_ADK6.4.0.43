/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file accmd_sched.h
 * \ingroup accmd
 * Task definitions for accmd
 */
#ifndef ACCMD_SCHED_H
#define ACCMD_SCHED_H

#if defined(AUDIO_SECOND_CORE) && !defined(INSTALL_DUAL_CORE_SUPPORT)

/* No accmd task for stand alone second core build */
#define ACCMD_SCHED_TASK(m)
#define ACCMD_QUEUE NO_QID

/* Avoid warnings until compile off the handler functions */
void accmd_task_init(void **data);
void accmd_task(void ** task_data);

#else

#define ACCMD_SCHED_TASK(m)                                     \
    SCHED_TASK_START(m, (accmd, accmd_task_init,                \
                         accmd_task, RUNLEVEL_BASIC))          \
    SCHED_TASK_QUEUE(m, (accmd, ACCMD_QUEUE))                   \
    SCHED_TASK_END(m, (accmd))

#endif

#define ACCMD_BG_INT(m)

#endif /* ACCMD_SCHED_H */
