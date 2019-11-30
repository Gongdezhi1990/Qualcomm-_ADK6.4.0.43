/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file fault_sched.h
 * \ingroup fault
 *
 * Scheduler header for fault subsystem.  Only needed to generate
 * a background interrupt to kick the fault publisher.
 */

#ifndef FAULT_SCHED_H
#define FAULT_SCHED_H

#define FAULT_SCHED_TASK(m)

#ifdef FAULT_LEAN_AND_MEAN
#define FAULT_BG_INT(m)
#else /* FAULT_LEAN_AND_MEAN */
#define FAULT_BG_INT(m)                                       \
    BG_INT(m, (fault_publish, publish_faults_bg))             \
    BG_INT(m, (fault_publish_fresh, publish_fresh_faults_bg))
#endif /* FAULT_LEAN_AND_MEAN */

#endif /* FAULT_SCHED_H */
