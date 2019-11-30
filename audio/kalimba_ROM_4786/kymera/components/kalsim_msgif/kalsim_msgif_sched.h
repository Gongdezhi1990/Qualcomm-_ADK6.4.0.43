/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file
 *
 * Scheduler header for kalsim_msgif.
 */

#ifndef KALSIM_MSGIF_SCHED_H
#define KALSIM_MSGIF_SCHED_H

#define KALSIM_MSGIF_SCHED_TASK(m)                                     \
    SCHED_TASK_START(m, (kalsim_msgif, kalsim_msgif_task_init,         \
                         kalsim_msgif_task, RUNLEVEL_BASIC))           \
    SCHED_TASK_QUEUE(m, (kalsim_msgif, KALSIM_MSGIF_QUEUE))            \
    SCHED_TASK_END(m, (kalsim_msgif))

#define KALSIM_MSGIF_BG_INT(m)                                       \
    BG_INT(m, (kalsim_msgif, kalsim_msgif_bg_int_handler))

#endif /* KALSIM_MSGIF_SCHED_H */
