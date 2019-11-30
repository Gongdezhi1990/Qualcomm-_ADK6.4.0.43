/* Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 */
#include "ipc/ipc_private.h"


#include <message.h>

void ipc_memory_access_fault_handler(IPC_SIGNAL_ID id, const void *msg)
{
    switch(id)
    {
    case IPC_SIGNAL_ID_MEMORY_ACCESS_FAULT_INFO:
    {
        const IPC_MEMORY_ACCESS_FAULT_INFO *fault_msg = msg;
        L0_DBG_MSG("ipc_memory_access_fault_handler: CPU1 access interrupt occurred");
        L0_DBG_MSG3("ipc_memory_access_fault_handler: FaultType 0x%x FaultAddress 0x%x FaultArea 0x%x",
            fault_msg->type,fault_msg->address,fault_msg->areaidx);
        L0_DBG_MSG1("ipc_memory_access_fault_handler: P1 ProgramCounter 0x%08x", fault_msg->pc); 
        panic_diatribe(PANIC_KALIMBA_MEMORY_EXCEPTION, fault_msg->address);
    }

    default:
        L1_DBG_MSG1("ipc_memory_access_fault_handler: unexpected signal ID 0x%x received", id);
        assert(FALSE);
        break;
    }
}
