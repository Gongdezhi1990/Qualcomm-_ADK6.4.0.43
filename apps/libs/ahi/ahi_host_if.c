/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_host_if.c

DESCRIPTION
    AHI Host interface - glues the AHI library internals to the
    AHI protocol. Contains:
    
      Helper functions for sending AHI data packets to the Host.

      Helper functions for converting AHI data packets to internal
      AHI messages.

*/

#include <panic.h>
#include <print.h>
#include <stdlib.h>
#include <string.h>

#include "ahi.h"
#include "ahi_host_if.h"
#include "ahi_msg.h"
#include "ahi_private.h"
#include "ahi_protocol.h"
#include "byte_utils.h"


/* Macros used to make messsage creation simpler */
#define MAKE_AHI_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);
#define COPY_AHI_MESSAGE_WITH_LEN(TYPE, LEN, src, dst) memmove((dst), (src), sizeof(TYPE##_T) + LEN);


/******************************************************************************
    Private functions 
*/
static void transportSendData(Task transport_task, AHI_TRANSPORT_SEND_DATA_T *payload);
static AHI_TRANSPORT_SEND_DATA_T *createAhiMessagePanic(uint8 opcode, uint8 flags, uint16 length);

static uint16 ApplicationModeToAppModeFlags(ahi_application_mode_t app_mode);
static ahi_application_mode_t AppModeFlagsToApplicationMode(uint16 flags);

static void handleConnectReq(Task transport_task);
static void handleDisconnectReq(Task transport_task);
static void handleConfigGetReq(const uint8 *data);
static void handleConfigSetReq(const uint8 *data);
static void handleModeSetReq(const uint8 *data);
static void handleEventInjectInd(const uint8 *data);


/******************************************************************************
NAME
    transportSendData

DESCRIPTION
    Tell the AHI transport to send a data packet to the Host by sending
    it a AHI_TRANSPORT_SEND_DATA message.
*/
static void transportSendData(Task transport_task, AHI_TRANSPORT_SEND_DATA_T *msg)
{
    if (!transport_task)
    {
        PRINT(("AHI: transportSendData called with an unconnected transport\n"));
        free(msg);
        return;
    }

    if (!ahiCanSendTransportData())
    {
        MAKE_AHI_MESSAGE_WITH_LEN(AHI_INTERNAL_DELAYED_SEND_DATA, msg->size);

        PRINT(("AHI:   no space on transport; delaying msg. payload 0x%p size %u\n", msg->payload, msg->size));

        message->transport_task = transport_task;
        COPY_AHI_MESSAGE_WITH_LEN(AHI_TRANSPORT_SEND_DATA, msg->size, msg, &message->send_data_msg);

        MessageSend(ahiTask(), AHI_INTERNAL_DELAYED_SEND_DATA, message);
        free(msg);
        return;
    }
    ahiTransportDataSlotsDec();

    MessageSend(transport_task, AHI_TRANSPORT_SEND_DATA, msg);
}

/******************************************************************************
NAME
    createAhiMessagePanic

DESCRIPTION
    Allocate memory and write the header for an AHI msg.

    Memory for the entire message (header + payload) is allocated but
    only the header fields are written.

    This function will panic if the AHI message cannot be created
    (for any reason).

RETURNS
    Pointer to the msg buffer.
*/
static AHI_TRANSPORT_SEND_DATA_T *createAhiMessagePanic(uint8 opcode, uint8 flags, uint16 length)
{
    MAKE_AHI_MESSAGE_WITH_LEN(AHI_TRANSPORT_SEND_DATA, (AHI_MSG_HEADER_SIZE + length));

    AHI_ASSERT(length <= AHI_MSG_PAYLOAD_SIZE_MAX);

    message->size = (AHI_MSG_HEADER_SIZE + length);
    ByteUtilsSet1Byte(message->payload, AHI_MSG_INDEX_OPCODE, opcode);
    ByteUtilsSet1Byte(message->payload, AHI_MSG_INDEX_FLAGS, flags);
    ByteUtilsSet2Bytes(message->payload, AHI_MSG_INDEX_LENGTH, length);

    return message;
}

/******************************************************************************
NAME
    ApplicationModeToAppModeFlags

DESCRIPTION
    Convert a ahi_application_mode_t into the bitfield value used in
    AHI_MODE_GET_CFM and AHI_MODE_SET_CFM.

RETURNS
    AHI msg bitfield value
*/
static uint16 ApplicationModeToAppModeFlags(ahi_application_mode_t app_mode)
{
    switch (app_mode)
    {
    case ahi_app_mode_normal:
        return AHI_MODE_NORMAL;
    case ahi_app_mode_configuration:
        return AHI_MODE_CONFIG;
    case ahi_app_mode_normal_test:
        return AHI_MODE_TEST;
    default:
        return AHI_MODE_UNDEFINED;
    }
}

/******************************************************************************
NAME
    AppModeFlagsToApplicationMode

DESCRIPTION
    Convert a the bitfield value for app mode used in AHI_MODE_GET_CFM
    and AHI_MODE_SET_CFM into a ahi_application_mode_t.

RETURNS
    AHI msg bitfield value
*/
static ahi_application_mode_t AppModeFlagsToApplicationMode(uint16 flags)
{
    if (flags == AHI_MODE_NORMAL)
        return ahi_app_mode_normal;
    else if (flags == AHI_MODE_CONFIG)
        return ahi_app_mode_configuration;
    else if (flags == AHI_MODE_TEST)
        return ahi_app_mode_normal_test;
    else
        return ahi_app_mode_undefined;
}

/******************************************************************************
    Local functions 
*/

uint16 ahiCalculateModeSetStatus(ahi_application_mode_t app_mode, ahi_application_mode_t requested_mode, bool need_reboot)
{
    /* Check the various combinations of whether the request was rejected or not. */
    if (requested_mode != app_mode)
    {
        return AHI_STATUS_REJECTED;
    }
    else
    {
        if (need_reboot)
            return AHI_STATUS_REBOOT_REQUIRED;
        else
            return AHI_STATUS_SUCCESS;
    }
}

uint16 ahiConvertApiStatusToMessageStatus(ahi_status_t status)
{
    switch (status)
    {
        case ahi_status_success: return AHI_STATUS_SUCCESS;
        case ahi_status_not_initialised: return AHI_STATUS_NOT_FOUND;
        case ahi_status_bad_parameter: return AHI_STATUS_BAD_PARAMETER;
        case ahi_status_already_in_use: return AHI_STATUS_ALREADY_IN_USE;
        case ahi_status_no_memory: return AHI_STATUS_NO_MEMORY;
        default: return AHI_STATUS_NOT_FOUND;
    }
}

void ahiSendConnectCfm(Task transport_task, uint16 status)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendConnectCfm status 0x%x\n", status));

    msg = createAhiMessagePanic(AHI_CONNECT_CFM, AHI_FLAG_NONE, AHI_CONNECT_CFM_LENGTH);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, status);

    transportSendData(transport_task, msg);
}

void ahiSendDisconnectCfm(Task transport_task, uint16 status)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendDisonnectCfm status 0x%x\n", status));

    msg = createAhiMessagePanic(AHI_DISCONNECT_CFM, AHI_FLAG_NONE, AHI_DISCONNECT_CFM_LENGTH);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, status);

    transportSendData(transport_task, msg);
}

void ahiSendModeGetCfm(Task transport_task, AHI_MODE_GET_CFM_T *cfm)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendModeGetCfm status 0x%x mode 0x%x\n", cfm->status, cfm->current_mode));

    msg = createAhiMessagePanic(AHI_MODE_GET_CFM, AHI_FLAG_NONE, AHI_MODE_GET_CFM_LENGTH);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->status);
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, ApplicationModeToAppModeFlags(cfm->current_mode));	

    transportSendData(transport_task, msg);
}

void ahiSendModeSetCfm(Task transport_task, AHI_MODE_SET_CFM_T *cfm)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendModeSetCfm status 0x%x app_mode 0x%x\n", cfm->status, ApplicationModeToAppModeFlags(cfm->app_mode)));

    msg = createAhiMessagePanic(AHI_MODE_SET_CFM, AHI_FLAG_NONE, AHI_MODE_SET_CFM_LENGTH);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->status);
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, ApplicationModeToAppModeFlags(cfm->app_mode));

    transportSendData(transport_task, msg);
}

void ahiSendVersionCfm(Task transport_task, AHI_VERSION_CFM_T *cfm)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendVersionCfm major 0x%x, minor 0x%x\n", AHI_VERSION_MAJOR, AHI_VERSION_MINOR));

    msg = createAhiMessagePanic(AHI_VERSION_CFM, AHI_FLAG_NONE, AHI_VERSION_CFM_LENGTH);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->status);
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->major);
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->minor);

    transportSendData(transport_task, msg);
}

void ahiSendConfigGetCfm(Task transport_task, AHI_CONFIG_GET_CFM_T *cfm)
{
    uint16 byteIndex;
    uint16 data_size;
    uint8 flags;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    do 
    {
        if (!ahiCanSendTransportData())
        {
            MESSAGE_MAKE(delayed_msg, AHI_CONFIG_GET_CFM_T);
            PRINT(("   host_if currently full\n"));
            /* Send this back to ourselves to be processed after transport
               task has had chance to consume previous messages. */
            memcpy(delayed_msg, cfm, sizeof(*delayed_msg));
            MessageSend(ahiTask(), AHI_CONFIG_GET_CFM, delayed_msg);
            break;
        }

        /* Limit data size to the maximum allowed in a AHI_CONFIG_GET_CFM. */
        data_size = (cfm->size - cfm->offset);
        flags = AHI_FLAG_NONE;
        if (data_size > AHI_CONFIG_GET_CFM_DATA_SIZE_MAX)
        {
            data_size = AHI_CONFIG_GET_CFM_DATA_SIZE_MAX;
            flags = AHI_FLAG_MORE_DATA;
        }


        PRINT(("AHI: ahiSendConfigGetCfm status 0x%x config_id 0x%x size %u data 0x%p offset %u data_size %u\n", 
                cfm->status, cfm->config_id, cfm->size, cfm->data, cfm->offset, data_size));

        msg = createAhiMessagePanic(AHI_CONFIG_GET_CFM, flags, 
                               (AHI_CONFIG_GET_CFM_LENGTH + data_size));

        byteIndex = AHI_MSG_INDEX_PAYLOAD;
        byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->config_id);
        byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->status);
        byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->size);

        if (cfm->data && (data_size > 0))
        {
            ByteUtilsMemCpy16(msg->payload, byteIndex, (uint16 *)cfm->data, cfm->offset, data_size);
        }

        transportSendData(transport_task, msg);
        cfm->offset += data_size;

        /* Release the config block only after we have sent all the data. */
        if (cfm->status == AHI_STATUS_SUCCESS
            && (flags & AHI_FLAG_MORE_DATA) != AHI_FLAG_MORE_DATA)
        {
            MESSAGE_MAKE(complete, AHI_INTERNAL_CONFIG_GET_REQ_COMPLETE_T);
            complete->config_id = cfm->config_id;
            MessageSend(ahiTask(), AHI_INTERNAL_CONFIG_GET_REQ_COMPLETE, complete);
        }
    }
    while (cfm->offset < cfm->size);
}

void ahiSendConfigSetCfm(Task transport_task, AHI_CONFIG_SET_CFM_T *cfm)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendConfigSetCfm status 0x%x config_id 0x%x\n", 
            cfm->status, cfm->config_id));

    msg = createAhiMessagePanic(AHI_CONFIG_SET_CFM, AHI_FLAG_NONE, 
                           AHI_CONFIG_SET_CFM_LENGTH);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->config_id);
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->status);

    transportSendData(transport_task, msg);
}

void ahiSendConfigFileGetCfm(Task transport_task, AHI_CONFIG_FILE_GET_CFM_T *cfm)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendConfigFileGetCfm status 0x%x size 0x%lx\n", 
            cfm->status, cfm->size));

    msg = createAhiMessagePanic(AHI_CONFIG_FILE_GET_CFM, AHI_FLAG_NONE, 
                           AHI_CONFIG_FILE_GET_CFM_LENGTH);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->status);
    byteIndex += ByteUtilsSet4Bytes(msg->payload, byteIndex, cfm->size);

    transportSendData(transport_task, msg);
}

void ahiSendConfigFileGetDataInd(Task transport_task, AHI_CONFIG_FILE_GET_DATA_IND_T *ind)
{
    uint16 byteIndex;

    while (SourceSize(ind->source))
    {
        uint32 left;
        AHI_TRANSPORT_SEND_DATA_T *msg;
        uint16 payload_size;
        uint8 flags;
        uint8 *payload;
        const uint8 *data;

        left = SourceSize(ind->source);
        PRINT(("AHI:  left %lu\n", left));

        if (!ahiCanSendTransportData())
        {
            MESSAGE_MAKE(delayed_msg, AHI_CONFIG_FILE_GET_DATA_IND_T);
            PRINT(("   host_if currently full\n"));
            /* Send this back to ourselves to be processed after transport
               task has had chance to consume previous messages. */
            delayed_msg->source = ind->source;
            MessageSend(ahiTask(), AHI_CONFIG_FILE_GET_DATA_IND, delayed_msg);
            break;
        }

        payload_size = (left <= AHI_MSG_PAYLOAD_SIZE_MAX) ? left : AHI_MSG_PAYLOAD_SIZE_MAX;
        flags = ((left > AHI_MSG_PAYLOAD_SIZE_MAX) ? AHI_FLAG_MORE_DATA : AHI_FLAG_NONE);

        PRINT(("AHI: ahiSendConfigFileGetDataInd left %lu flags 0x%x payload_size %u\n", 
                left, flags, payload_size));

        data = SourceMap(ind->source);
        if (!data)
        {
            /* ind->source should always be valid because SourceSize returned > 0 above,
               so if SourceMap has failed something strange has happened. */
            PRINT(("AHI: Failed to map file source 0x%p\n", (void *)ind->source));
            Panic();
        }

        msg = createAhiMessagePanic(AHI_CONFIG_FILE_GET_DATA_IND, flags, payload_size);

        byteIndex = AHI_MSG_INDEX_PAYLOAD;
        payload = (uint8 *)((uint16 *)msg->payload + (byteIndex >> 1));
        ByteUtilsMemCpyFromStream(payload, data, payload_size);
        SourceDrop(ind->source, payload_size);

        /* Only close the source if all the data has been sent */
        if (SourceSize(ind->source) == 0)
        {
            PRINT(("AHI:   Closing source\n"));
            SourceClose(ind->source);
        }

        transportSendData(transport_task, msg);
    }
}

void ahiSendEventReportInd(AHI_EVENT_REPORT_IND_T *ind)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendEventReportInd event 0x%x\n", ind->id));

    if (!ind->payload)
    {
        ind->payload_size = 0;
    }
    else if (ind->payload_size > AHI_EVENT_REPORT_IND_DATA_SIZE_MAX)
    {
        ind->payload_size = AHI_EVENT_REPORT_IND_DATA_SIZE_MAX;
    }

    msg = createAhiMessagePanic(AHI_EVENT_REPORT_IND, AHI_FLAG_NONE, 
                            (AHI_EVENT_REPORT_IND_LENGTH + ind->payload_size));

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, ind->id);
    if (ind->payload && (ind->payload_size > 0))
    {
        ByteUtilsMemCpy(msg->payload, byteIndex, ind->payload, 0, ind->payload_size);
    }

    transportSendData(ahiGetTransportTask(), msg);
}

void ahiSendStateMachineStateReportInd(AHI_STATE_MACHINE_STATE_REPORT_IND_T *ind)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendStateMachineStateSendInd machine 0x%x state 0x%x\n", ind->machine_id, ind->state));

    msg = createAhiMessagePanic(AHI_STATE_MACHINE_STATE_REPORT_IND, AHI_FLAG_NONE,
                            AHI_STATE_MACHINE_STATE_REPORT_IND_LENGTH);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, ind->machine_id);
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, ind->state);

    transportSendData(ahiGetTransportTask(), msg);
}

void ahiSendMtuReportInd(AHI_REPORT_MTU_IND_T *ind)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendMtuReportInd mtu 0x%x\n", ind->mtu));

    msg = createAhiMessagePanic(AHI_REPORT_MTU_IND, AHI_FLAG_NONE,
                            AHI_REPORT_MTU_IND_LENGTH);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, ind->mtu);

    transportSendData(ahiGetTransportTask(), msg);
}

void ahiSendConnIntReportInd(AHI_REPORT_CONN_INT_IND_T *ind)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendConnIntReportInd  ind->ci 0x%x \n", ind->ci));

    msg = createAhiMessagePanic(AHI_REPORT_CONN_INT_IND, AHI_FLAG_NONE,
                            AHI_REPORT_CONN_INT_IND_LENGTH);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, ind->ci);

    transportSendData(ahiGetTransportTask(), msg);
}

void ahiSendConfigFileSignatureCfm(Task transport_task, AHI_CONFIG_FILE_SIGNATURE_CFM_T *cfm)
{
    uint16 byteIndex;
    uint16 signature_size = 0;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendConfigFileSignatureCfm status 0x%x signature 0x%p\n", cfm->status, (void *)cfm->signature));

    if (cfm->signature)
    {
        signature_size = AHI_CONFIG_FILE_SIGNATURE_CFM_SIGNATURE_SIZE;
    }

    msg = createAhiMessagePanic(AHI_CONFIG_FILE_SIGNATURE_CFM, AHI_FLAG_NONE, 
                            AHI_CONFIG_FILE_SIGNATURE_CFM_LENGTH + signature_size);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->status);
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->type);
    if (cfm->signature)
    {
        ByteUtilsMemCpy16(msg->payload, byteIndex, (uint16 *)cfm->signature, 0, signature_size);
    }

    transportSendData(transport_task, msg);
}

void ahiSendDisableTransportCfm(Task transport_task, AHI_DISABLE_TRANSPORT_CFM_T *cfm)
{
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendDisableTransportCfm status 0x%x\n", cfm->status));

    msg = createAhiMessagePanic(AHI_DISABLE_TRANSPORT_CFM, AHI_FLAG_NONE, 
                                AHI_DISABLE_TRANSPORT_CFM_LENGTH);

    ByteUtilsSet2Bytes(msg->payload, AHI_MSG_INDEX_PAYLOAD, cfm->status);

    transportSendData(transport_task, msg);
}

void ahiSendProductIdGetCfm(Task transport_task, AHI_PRODUCT_ID_GET_CFM_T *cfm)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendProductIdGetCfm status 0x%x id 0x%lx\n", cfm->status, cfm->product_id));

    msg = createAhiMessagePanic(AHI_PRODUCT_ID_GET_CFM, AHI_FLAG_NONE, AHI_PRODUCT_ID_GET_CFM_LENGTH);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->status);
    byteIndex += ByteUtilsSet4Bytes(msg->payload, byteIndex, cfm->product_id);	

    transportSendData(transport_task, msg);
}


void ahiSendAppBuildIdGetCfm(Task transport_task, AHI_APP_BUILD_ID_GET_CFM_T *cfm)
{
    uint16 byteIndex;
    AHI_TRANSPORT_SEND_DATA_T *msg;

    PRINT(("AHI: ahiSendAppBuildIdGetCfm status 0x%x id %u\n", cfm->status, cfm->app_build_id));

    msg = createAhiMessagePanic(AHI_APP_BUILD_ID_GET_CFM, AHI_FLAG_NONE, AHI_APP_BUILD_ID_GET_CFM_LENGTH);

    byteIndex = AHI_MSG_INDEX_PAYLOAD;
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->status);
    byteIndex += ByteUtilsSet2Bytes(msg->payload, byteIndex, cfm->app_build_id);	

    transportSendData(transport_task, msg);
}


void ahiSendDelayedSendData(AHI_INTERNAL_DELAYED_SEND_DATA_T *msg)
{
    MAKE_AHI_MESSAGE_WITH_LEN(AHI_TRANSPORT_SEND_DATA, msg->send_data_msg.size);

    COPY_AHI_MESSAGE_WITH_LEN(AHI_TRANSPORT_SEND_DATA, msg->send_data_msg.size, &msg->send_data_msg, message);

    transportSendData(msg->transport_task, message);
}

/*
    Handler functions for messages from Host with a payload.
*/

static void handleConnectReq(Task transport_task)
{
    MESSAGE_MAKE(msg, AHI_CONNECT_REQ_T);
    msg->transport_task = transport_task;
    MessageSend(ahiTask(), AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_CONNECT_REQ), msg);
}

static void handleDisconnectReq(Task transport_task)
{
    MESSAGE_MAKE(msg, AHI_DISCONNECT_REQ_T);
    msg->transport_task = transport_task;
    MessageSend(ahiTask(), AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_DISCONNECT_REQ), msg);
}

static void handleConfigGetReq(const uint8 *data)
{
    MESSAGE_MAKE(msg, AHI_CONFIG_GET_REQ_T);
    msg->config_id = ByteUtilsGet2BytesFromStream(&data[AHI_CONFIG_GET_REQ_CONFIG_ID]);
    MessageSend(ahiTask(), AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_CONFIG_GET_REQ), msg);
}

static void handleConfigSetReq(const uint8 *data)
{
    uint16 data_size = (ByteUtilsGet2BytesFromStream(&data[AHI_MSG_INDEX_LENGTH]) - AHI_CONFIG_SET_REQ_LENGTH);
    MAKE_AHI_MESSAGE_WITH_LEN(AHI_CONFIG_SET_REQ, data_size);

    message->flags = data[AHI_MSG_INDEX_FLAGS];
    message->config_id = ByteUtilsGet2BytesFromStream(&data[AHI_CONFIG_SET_REQ_CONFIG_ID]);
    message->size = ByteUtilsGet2BytesFromStream(&data[AHI_CONFIG_SET_REQ_SIZE]);
    message->data_size = data_size;
    ByteUtilsMemCpyFromStream(message->data, &data[AHI_CONFIG_SET_REQ_DATA], data_size);

    MessageSend(ahiTask(), AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_CONFIG_SET_REQ), message);
}

static void handleModeSetReq(const uint8 *data)
{
    uint16 flags;
    MESSAGE_MAKE(msg, AHI_MODE_SET_REQ_T);
    flags = ByteUtilsGet2BytesFromStream(&data[AHI_MODE_SET_REQ_APP_MODE]);
    msg->app_mode = AppModeFlagsToApplicationMode(flags);
    PRINT(("AHI: flags 0x%x mode 0x%x\n", flags, msg->app_mode));
    MessageSend(ahiTask(), AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_MODE_SET_REQ), msg);
}

static void handleEventInjectInd(const uint8 *data)
{
    MESSAGE_MAKE(msg, AHI_EVENT_INJECT_IND_T);
    msg->event = ByteUtilsGet2BytesFromStream(&data[AHI_EVENT_INJECT_IND_EVENT]);
    MessageSend(ahiTask(), AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_EVENT_INJECT_IND), msg);
}

/*
    Handler functions for rejecting requests from unconnected hosts.
*/

static void rejectVersionReq(Task transport_task)
{
    AHI_VERSION_CFM_T cfm;

    memset(&cfm, 0, sizeof(cfm));
    cfm.status = AHI_STATUS_NOT_CONNECTED;
    ahiSendVersionCfm(transport_task, &cfm);
}

static void rejectConfigGetReq(Task transport_task, const uint8 *data)
{
    AHI_CONFIG_GET_CFM_T cfm;

    memset(&cfm, 0, sizeof(cfm));
    cfm.config_id = ByteUtilsGet2BytesFromStream(&data[AHI_CONFIG_GET_REQ_CONFIG_ID]);
    cfm.status = AHI_STATUS_NOT_CONNECTED;
    ahiSendConfigGetCfm(transport_task, &cfm);
}

static void rejectConfigSetReq(Task transport_task, const uint8 *data)
{
    AHI_CONFIG_SET_CFM_T cfm;

    memset(&cfm, 0, sizeof(cfm));
    cfm.config_id = ByteUtilsGet2BytesFromStream(&data[AHI_CONFIG_SET_REQ_CONFIG_ID]);
    cfm.status = AHI_STATUS_NOT_CONNECTED;
    ahiSendConfigSetCfm(transport_task, &cfm);
}

static void rejectConfigFileSignatureReq(Task transport_task)
{
    AHI_CONFIG_FILE_SIGNATURE_CFM_T cfm;

    memset(&cfm, 0, sizeof(cfm));
    cfm.status = AHI_STATUS_NOT_CONNECTED;
    ahiSendConfigFileSignatureCfm(transport_task, &cfm);
}

static void rejectConfigFileGetReq(Task transport_task)
{
    AHI_CONFIG_FILE_GET_CFM_T cfm;

    memset(&cfm, 0, sizeof(cfm));
    cfm.status = AHI_STATUS_NOT_CONNECTED;
    ahiSendConfigFileGetCfm(transport_task, &cfm);
}

static void rejectModeGetReq(Task transport_task)
{
    AHI_MODE_GET_CFM_T cfm;

    memset(&cfm, 0, sizeof(cfm));
    cfm.status = AHI_STATUS_NOT_CONNECTED;
    ahiSendModeGetCfm(transport_task, &cfm);
}

static void rejectModeSetReq(Task transport_task)
{
    AHI_MODE_SET_CFM_T cfm;
    memset(&cfm, 0, sizeof(cfm));
    cfm.status = AHI_STATUS_NOT_CONNECTED;
    ahiSendModeSetCfm(transport_task, &cfm);
}

static void rejectDisableTransportReq(Task transport_task)
{
    AHI_DISABLE_TRANSPORT_CFM_T cfm;

    memset(&cfm, 0, sizeof(cfm));
    cfm.status = AHI_STATUS_NOT_CONNECTED;
    ahiSendDisableTransportCfm(transport_task, &cfm);
}


/******************************************************************************
NAME
    ahiHostIfProcessData

DESCRIPTION
    Handle AHI data packets from the Host.

    Convert the raw data packets into machine native format and send them
    on to the internal ahi task (in ahi_task.c) for processing.
*/
void ahiHostIfProcessData(Task transport_task, const uint8 *data)
{
    uint8 opcode = data[AHI_MSG_INDEX_OPCODE];

    PRINT(("AHI: ahiHostIfProcessData opcode 0x%x", opcode));
    PRINT((" flags 0x%x length %u\n", data[AHI_MSG_INDEX_FLAGS],
           ByteUtilsGet2BytesFromStream(&data[AHI_MSG_INDEX_LENGTH])));

    if (transport_task != ahiGetTransportTask())
    {
        switch (opcode)
        {
        /* Connect/disconnect messages that need the host transport data. */
        case AHI_CONNECT_REQ:
            handleConnectReq(transport_task);
            break;
        case AHI_DISCONNECT_REQ:
            handleDisconnectReq(transport_task);
            break;

        /* Reject any other opcodes */
        case AHI_VERSION_REQ:
            rejectVersionReq(transport_task);
            break;
        case AHI_CONFIG_GET_REQ:
            rejectConfigGetReq(transport_task, data);
            break;
        case AHI_CONFIG_SET_REQ:
            rejectConfigSetReq(transport_task, data);
            break;
        case AHI_CONFIG_FILE_SIGNATURE_REQ:
            rejectConfigFileSignatureReq(transport_task);
            break;
        case AHI_CONFIG_FILE_GET_REQ:
            rejectConfigFileGetReq(transport_task);
            break;
        case AHI_MODE_GET_REQ:
            rejectModeGetReq(transport_task);
            break;
        case AHI_MODE_SET_REQ:
            rejectModeSetReq(transport_task);
            break;
        case AHI_DISABLE_TRANSPORT_REQ:
            rejectDisableTransportReq(transport_task);
            break;
        
        /* Ignore by default or for certain opcodes. */
        case AHI_EVENT_INJECT_IND:
        default:
            PRINT(("AHI: Ignored opcode 0x%x\n", opcode));
            break;
        }
    }
    else
    {
        switch (opcode)
        {
        /* Connect/disconnect messages that need the host transport data. */
        case AHI_CONNECT_REQ:
            handleConnectReq(transport_task);
            break;
        case AHI_DISCONNECT_REQ:
            handleDisconnectReq(transport_task);
            break;

        /* Simple messages where only the opcode is needed. */
        case AHI_VERSION_REQ:
        case AHI_CONFIG_FILE_GET_REQ:
        case AHI_MODE_GET_REQ:
        case AHI_CONFIG_FILE_SIGNATURE_REQ:
        case AHI_DISABLE_TRANSPORT_REQ:
        case AHI_PRODUCT_ID_GET_REQ:
        case AHI_APP_BUILD_ID_GET_REQ:
            MessageSend(ahiTask(), AHI_INTERNAL_MSG_ID_FROM_OPCODE(opcode), 0);
            break;

        /* Messages that have a payload that needs to be read. */
        case AHI_CONFIG_GET_REQ:
            handleConfigGetReq(data);
            break;
        case AHI_CONFIG_SET_REQ:
            handleConfigSetReq(data);
            break;
        case AHI_MODE_SET_REQ:
            handleModeSetReq(data);
            break;
        case AHI_EVENT_INJECT_IND:
            handleEventInjectInd(data);
            break;

        default:
            PRINT(("AHI: Unrecognised opcode 0x%x\n", opcode));
        }
    }
}
