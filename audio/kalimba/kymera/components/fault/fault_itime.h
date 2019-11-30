/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file fault_itime.h
 * \ingroup fault
 * itime events used for groaning quietly
 */

#ifndef FAULT_ITIME_H
#define FAULT_ITIME_H

/*
 * Handler functions, defined in fault.c
 */
extern void schedule_publish_faults(void);
extern void publish_faults(void);

/** Define the itime event identifier for this subsystem */
#if (!(defined(P0_LEAN_AND_MEAN)))
#define FAULT_ITIME_EVENT(m)                                            \
    ITIME_EVENT_LO(m, (itid_fault_schedule, schedule_publish_faults))   \
    ITIME_EVENT_LO(m, (itid_fault_publish, publish_faults))
#else
#define FAULT_ITIME_EVENT(m)
#endif

#endif /* FAULT_ITIME_H */
