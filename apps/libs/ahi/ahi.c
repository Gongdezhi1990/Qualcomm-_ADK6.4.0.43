/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi.c

DESCRIPTION
    Implementation of the public Application Host Interface API.

*/
/*!
@file   ahi.c
@brief  Implementation of the public Application Host Interface API.
*/

#include <panic.h>
#include <print.h>
#include <stdlib.h>
#include <string.h>

#include "ahi.h"
#include "ahi_host_if.h"
#include "ahi_private.h"
#include "ahi_task.h"


/* Global AHI data */
typedef struct
{
    Task app_task; /**< VM app task for e.g. sending user events to */
    ahi_application_mode_t app_mode; /**< Current VM app mode */
    ahi_application_mode_t requested_mode; /**< App mode requested by the Host. */
    uint16 app_build_id; /**< Build ID of the VM application. */
    Task transport_task; /**< AHI transport task for sending data to Host */
    TaskData ahi_task; /**< AHI task for processing data from Host */
    uint16 tx_host_slots; /**< Number of available slots to send message to the host. */
    config_set_ctx *ctx; /**< Data to be remembered when processing
                                a config set over multiple messages. */
#ifdef ENABLE_REBOOT_AFTER_DISCONNECT
    bool reboot_needed;
#endif
} AhiData;

static AhiData *ahi;


/******************************************************************************
    Private functions 
*/
static bool isValidAppMode(ahi_application_mode_t app_mode);


/******************************************************************************
NAME
    isValidAppMode

DESCRIPTION
    Check if the given app mode is valid.

RETURNS
    TRUE if app_mode is valid, FALSE otherwise.
*/
static bool isValidAppMode(ahi_application_mode_t app_mode)
{
    if (app_mode >= ahi_app_mode_max)
        return FALSE;

    return TRUE;
}

/******************************************************************************
    Local functions 
*/

/******************************************************************************
NAME
    ahiIsInitialised

DESCRIPTION
    Check if the AHI module is successfully initialised

RETURNS
    TRUE if AHI is initialised, FALSE otherwise.
*/
bool ahiIsInitialised(void)
{
    return (ahi ? TRUE : FALSE);
}

/******************************************************************************
NAME
    ahiAppTask

DESCRIPTION
    Get the Task registered as the VM app task.

RETURNS
    Task if ahi is initialised, 0 otherwise.
*/
Task ahiAppTask(void)
{
    return (ahi ? ahi->app_task : 0);
}

/******************************************************************************
NAME
    ahiGetTransportTask

DESCRIPTION
    Get the Task registered as the AHI transport task.

RETURNS
    Task if ahi is initialised, 0 otherwise.
*/
Task ahiGetTransportTask(void)
{
    return (ahi ? ahi->transport_task : 0);
}

/******************************************************************************
NAME
    ahiSetTransportTask

DESCRIPTION
    Set the Task registered as the AHI transport task.

RETURNS
    n/a
*/
void ahiSetTransportTask(Task transport_task)
{
    if (ahi)
        ahi->transport_task = transport_task;
}

/******************************************************************************
NAME
    ahiTask

DESCRIPTION
    Get the internal AHI library Task.

RETURNS
    Task if ahi is initialised, 0 otherwise.
*/
Task ahiTask(void)
{
    return (ahi ? &ahi->ahi_task : 0);
}


/******************************************************************************
NAME
    ahiCanSendTransportData

DESCRIPTION
    Check if there are enough resources to send an outgoing data packet to the
    Host transport.

RETURNS
    TRUE if it is ok to send a data packet, FALSE otherwise.
*/
bool ahiCanSendTransportData(void)
{
    if (!ahiIsInitialised())
        return FALSE;

    return (ahi->tx_host_slots ? TRUE : FALSE);
}

/******************************************************************************
NAME
    ahiTransportDataSlotsInc

DESCRIPTION
    Increase the count of available outgoing data packet slots.
*/
void ahiTransportDataSlotsInc(void)
{
    if (!ahiIsInitialised())
        return;

    /* Don't throttle vm->host messages in test mode. */
    if (ahi->app_mode == ahi_app_mode_normal_test)
        return;

    ahi->tx_host_slots++;

    PRINT(("AHI: ahiTransportDataSlotsInc slots %u\n", ahi->tx_host_slots));
}

/******************************************************************************
NAME
    ahiTransportDataSlotsInc

DESCRIPTION
    Decrease the count of available outgoing data packet slots.
*/
void ahiTransportDataSlotsDec(void)
{
    if (!ahiIsInitialised())
        return;

    /* Don't throttle vm->host messages in test mode. */
    if (ahi->app_mode == ahi_app_mode_normal_test)
        return;

    ahi->tx_host_slots--;

    PRINT(("AHI: ahiTransportDataSlotsDec slots %u\n", ahi->tx_host_slots));
}

/******************************************************************************
NAME
    ahiGetConfigSetContext

DESCRIPTION
    Get the current config_set context.

RETURNS
    Pointer to the config_set context or 0 if it does not exist.
*/
config_set_ctx * ahiGetConfigSetContext(void)
{
    if (!ahiIsInitialised())
        return 0;

    return ahi->ctx;
}

/******************************************************************************
NAME
    ahiSetConfigSetContext

DESCRIPTION
    Store the current config_set context in the global AHI data.
*/
void ahiSetConfigSetContext(config_set_ctx *ctx)
{
    if (!ahiIsInitialised())
        return;

    ahi->ctx = ctx;
}

void ahiSetSupportedMode(ahi_application_mode_t mode)
{
    if (!ahiIsInitialised())
        return;

    PRINT(("AHI: ahiSetSupportedMode 0x%x\n", mode));
    ahi->app_mode = mode;
}

ahi_application_mode_t ahiGetRequestedAppMode(void)
{
    if (!ahiIsInitialised())
        return ahi_app_mode_undefined;

    PRINT(("AHI: ahiGetRequestedAppMode 0x%x\n", ahi->requested_mode));
    return ahi->requested_mode;
}

void ahiSetRequestedAppMode(ahi_application_mode_t mode)
{
    if (!ahiIsInitialised())
        return;

    PRINT(("AHI: ahiSetRequestedAppMode 0x%x\n", mode));
    ahi->requested_mode = mode;
}

void ahiExit(void)
{
    if (!ahiIsInitialised())
        return;

    free(ahi);
    ahi = NULL;
}

#ifdef ENABLE_REBOOT_AFTER_DISCONNECT
bool ahiIsRebootNeeded(void)
{
    if (!ahiIsInitialised())
        return FALSE;

    return ahi->reboot_needed;
}
#endif


/******************************************************************************
    Public functions 
*/

ahi_status_t AhiInit(Task app_task, ahi_application_mode_t app_mode)
{
    if (ahiIsInitialised())
        return ahi_status_already_in_use;

    if (!isValidAppMode(app_mode))
        return ahi_status_bad_parameter;

    ahi = PanicUnlessMalloc(sizeof(*ahi));
    memset(ahi, 0, sizeof(*ahi));

    ahi->app_task = app_task;
    ahi->app_mode = app_mode;
    ahi->requested_mode = ahi_app_mode_undefined;
    ahi->app_build_id = 0;
    ahi->ahi_task.handler = ahiHandleMessage;
    ahi->tx_host_slots = 1;

    PRINT(("AHI: initalised OK\n"));

    return ahi_status_success;
}

void AhiAppModeChangeCfm(ahi_application_mode_t app_mode, bool need_reboot)
{
    AHI_MODE_SET_CFM_T cfm;

    if (!ahiIsInitialised())
        return;

    ahiSetSupportedMode(app_mode);

    cfm.app_mode = app_mode;
    cfm.status = ahiCalculateModeSetStatus(app_mode, ahiGetRequestedAppMode(), need_reboot);
    ahiSendModeSetCfm(ahiGetTransportTask(), &cfm);

    ahiSetRequestedAppMode(ahi_app_mode_undefined);

    if (need_reboot)
    {
#ifdef ENABLE_REBOOT_AFTER_DISCONNECT
        ahi->reboot_needed = TRUE;
#else
        /* If a reboot is required, send a delayed msg to ahi_task to perform
           the reboot. */
        MessageSendLater(ahiTask(), AHI_INTERNAL_APP_MODE_SET_REBOOT, 0, 100);
#endif
    }    
}

void AhiAppTransportDisableCfm(ahi_status_t status)
{
    AHI_DISABLE_TRANSPORT_CFM_T cfm;

    if (!ahiIsInitialised())
        return;

    cfm.status = ahiConvertApiStatusToMessageStatus(status);
    ahiSendDisableTransportCfm(ahiGetTransportTask(), &cfm);

    MessageSendLater(ahiTask(), AHI_INTERNAL_APP_MODE_SET_REBOOT, 0, 500);
}

void AhiTransportProcessData(Task transport_task, const uint8 *data)
{
    PRINT(("AHI: AhiTransportProcessData\n"));
    if (!ahiIsInitialised())
    {
        return;
    }

    ahiHostIfProcessData(transport_task, data);
}

void AhiTransportSendDataAck(void)
{
    ahiTransportDataSlotsInc();
}

ahi_application_mode_t AhiGetSupportedMode(void)
{
    if (!ahiIsInitialised())
        return ahi_app_mode_undefined;

    return ahi->app_mode;
}

void AhiTestReportEvent(uint16 event_id, const uint8 *payload, uint16 payload_size)
{
    AHI_EVENT_REPORT_IND_T ind;

    if (!ahiIsInitialised() || !ahiGetTransportTask()
        || (ahi->app_mode != ahi_app_mode_normal_test))
        return;

    ind.id = event_id;
    ind.payload = payload;
    /* Convert size from machine words to octets (8bit words) */
    ind.payload_size = ((payload_size << 1) / sizeof(uint16));
    ahiSendEventReportInd(&ind);
}

void AhiTestReportStateMachineState(uint16 machine_id, uint16 state)
{
    AHI_STATE_MACHINE_STATE_REPORT_IND_T ind;

    if (!ahiIsInitialised() || !ahiGetTransportTask()
        || (ahi->app_mode != ahi_app_mode_normal_test))
        return;

    ind.machine_id = machine_id;
    ind.state = state;
    ahiSendStateMachineStateReportInd(&ind);
}

void AhiTestReportMtu(uint16 mtu)
{
    AHI_REPORT_MTU_IND_T ind;

    if (!ahiIsInitialised() || !ahiGetTransportTask()
        || (ahi->app_mode != ahi_app_mode_normal_test))
        return;

    ind.mtu = mtu;
    ahiSendMtuReportInd(&ind);
}

void AhiTestReportConnectionInterval(uint16 conn_interval)
{
    AHI_REPORT_CONN_INT_IND_T ind;

    if (!ahiIsInitialised() || !ahiGetTransportTask()
        || (ahi->app_mode != ahi_app_mode_normal_test))
        return;

    ind.ci = conn_interval;
    ahiSendConnIntReportInd(&ind);
}

void AhiPrintVmLogs(const char *vm_print_buffer)
{
    AhiTestReportEvent(AHI_EVENT_TO_PRINT_VM_LOGS, \
                       (const uint8 *)vm_print_buffer, strlen(vm_print_buffer)+1);
}

ahi_status_t AhiSetAppBuildId(uint16 build_id)
{
    if (!ahiIsInitialised())
    {
        return ahi_status_not_initialised;
    }

    ahi->app_build_id = build_id;
    return ahi_status_success;
}

ahi_status_t AhiGetAppBuildId(uint16 *build_id)
{
    if (!ahiIsInitialised())
    {
        return ahi_status_not_initialised;
    }
    
    if (build_id)
    {
        *build_id = ahi->app_build_id;
    }
    
    return ahi_status_success;
}

