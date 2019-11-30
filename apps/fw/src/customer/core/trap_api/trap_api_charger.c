/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 */

#include "trap_api/trap_api_private.h"


#if TRAPSET_CHARGERMESSAGE

Task MessageChargerTask(Task task)
{
    return trap_api_register_message_task(task, IPC_MSG_TYPE_CHARGER);
}

#endif /* TRAPSET_CHARGERMESSAGE */


