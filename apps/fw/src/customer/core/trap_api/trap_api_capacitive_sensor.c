/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 */

#include "trap_api/trap_api_private.h"

#ifndef DESKTOP_TEST_BUILD

#if TRAPSET_CAPACITIVE_SENSOR

Task MessageCapacitiveSensorTask(Task task)
{
    return trap_api_register_message_task(task, IPC_MSG_TYPE_CAPACITIVE_SENSOR);
}

#endif /* TRAPSET_CAPACITIVE_SENSOR */

#endif /* DESKTOP_TEST_BUILD */
