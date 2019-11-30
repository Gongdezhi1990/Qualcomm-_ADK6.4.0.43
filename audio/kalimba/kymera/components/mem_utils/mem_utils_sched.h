/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file 
 * \ingroup mem_utils
 * Task definitions for mem_utils. Used for exported constant support.
 */
#ifndef MEM_UTILS_SCHED_H
#define MEM_UTILS_SCHED_H

#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT

#if defined(AUDIO_SECOND_CORE) && !defined(INSTALL_DUAL_CORE_SUPPORT)

/* No mem_utils task for second core build */
#define MEM_UTILS_SCHED_TASK(m)
#define MEM_UTILS_QUEUE NO_QID

/* Avoid warnings until compile off the handler functions */
void mem_utils_task_init(void **data);
void mem_utils_task(void ** task_data);

#else

#define MEM_UTILS_SCHED_TASK(m)                                     \
    SCHED_TASK_START(m, (mem_utils, mem_utils_task_init,                \
                         mem_utils_task, RUNLEVEL_BASIC))          \
    SCHED_TASK_QUEUE(m, (mem_utils, MEM_UTILS_QUEUE))                   \
    SCHED_TASK_END(m, (mem_utils))

#endif

#else
#define MEM_UTILS_SCHED_TASK(m)
#define MEM_UTILS_QUEUE NO_QID

#endif /* INSTALL_CAPABILITY_CONSTANT_EXPORT */

#define MEM_UTILS_BG_INT(m)

#endif /* MEM_UTILS_SCHED_H */
