/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Implements remote fault reporting.
 */ 

#include "fault/fault_private.h"
#include "fault/fault_appcmd.h"



void groan(const faultinfo *fi, uint16 cpu)
{
    DIATRIBE_TYPE arg = fi->arg;
    UNUSED(cpu);
#ifdef IPC_MODULE_PRESENT
    ipc_send_fault((uint16)fi->f, (uint16)fi->n, fi->st, arg);
#else /* IPC_MODULE_PRESENT */
    UNUSED(fi);
    UNUSED(arg);
#endif /* IPC_MODULE_PRESENT */

    /* For testing purposes. See fault_appcmd.c */
    record_last_fault();
}


