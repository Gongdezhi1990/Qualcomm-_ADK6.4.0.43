/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "ipc/ipc_private.h"

IPC_DATA ipc_data;
PANIC_DATA *panic_data;

static void ipc_interrupt_handler(void)
{
    GEN_BG_INT(ipc);
    ipc_data.pending = TRUE;
}


void ipc_init(void)
{
    const uint8 *from_p0;

    /* We do a raw read from the known handle in order to get the pointer to the
     * BUFFER_MSG in shared memory */
    from_p0 = mmu_read_port_map_8bit(MMU_INDEX_RESERVED_IPC, 0);

    ipc_data.recv = (BUFFER_MSG *)(*((const uint32*)from_p0));
    buf_update_back(ipc_data.recv);

    IPC_RECV_POINTER(ipc_data.send, BUFFER_MSG);
    /* Need to wait until we know about ipc_data.send before we trigger an IPC
     * message by trying to free data from ipc_data.recv */
    buf_update_behind_free(ipc_data.recv);

#ifdef CHIP_DEF_P1_SQIF_SHALLOW_SLEEP_WA_B_195036
    IPC_RECV_VALUE(ipc_data.p1_pm_flash_offset_from_p0);
#endif  /* CHIP_DEF_P1_SQIF_SHALLOW_SLEEP_WA_B_195036 */

    IPC_RECV_POINTER(panic_data, PANIC_DATA);
    assert(panic_data);

    configure_interrupt(INT_SOURCE_INTERPROC_EVENT_1,
                        INT_LEVEL_FG,
                        ipc_interrupt_handler);

    configure_interrupt(INT_SOURCE_INTERPROC_EVENT_2,
                        INT_LEVEL_EXCEPTION,
                        panic_interrupt_handler);
#ifdef FW_IPC_UNIT_TEST
    ipc_test_init();
#endif
}

#ifdef CHIP_DEF_P1_SQIF_SHALLOW_SLEEP_WA_B_195036


uint32 ipc_get_p1_flash_offset(void)
{
    return ipc_data.p1_pm_flash_offset_from_p0;
}

#endif /* CHIP_DEF_P1_SQIF_SHALLOW_SLEEP_WA_B_195036 */

void ipc_recv_buffer_mapping_policy_init(void)
{
    /* Nothing to do here on P1. P0 will send the
     * IPC_LEAVE_RECV_BUFFER_PAGES_MAPPED signal if the key is set. */
}
