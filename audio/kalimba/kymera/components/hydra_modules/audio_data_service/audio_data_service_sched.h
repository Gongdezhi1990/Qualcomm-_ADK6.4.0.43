/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file audio_data_service_sched.h
 * \ingroup audio_data_service
 *
 * Task definitions for audio_data_service. (Task is only used
 * for EOF forwarding.)
 */

#ifndef AUDIO_DATA_SERVICE_SCHED_H
#define AUDIO_DATA_SERVICE_SCHED_H

#define AUDIO_DATA_SERVICE_SCHED_TASK(m)                                        \
    SCHED_TASK_START(m, (audio_data_service, audio_data_service_task_init,      \
                         audio_data_service_task_handler, RUNLEVEL_BASIC))      \
    SCHED_TASK_QUEUE(m, (audio_data_service, AUDIO_DATA_SERVICE_TASK_QUEUE_ID)) \
    SCHED_TASK_END(m, (audio_data_service))

#define AUDIO_DATA_SERVICE_BG_INT(m)

#endif /* AUDIO_DATA_SERVICE_SCHED_H */
