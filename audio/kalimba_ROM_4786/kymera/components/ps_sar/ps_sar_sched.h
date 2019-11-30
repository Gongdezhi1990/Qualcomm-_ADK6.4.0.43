/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file ps_sar_sched.h
 * \ingroup ps_msg
 *
 * Definition of oxygen tasks for ps_sar.
 */
#ifndef PS_SAR_SCHED_H
    #define PS_SAR_SCHED_H

    /* PS_SAR is only used by Hydra parts. */
    #define PS_SAR_SCHED_TASK(m)                                 \
        SCHED_TASK_START(m, (ps_sar, ps_sar_task_init,           \
                             ps_sar_handler, RUNLEVEL_BASIC))    \
        SCHED_TASK_QUEUE(m, (ps_sar, PS_SAR_TASK_QUEUE_ID))      \
        SCHED_TASK_END(m, (ps_sar))
    #define PS_SAR_BG_INT(m)
#endif /* PS_SAR_SCHED_H */