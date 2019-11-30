/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cap_download_mgr_sched.h
 * \ingroup  cap_download_mgr
 *
 * Capability Download Manager scheduler header file
 */

#ifndef CAP_DOWNLOAD_MGR_SCHED_H_
#define CAP_DOWNLOAD_MGR_SCHED_H_

#define CAP_DOWNLOAD_MGR_SCHED_TASK(m)                                         \
    SCHED_TASK_START(m, (CAP_DOWNLOAD_MGR, cap_download_mgr_task_init,                   \
                         cap_download_mgr_task_handler, RUNLEVEL_BASIC))                 \
    SCHED_TASK_QUEUE(m, (CAP_DOWNLOAD_MGR, CAP_DOWNLOAD_MGR_TASK_QUEUE_ID))    \
    SCHED_TASK_END(m, (CAP_DOWNLOAD_MGR))

#define CAP_DOWNLOAD_MGR_BG_INT(m)

#endif /* CAP_DOWNLOAD_MGR_SCHED_H */
