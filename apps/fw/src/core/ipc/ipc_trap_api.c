/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "ipc/ipc_private.h"

#include "trap_api/trap_api.h"



/*****************************************************************************
 *
 *  ipc_trap_fast_api_handler
 *
 *  Handles receiving ipc api calls that are marked as 'fast' in the xml
 */


bool ipc_trap_api_handler(IPC_SIGNAL_ID id, const void *prim, uint16 length_bytes)
{
    switch(id)
    {
        case IPC_SIGNAL_ID_APP_MSG:
        {
            const IPC_APP_MSG_PRIM *ipc_app_msg = (const IPC_APP_MSG_PRIM *)prim;
            trap_api_send_message_filtered(ipc_app_msg->hdlr,
                                           ipc_app_msg->id,
                                           ipc_app_msg->message,
                                           ipc_app_msg->allow_duplicates);
        }
            break;
        case IPC_SIGNAL_ID_APP_SINK_SOURCE_MSG:
        {
            const IPC_APP_SINK_SOURCE_MSG_PRIM *ipc_msg =
                                    (const IPC_APP_SINK_SOURCE_MSG_PRIM *)prim;
            if (!trap_api_send_sink_source_message_filtered(ipc_msg->stream_id,
                                                            ipc_msg->id,
                                                            ipc_msg->message,
                                                            ipc_msg->allow_duplicates))
            {
                return FALSE;
            }
        }
            break;
        case IPC_SIGNAL_ID_APP_MSG_TO_HANDLER:
        {
            const IPC_APP_MSG_TO_HANDLER_PRIM *ipc_msg =
                                     (const IPC_APP_MSG_TO_HANDLER_PRIM *)prim;

            /* The message length is the primitive length minus the structure
               data. This ignores padding for IPC alignment so could be up to 3
               bytes larger than on P0, but never smaller. */
            uint32 len = length_bytes -
                         offsetof(IPC_APP_MSG_TO_HANDLER_PRIM, message);
            void *message = pmalloc(len);
            memcpy(message, ipc_msg->message, len);
            trap_api_send_message_to_task_filtered((Task)ipc_msg->handler,
                                                   ipc_msg->id,
                                                   message,
                                                   ipc_msg->allow_duplicates);
        }
            break;
        default:
            L1_DBG_MSG1("ipc_trap_api_slow: received unexpected prim %d", id);
            assert(FALSE);
            break;
    }
    return TRUE;
}


void ipc_send_trap_api_version_info(void)
{
#ifdef TRAP_VERSION_MODULE_PRESENT
    IPC_TRAP_API_VERSION_INFO ipc_msg;

    memcpy(ipc_msg.version, trap_version, sizeof(trap_version));
    ipc_msg.trapset_bitmap_dwords = trapset_bitmap_length;
    ipc_msg.trapset_bitmap = trapset_bitmap;

    ipc_send(IPC_SIGNAL_ID_TRAP_API_VERSION, &ipc_msg, sizeof(ipc_msg));
#endif /* MODULE_TRAP_VERSION_PRESENT */
}

/**
 * Handle the IPC message giving the Trap API version information
 * and check for compatibility.
 * CS-328404-DD-LATEST.pdf, section 7.2.1 describes the compatibility criteria:
 * That major versions must match and the minor version of P1
 * code must be less than or equal to the minor version of P0 code.
 */
void ipc_trap_api_version_prim_handler(IPC_SIGNAL_ID id, const void *prim)
{
#ifdef TRAP_VERSION_MODULE_PRESENT
    if(id == IPC_SIGNAL_ID_TRAP_API_VERSION)
    {
        const uint32 * p0_version, * p1_version;
        const uint32 * p0_trapset_bitmap, * p1_trapset_bitmap;
        uint32 i;
        const IPC_TRAP_API_VERSION_INFO *ipc_msg =
                                    (const IPC_TRAP_API_VERSION_INFO *)prim;
        L2_DBG_MSG3("IPC Other processor has Trap API v%d.%d.%d",
                ipc_msg->version[0], ipc_msg->version[1], ipc_msg->version[2]);
        p0_version = &ipc_msg->version[0];
        p1_version = &trap_version[0];
        p0_trapset_bitmap = &ipc_msg->trapset_bitmap[0];
        p1_trapset_bitmap = &trapset_bitmap[0];
        if((p0_version[0] != p1_version[0]) || (p0_version[1] < p1_version[1]))
        {
            panic(PANIC_TRAP_API_VERSION_INCOMPATIBILITY);
        }

        for(i=0; i< trapset_bitmap_length; ++i)
        {
            if(p1_trapset_bitmap[i] & ~p0_trapset_bitmap[i])
            {
                panic(PANIC_INCOMPATIBLE_TRAPSETS);
            }
        }
    }
#else
    UNUSED(id);
    UNUSED(prim);
#endif /* TRAP_VERSION_MODULE_PRESENT */
}
