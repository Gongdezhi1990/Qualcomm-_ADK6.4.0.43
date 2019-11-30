/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 *
 * Scheduler header for fault subsystem.  Only needed to generate
 * a background interrupt to kick the fault publisher.
 */

#ifndef FAULT_SCHED_H
#define FAULT_SCHED_H

#define CORE_FAULT_SCHED_TASK(m)

#define CORE_FAULT_BG_INT(m)                                       \
    BG_INT(m, (fault_publish, publish_faults_bg))             \

#endif /* FAULT_SCHED_H */
