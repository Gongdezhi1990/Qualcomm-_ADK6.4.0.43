/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    transport_adaptation_common.c

DESCRIPTION
    private/utility functions for transport adaptation
*/
#include <stdlib.h>
#include <source.h>
#include <sink.h>
#include <stream.h>
#include "transport_adaptation_common.h"
#include "transport_adaptation.h"
#include "transport_gatt.h"
#include "print.h"

static transport_adaptation_data_t ta_data;

/*****************************************************************************/
bool transportIsInitialised(void)
{
    PRINT(("transportIsInitialised\n"));

    if((ta_data.appTask == NULL) || (ta_data.transportTask.handler == NULL))
    {
        return FALSE;
    }
    return TRUE;
}

/*****************************************************************************/
void transportInitialise(Task app_task)
{
    PRINT(("transportInitialise\n"));
    /* Store the application task. */
    ta_data.appTask = app_task;

    /* Set the handler function */
    ta_data.transportTask.handler = TransportHandleMessage;

    /* Initailase the GATT connection identifier. */
    ta_data.cid = TA_GATT_INVALID_CID;
}

/*****************************************************************************/
Task transportGetApptask(void)
{
    PRINT(("transportGetApptask\n"));
    return ta_data.appTask;
}

/*****************************************************************************/
Task transportGetLibTask(void)
{
    PRINT(("transportGetLibTask\n"));
    /* return TA library task */
    return (Task) (&ta_data.transportTask);
}

/*****************************************************************************/
uint16 transportGetGattCid(void)
{
    PRINT(("transportGetGattCid\n"));
    /* return Connection identifier */
    return ta_data.cid;
}

/*****************************************************************************/
void transportSetGattCid(uint16 cid)
{
    PRINT(("transportSetGattCid\n"));
    /* Store the GATT connection identifier. */
    ta_data.cid = cid;
}

