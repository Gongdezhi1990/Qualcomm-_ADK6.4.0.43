/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 */

#include "trap_api/trap_api_private.h"

#if TRAPSET_BITSERIAL

Task MessageBitserialTask(Task task)
{
    return trap_api_register_message_task(task, IPC_MSG_TYPE_BITSERIAL);
}

#endif /* TRAPSET_BITSERIAL */
