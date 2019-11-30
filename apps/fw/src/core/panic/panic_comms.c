/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Implements remote panic reporting.
 */ 

#include "panic/panic_private.h"
#include "hydra_log/hydra_log.h"


void panic_from_p0(uint16 id, uint32 t, uint32 arg)
{
    UNUSED(t);
    UNUSED(arg);
    L2_DBG_MSG2("Received panic from P0: ID %d, arg 0x%08x", id, arg);
    panic_diatribe(PANIC_P0_PANICKED, id);
}

void panic_report(panicid id, uint32 t, DIATRIBE_TYPE arg, uint16 cpu)
{

    UNUSED(cpu);
#ifdef IPC_MODULE_PRESENT
    /* Don't inform P0 that it has panicked */
    if (id != PANIC_P0_PANICKED)
    {
        ipc_send_panic(id, t, arg);
    }
#else /* IPC_MODULE_PRESENT */
    UNUSED(id);
    UNUSED(t);
    UNUSED(arg);
#endif /* IPC_MODULE_PRESENT */
}

