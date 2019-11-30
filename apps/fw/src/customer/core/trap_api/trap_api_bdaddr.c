
/* Copyright (c) 2019 Qualcomm Technologies International, Ltd. */
/*    */
#include "trap_api/trap_api_private.h"

#if TRAPSET_BDADDR
bool VmOverrideBdaddr(const bdaddr* bd_addr)
{
    IPC_VM_OVERRIDE_BDADDR send_prim;
    IPC_BOOL_RSP recv_prim;

    if (bd_addr == NULL)
    {
        return FALSE;
    }
    send_prim.bd_addr = (const BD_ADDR_T*) bd_addr;

    ipc_send(IPC_SIGNAL_ID_VM_OVERRIDE_BDADDR, &send_prim, sizeof(send_prim));
    (void)ipc_recv(IPC_SIGNAL_ID_VM_OVERRIDE_BDADDR_RSP, &recv_prim);
    return recv_prim.ret;
}
#endif
