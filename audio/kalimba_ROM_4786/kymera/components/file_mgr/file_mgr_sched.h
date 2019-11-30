/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  file_mgr_sched.h
 * \ingroup file_mgr
 *
 *  File Download Manager scheduler header file
 */

#ifndef FILE_MGR_SCHED_H_
#define FILE_MGR_SCHED_H_

#define FILE_MGR_SCHED_TASK(m)                                           \
    SCHED_TASK_START(m, (FILE_MGR, file_mgr_task_init,                   \
                        file_mgr_task_handler, RUNLEVEL_BASIC)) \
    SCHED_TASK_QUEUE(m, (FILE_MGR, FILE_MGR_TASK_QUEUE_ID))              \
    SCHED_TASK_END(m, (FILE_MGR))

#define FILE_MGR_BG_INT(m)

#endif /* FILE_MGR_SCHED_H */
