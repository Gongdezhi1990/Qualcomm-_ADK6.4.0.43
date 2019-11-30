/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/**
 * \file
 *
 * This file has a magic name that causes the build process to spot it.
 *
 * This declares a CARLOS TASK & BG INT that may be used to implement a
 * light weight scheduler in CARLOS environments.
 *
 * \todo    Move the implementation out of hydra_sssm.
 *
 * \todo    Side step all this SCHED/TASK macro stuff if possible.
 */
#ifndef HYDRA_SSSM_SCHED_H
#define HYDRA_SSSM_SCHED_H

#if defined(SUBSYSTEM_BT)

#define HYDRA_SSSM_SCHED_TASK(m)                                   \
    SCHED_TASK_START(m, (sssm, sssm_bt_task_init,                  \
                         sssm_bt_task_msg_handler, RUNLEVEL_BOOT)) \
    SCHED_TASK_QUEUE(m, (sssm, sssm_bt_task_msg_queue_id))         \
    SCHED_TASK_END(m, (sssm))

#define HYDRA_SSSM_BG_INT(m)

#elif defined(SUBSYSTEM_APPS)

#define HYDRA_SSSM_SCHED_TASK(m)                                   \
    SCHED_TASK_START(m, (sssm, sssm_apps_task_init,                  \
                         sssm_apps_task_msg_handler, RUNLEVEL_BOOT)) \
    SCHED_TASK_QUEUE(m, (sssm, sssm_apps_task_msg_queue_id))         \
    SCHED_TASK_END(m, (sssm))

#define HYDRA_SSSM_BG_INT(m)

#elif defined(SUBSYSTEM_AUDIO)

#if defined(AUDIO_SECOND_CORE) && !defined(INSTALL_DUAL_CORE_SUPPORT)

/* Disable sssm task from stand alone second core */
#define HYDRA_SSSM_SCHED_TASK(m)
#define sssm_audio_task_msg_queue_id NO_QID

/*Avoid compiler warnings until compile if off */
void sssm_audio_task_init(void **t);
void sssm_audio_task_msg_handler( void **t);

#else

#define HYDRA_SSSM_SCHED_TASK(m)                                   \
    SCHED_TASK_START(m, (sssm, sssm_audio_task_init,                  \
                         sssm_audio_task_msg_handler, RUNLEVEL_BOOT)) \
    SCHED_TASK_QUEUE(m, (sssm, sssm_audio_task_msg_queue_id))         \
    SCHED_TASK_END(m, (sssm))


#endif 

#define HYDRA_SSSM_BG_INT(m)

#else /* !defined(SUBSYSTEM_BT | SUBSYSTEM_APPS) */

#define HYDRA_SSSM_SCHED_TASK(m)
#define HYDRA_SSSM_BG_INT(m)

#endif /* !defined(SUBSYSTEM_BT) */

#endif /* HYDRA_SSSM_SCHED_H */
