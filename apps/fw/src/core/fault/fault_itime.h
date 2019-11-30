/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * itime event used for publishing faults after a reasonable delay.
 */

#ifndef FAULT_ITIME_H
#define FAULT_ITIME_H

/*
 * Handler functions, defined in fault.c
 */
extern void publish_faults(void);

/** Define the itime event identifier for this subsystem */
#if (!(defined(P0_LEAN_AND_MEAN)))
#define CORE_FAULT_ITIME_EVENT(m)                                            \
    ITIME_EVENT_LO(m, (itid_fault_publish, publish_faults))
#else
#define FAULT_ITIME_EVENT(m)
#endif

#endif /* FAULT_ITIME_H */
