/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_private.h

DESCRIPTION
    Private AHI data and functions shared throughout the library.

*/

#ifndef AHI_PRIVATE_H_
#define AHI_PRIVATE_H_

#include <message.h>

/*#define ENABLE_REBOOT_AFTER_DISCONNECT*/


/* Macro used for all critical code sections */
#define AHI_ASSERT(x) {if (!(x)) Panic();}

#define  AHI_EVENT_TO_PRINT_VM_LOGS    0x5f00

/******************************************************************************
NAME
    ahiIsInitialised

DESCRIPTION
    Check if the AHI module is successfully initialised

RETURNS
    TRUE if AHI is initialised, FALSE otherwise.
*/
bool ahiIsInitialised(void);

/******************************************************************************
NAME
    ahiAppTask

DESCRIPTION
    Get the Task registered as the VM app task.

RETURNS
    Task if ahi is initialised, 0 otherwise.
*/
Task ahiAppTask(void);

/******************************************************************************
NAME
    ahiGetTransportTask

DESCRIPTION
    Get the Task registered as the AHI transport task.

RETURNS
    Task if ahi is initialised and a transport is connected, 0 otherwise.
*/
Task ahiGetTransportTask(void);

/******************************************************************************
NAME
    ahiSetTransportTask

DESCRIPTION
    Set the Task registered as the AHI transport task.

RETURNS
    n/a
*/
void ahiSetTransportTask(Task transport_task);

/******************************************************************************
NAME
    ahiTask

DESCRIPTION
    Get the internal AHI library Task.

RETURNS
    Task if ahi is initialised, 0 otherwise.
*/
Task ahiTask(void);

/******************************************************************************
NAME
    ahiCanSendTransportData

DESCRIPTION
    Check if there are enough resources to send an outgoing data packet to the
    Host transport.

    This function is used to throttle the maximum number of messages that
    are posted onto the Host transport's Task. Without it, it is possible
    to run out of VM resources (pmalloc slots) before the Host transport
    has had a chance to process the queued messages.

    Throttling messages is only required for operations that send multiple
    messages to the Host without waiting for a reply for each one.
    Currently that is:
        AHI_CONFIG_GET_CFM
        AHI_CONFIG_FILE_GET_CFM

RETURNS
    TRUE if it is ok to send a data packet, FALSE otherwise.
*/
bool ahiCanSendTransportData(void);

/******************************************************************************
NAME
    ahiTransportDataSlotsInc

DESCRIPTION
    Increase the count of available outgoing data packet slots.

    Should only be called by AhiTransportSendDataAck() because it can only
    be increased when the Host transport Task has processed a queued
    data packet.
*/
void ahiTransportDataSlotsInc(void);

/******************************************************************************
NAME
    ahiTransportDataSlotsInc

DESCRIPTION
    Decrease the count of available outgoing data packet slots.

    Must be called just before a data packet is posted to the Host transport
    Task via MessageSend().
*/
void ahiTransportDataSlotsDec(void);

/*!
    @brief Data that stores the current state of a config_set operation.

    A config_set can consist of multiple data packets; AHI uses
    this struct to keep track of the state of the operation.
*/
typedef struct
{
    uint16 id;      /**< Config block id */
    void * data;    /**< Pointer to writeable config block data. */
    uint16 size;    /**< Total size of the config block, in octets. */
    uint16 offset;  /**< Amount of the data block that has been written to. */
    uint16 status;  /**< Status of the last step of the config_set operation. */
} config_set_ctx;

/******************************************************************************
NAME
    ahiGetConfigSetContext

DESCRIPTION
    Get the current config_set context.

RETURNS
    Pointer to the config_set context or 0 if it does not exist.
*/
config_set_ctx * ahiGetConfigSetContext(void);

/******************************************************************************
NAME
    ahiSetConfigSetContext

DESCRIPTION
    Store the current config_set context in the global AHI data.
*/
void ahiSetConfigSetContext(config_set_ctx *ctx);

/******************************************************************************
NAME
    ahiSetSupportedMode

DESCRIPTION
    Set the current app mode for the AHI library.
*/
void ahiSetSupportedMode(ahi_application_mode_t mode);

/******************************************************************************
NAME
    ahiGetRequestedAppMode

DESCRIPTION
    Get the current requested app mode from a AHI_MODE_SET_REQ operation.
    
RETURNS
    If there is AHI_MODE_SET_REQ operation in progress returns
    the requested mode, otherwise returns ahi_app_mode_undefined.
*/
ahi_application_mode_t ahiGetRequestedAppMode(void);

/******************************************************************************
NAME
    ahiSetRequestedAppMode

DESCRIPTION
    Set the current requested app mode from a AHI_MODE_SET_REQ operation.
*/
void ahiSetRequestedAppMode(ahi_application_mode_t mode);

/******************************************************************************
NAME
    ahiExit

DESCRIPTION
    NOTE: This is private because it is only for the unit tests. Most VM
    libraries don't have an "exit" function so AHI follows the pattern.

    Free any resources used by AHI and leave it in an unitialised state.
    Basically the opposite of AhiInit().
*/
void ahiExit(void);

#ifdef ENABLE_REBOOT_AFTER_DISCONNECT
bool ahiIsRebootNeeded(void);
#else
#define ahiIsRebootNeeded() (FALSE)
#endif

#endif /* AHI_PRIVATE_H_ */

