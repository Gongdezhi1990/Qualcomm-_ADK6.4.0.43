/****************************************************************************
FILE
   appcmd_sched.h  -  scheduler header for APPCMD config

LEGAL
   CONFIDENTIAL
Copyright (c) 2010 - 2016 Qualcomm Technologies International, Ltd.
  

CONTAINS

DESCRIPTION
   Header file for appcmd background
*/

#ifndef APPCMD_SCHED_H
#define APPCMD_SCHED_H

#define CORE_APPCMD_SCHED_TASK(m)                                             \
    SCHED_TASK_START(m, (appcmd_sched_task, appcmd_sched_init,       \
                                  appcmd_sched_msg_handler, RUNLEVEL_BOOT))                              \
    SCHED_TASK_QUEUE(m, (appcmd_sched_task, appcmd_sched_queue_id))       \
    SCHED_TASK_END(m, (appcmd_sched_task))

#define CORE_APPCMD_BG_INT(m) \
    BG_INT(m, (appcmd_bg_handler, appcmd_background_handler))

#endif /* APPCMD_SCHED_H */
