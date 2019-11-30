/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_task.c

DESCRIPTION
    The internal AHI library Task.

    Mainly processes commands from the Host.

*/

#include <boot.h>
#include <file.h>
#include <panic.h>
#include <print.h>
#include <string.h>
#include <stdlib.h>
#include <stream.h>
#include <byte_utils.h>
#include <config_store.h>
#include <vmal.h>

#include "ahi.h"
#include "ahi_host_if.h"
#include "ahi_msg.h"
#include "ahi_private.h"
#include "ahi_protocol.h"
#include "ahi_task.h"


/******************************************************************************
    Private functions 
*/
static ahi_message_status_t convertConfigStatusToAhiStatus(config_store_status_t status);
static config_set_ctx *ahiConfigSetContextCreate(uint16 config_id, uint16 size);
static void ahiConfigSetContextDestroy(config_set_ctx *ctx);
static ahi_message_status_t ahiGetConfigFileSource(Source *source, uint32 *size);

static void handleConnectReq(AHI_CONNECT_REQ_T *req);
static void handleDisconnectReq(AHI_DISCONNECT_REQ_T *req);

static void handleConfigGetReq(AHI_CONFIG_GET_REQ_T *req);
static void handleConfigGetReqComplete(AHI_INTERNAL_CONFIG_GET_REQ_COMPLETE_T *msg);
static void handleConfigSetReq(AHI_CONFIG_SET_REQ_T *req);
static void handleConfigFileGetReq(void);

static void handleModeGetReq(void);
static void handleModeSetReq(AHI_MODE_SET_REQ_T *message);

static void handleEventInjectInd(AHI_EVENT_INJECT_IND_T *msg);

static void handleAppModeSetReboot(void);

static void handleDelayedSendData(AHI_INTERNAL_DELAYED_SEND_DATA_T * msg);

static void handleVersionReq(void);


/******************************************************************************
NAME
    convertConfigStatusToAhiStatus

DESCRIPTION
    Convert a config_store_status_t to an equivalent ahi_message_status_t.
*/
static ahi_message_status_t convertConfigStatusToAhiStatus(config_store_status_t status)
{
    switch (status)
    {
    case config_store_success: return AHI_STATUS_SUCCESS;
    case config_store_error_dynamic_config_init: return AHI_STATUS_REJECTED;
    case config_store_error_config_block_in_use: return AHI_STATUS_ALREADY_IN_USE;
    case config_store_error_memory: return AHI_STATUS_NO_MEMORY;
    case config_store_error_retrieving_dynamic_data: return AHI_STATUS_NOT_FOUND;
    case config_store_error_config_block_not_found: return AHI_STATUS_BAD_PARAMETER;
    default:
        return AHI_STATUS_REJECTED;
    }
}

/******************************************************************************
NAME
    ahiConfigSetContextCreate

DESCRIPTION
    Create and initialise a new config_set_ctx.

RETURNS
    A pointer to a valid config_set context.
*/
static config_set_ctx *ahiConfigSetContextCreate(uint16 config_id, uint16 size)
{
    config_store_status_t status;
    config_set_ctx *ctx;
    uint16 word_size;

    ctx = PanicUnlessMalloc(sizeof(*ctx));
    ctx->id = config_id;
    ctx->data = 0;
    ctx->size = size;
    ctx->offset = 0;
    ctx->status = AHI_STATUS_SUCCESS;

    /* Open config block. */
    if (ctx->size)
    {
        /* req->size is in octets but the config_store api
           expects a size in hextets (16bit words). */
        word_size = (ctx->size >> 1) + (ctx->size % 2);
        status = ConfigStoreGetWriteableConfig(ctx->id, &word_size, &ctx->data);
        ctx->status = convertConfigStatusToAhiStatus(status);
    }
    else
    {
        status = ConfigStoreRemoveConfig(ctx->id);
        ctx->status = convertConfigStatusToAhiStatus(status);
    }

    return ctx;
}

/******************************************************************************
NAME
    ahiConfigSetContextDestroy

DESCRIPTION
    Destroy the given config_set_ctx.
*/
static void ahiConfigSetContextDestroy(config_set_ctx *ctx)
{
    if (!ctx)
        return;

    if (ctx->data)
        ConfigStoreReleaseConfig(ctx->id);

    free(ctx);
}

/******************************************************************************
NAME
    ahiGetConfigFileSource

DESCRIPTION
    Read the config_store metadata to get the filename and size of the
    config definition file. Attempt to open the file as a Source.
*/
ahi_message_status_t ahiGetConfigFileSource(Source *source, uint32 *size)
{
    config_store_status_t status;
    const config_store_meta_data_t *metadata;
    char *filename;
    Source source_;
    uint16 filename_length;

    if (!source || !size)
    {
        return AHI_STATUS_BAD_PARAMETER;
    }

    /* Get the config metadata. */
    status = ConfigStoreGetConfigMetadata(&metadata);
    if (config_store_success != status)
    {
        *source = 0;
        *size = 0;
        return AHI_STATUS_NO_MEMORY;
    }

    /* Attempt to open the file. */
    ByteUtilsGet2Bytes((uint8 *)&metadata->filename_length, 0, &filename_length);
    filename = PanicUnlessMalloc(filename_length + 1);
    ByteUtilsMemCpyUnpackString((uint8 *)filename, metadata->filename, filename_length);
    filename[filename_length] = '\0';
    PRINT(("AHI: ahiGetConfigFileSource filename %s", filename));
    source_ = StreamFileSource(FileFind(FILE_ROOT, filename, strlen(filename)));
    PRINT((" source 0x%p\n", (void *)source));
    free(filename);

    if (source_)
    {
        *source = source_;
        ByteUtilsGet4Bytes((uint8 *)&metadata->file_size, 0, size);
        return AHI_STATUS_SUCCESS;
    }

    *source = 0;
    *size = 0;
    return AHI_STATUS_NOT_FOUND;
}

/******************************************************************************
NAME
    handleConnectReq

DESCRIPTION
    Host is requesting to create a new session with AHI.
    
    If another host is already connected return an error.
*/
static void handleConnectReq(AHI_CONNECT_REQ_T *req)
{
    uint16 status = AHI_STATUS_SUCCESS;

    if (!ahiIsInitialised())
        status = AHI_STATUS_REJECTED;

    if (ahiGetTransportTask()
        && (ahiGetTransportTask() != req->transport_task))
        status = AHI_STATUS_ALREADY_IN_USE;

    if (AHI_STATUS_SUCCESS == status)
        ahiSetTransportTask(req->transport_task);

    ahiSendConnectCfm(req->transport_task, status);
}

/******************************************************************************
NAME
    handleDisconnectReq

DESCRIPTION
    Host is requesting to end an existing session with AHI.
    
    Return an error if no host is connected.
*/
static void handleDisconnectReq(AHI_DISCONNECT_REQ_T *req)
{
    uint16 status = AHI_STATUS_SUCCESS;

    if (!ahiIsInitialised())
        status = AHI_STATUS_REJECTED;

    if (ahiGetTransportTask() != req->transport_task)
        status = AHI_STATUS_NOT_FOUND;
    
    /*
       Cancel any delayed vm-to-host messages to ensure that the Host does not
       receive any messages after AHI_DISCONNECT_CFM.
    */
    MessageCancelAll(ahiTask(), AHI_INTERNAL_DELAYED_SEND_DATA);

    ahiSendDisconnectCfm(req->transport_task, status);

    /* Don't remove the transport task until after the cfm msg has been sent.*/
    if (ahiGetTransportTask() == req->transport_task)
        ahiSetTransportTask(0);

#ifdef ENABLE_REBOOT_AFTER_DISCONNECT
    if (ahiIsRebootNeeded())
    {
        /* If a reboot is required, send a delayed msg to ahi_task to perform
           the reboot. */
        MessageSend(ahiTask(), AHI_INTERNAL_APP_MODE_SET_REBOOT, 0);
    }
#endif
}

/******************************************************************************
NAME
    handleConfigGetReq

DESCRIPTION
    Attempt to get a (read-only) pointer to a config data block and send the 
    data back to the Host.
    
    If we fail to get a read-only pointer to the config data block we still
    need to send a response with an error code to the Host.
*/
static void handleConfigGetReq(AHI_CONFIG_GET_REQ_T *req)
{
    config_store_status_t status;
    AHI_CONFIG_GET_CFM_T cfm;

    PRINT(("AHI:   req->config_id 0x%x\n", req->config_id));

    memset(&cfm, 0, sizeof(cfm));
    cfm.config_id = req->config_id;
    
    if (AhiGetSupportedMode() != ahi_app_mode_configuration)
    {
        /* Operation only allowed in config mode. */
        cfm.status = AHI_STATUS_REJECTED;
        ahiSendConfigGetCfm(ahiGetTransportTask(), &cfm);
        return;
    }

    status = ConfigStoreGetReadOnlyConfig(cfm.config_id, &cfm.size, &cfm.data);

    cfm.status = convertConfigStatusToAhiStatus(status);
    if (AHI_STATUS_SUCCESS != cfm.status)
    {
        cfm.size = 0;
        cfm.data = 0;
    }
    else
    {
        /* Convert size from hextets (16 bit words) to octets (8bit words) */
        cfm.size = (cfm.size << 1);
    }

    ahiSendConfigGetCfm(ahiGetTransportTask(), &cfm);
}

/******************************************************************************
NAME
    handleConfigGetReqComplete

DESCRIPTION
    Release the config data block that was acquired by handleConfigGetReq().

    This should only be called via an AHI_INTERNAL_CONFIG_GET_REQ_COMPLETE
    msg.
*/
static void handleConfigGetReqComplete(AHI_INTERNAL_CONFIG_GET_REQ_COMPLETE_T *msg)
{
    ConfigStoreReleaseConfig(msg->config_id);
}

/******************************************************************************
NAME
    handleConfigSetReq

DESCRIPTION
    Attempt to get a writeable pointer to a config data block and
    write the new config data into it.

    If we fail to get a writeable pointer to the config data block we still
    need to send a response with an error code to the Host.
*/
static void handleConfigSetReq(AHI_CONFIG_SET_REQ_T *req)
{
    config_store_status_t status;
    AHI_CONFIG_SET_CFM_T cfm;
    config_set_ctx *ctx = 0;

    PRINT(("AHI:   req 0x%p config_id 0x%x size %u data 0x%p data_size %u\n",
        (void *)req, req->config_id, req->size, req->data, req->data_size));

    if (AhiGetSupportedMode() != ahi_app_mode_configuration)
    {
        /* Operation only allowed in config mode. */
        cfm.config_id = req->config_id;
        cfm.status = AHI_STATUS_REJECTED;
        ahiSendConfigSetCfm(ahiGetTransportTask(), &cfm);
        return;
    }

    ctx = ahiGetConfigSetContext();
    if (!ctx)
    {
        /* Check data_size is not greater than the remaining config block data size. */
        if (req->data_size > req->size)
        {
            cfm.config_id = req->config_id;
            cfm.status = AHI_STATUS_BAD_PARAMETER;
            ahiSendConfigSetCfm(ahiGetTransportTask(), &cfm);
            return;
        }

        ctx = ahiConfigSetContextCreate(req->config_id, req->size);
        ahiSetConfigSetContext(ctx);
    }
    else
    {
        /* Check if existing context matches the new request. */
        if (ctx->id != req->config_id
            && ctx->size != req->size)
        {
            /* New request does not match the one in progress so reject it. */
            cfm.config_id = req->config_id;
            cfm.status = AHI_STATUS_ALREADY_IN_USE;
            ahiSendConfigSetCfm(ahiGetTransportTask(), &cfm);
            return;
        }
    }

    if(!ctx->size)  /* The config block data was removed */
    {
        cfm.config_id = req->config_id;
        cfm.status = ctx->status;
        ahiSendConfigSetCfm(ahiGetTransportTask(), &cfm);
        /* Destroy the config_set context. */
        ahiSetConfigSetContext(0);
        ahiConfigSetContextDestroy(ctx);
        return;
    }

    PRINT(("AHI:   ctx 0x%p id 0x%x data 0x%p size %u offset %u status 0x%x\n",
            (void *)ctx, ctx->id, (void *)ctx->data, ctx->size, ctx->offset, ctx->status));

    /* Check data_size is not greater than the remaining config block data size. */
    if (req->data_size > (ctx->size - ctx->offset))
    {
        ctx->status = AHI_STATUS_BAD_PARAMETER;
    }

    /* Write data into the config block. */
    if (ctx->data && AHI_STATUS_SUCCESS == ctx->status)
    {
        /* @todo Check req->data_size does not overflow the config block total size. */
        ByteUtilsMemCpy16((uint8 *)ctx->data, ctx->offset, (uint16 *)req->data, 0, req->data_size);
        ctx->offset += req->data_size;
    }

    /* If this is the last data packet, attempt to write the config block
       and send a response to the Host. */
    if (AHI_FLAG_NONE == req->flags)
    {
        if (AHI_STATUS_SUCCESS == ctx->status)
        {
            /* Check the whole requested config block was updated. */
            if (ctx->offset == ctx->size)
            {
                status = ConfigStoreWriteConfig(ctx->id);
                ctx->status = convertConfigStatusToAhiStatus(status);
            }
            else
            {
                ctx->status = AHI_STATUS_TRUNCATED;
            }
        }

        cfm.config_id = ctx->id;
        cfm.status = ctx->status;
        ahiSendConfigSetCfm(ahiGetTransportTask(), &cfm);

        /* Destroy the config_set context. */
        ahiSetConfigSetContext(0);
        ahiConfigSetContextDestroy(ctx);
    }
}

/******************************************************************************
NAME
    handleConfigFileGetReq

DESCRIPTION
    Attempt to open the config definition file on the internal filesystem and
    send it to the host.

    The config definition will be quite large so it will require sending it
    in more than one message. The AHI task needs to make sure it does not flood
    the outgoing transport task otherwise the vm will run out of memory
    to store the queued messages.
*/
static void handleConfigFileGetReq(void)
{
    AHI_CONFIG_FILE_GET_CFM_T cfm;
    AHI_CONFIG_FILE_GET_DATA_IND_T ind;
    ahi_application_mode_t app_mode;

    memset(&ind, 0, sizeof(ind));

    app_mode = AhiGetSupportedMode();
    if (app_mode == ahi_app_mode_normal
        || app_mode == ahi_app_mode_configuration)
    {
        /* Open the file and get its size. */
        cfm.status = ahiGetConfigFileSource(&ind.source, &cfm.size);
    }
    else
    {
        /* Operation only allowed in normal and config mode */
        cfm.status = AHI_STATUS_REJECTED;
        cfm.size = 0;
    }

    /* Send status response to Host. */
    ahiSendConfigFileGetCfm(ahiGetTransportTask(), &cfm);

    /* If status is ok, start sending the data. */
    if ((AHI_STATUS_SUCCESS == cfm.status) && ind.source)
    {
        ahiSendConfigFileGetDataInd(ahiGetTransportTask(), &ind);
    }
}

/******************************************************************************
NAME
    handleModeGetReq

DESCRIPTION
    Return the current AHI application mode to the Host.
*/
static void handleModeGetReq(void)
{
    AHI_MODE_GET_CFM_T cfm;

    cfm.status = AHI_STATUS_SUCCESS;
    cfm.current_mode = AhiGetSupportedMode();

    ahiSendModeGetCfm(ahiGetTransportTask(), &cfm);
}

/******************************************************************************
NAME
    handleModeSetReq

DESCRIPTION
    Ask the VM application if it is ok to change the application mode.

    The VM application must reply by calling AhiAppModeChangeCfm.
*/
static void handleModeSetReq(AHI_MODE_SET_REQ_T *msg)
{
    if (msg->app_mode == AhiGetSupportedMode())
    {
        /* Already in the requested mode so send a reply immediately. */
        AHI_MODE_SET_CFM_T cfm;
        cfm.app_mode = msg->app_mode;
        cfm.status = AHI_STATUS_SUCCESS;
        ahiSendModeSetCfm(ahiGetTransportTask(), &cfm);
    }
    else
    {
        /* Ask the VM app if it is ok to switch mode. */
        MESSAGE_MAKE(req, AHI_APP_MODE_CHANGE_REQ_T);
        req->app_mode = msg->app_mode;
        MessageSend(ahiAppTask(), AHI_APP_MODE_CHANGE_REQ, req);

        /* Store the request to compare against later. */
        ahiSetRequestedAppMode(msg->app_mode);
    }
}

/******************************************************************************
NAME
    handleEventInjectInd

DESCRIPTION
    Forward the User Event Id in the message to the VM application.
*/
static void handleEventInjectInd(AHI_EVENT_INJECT_IND_T *msg)
{
    if (!ahiGetTransportTask()
        || (AhiGetSupportedMode() != ahi_app_mode_normal_test))
        return;

    MessageSend(ahiAppTask(), msg->event, 0);
}

/******************************************************************************
NAME
    handleConfigFileSignatureReq

DESCRIPTION
    Read the configuration definition file signature / MD5 hash from the
    config store and send it back to the Host.
*/
static void handleConfigFileSignatureReq(void)
{
    AHI_CONFIG_FILE_SIGNATURE_CFM_T cfm;
    const config_store_meta_data_t *metadata;
    config_store_status_t status;

    /* Read the signature from config_store. */
    status = ConfigStoreGetConfigMetadata(&metadata);
    if (config_store_success == status)
    {
        cfm.status = AHI_STATUS_SUCCESS;
        cfm.type = AHI_SIGNATURE_MD5;
        cfm.signature = metadata->signature;
    }
    else
    {    
        cfm.status = AHI_STATUS_NOT_FOUND;
        cfm.type = AHI_SIGNATURE_UNDEFINED;
        cfm.signature = 0;
    }

    /* Send signature to the Host. */
    ahiSendConfigFileSignatureCfm(ahiGetTransportTask(), &cfm);
}

/******************************************************************************
NAME
    handleDisableTransportReq

DESCRIPTION
    
*/
static void handleDisableTransportReq(void)
{
    /* todo: Limit this to certain states only? */

    /* Ask the VM to disable the AHI transport. */
    MessageSend(ahiAppTask(), AHI_APP_TRANSPORT_DISABLE_REQ, 0);
}

/******************************************************************************
NAME
    handleProductIdGetReq

DESCRIPTION
    Return the Product ID to the Host.
*/
static void handleProductIdGetReq(void)
{
    AHI_PRODUCT_ID_GET_CFM_T cfm;

    cfm.status = AHI_STATUS_SUCCESS;
    cfm.product_id = VmalVmReadProductId();

    ahiSendProductIdGetCfm(ahiGetTransportTask(), &cfm);
}


/******************************************************************************
NAME
    handleAppBuildIdGetReq

DESCRIPTION
    Return the Application Build ID to the Host.
*/
static void handleAppBuildIdGetReq(void)
{
    AHI_APP_BUILD_ID_GET_CFM_T cfm;

    if (AhiGetAppBuildId(&cfm.app_build_id) == ahi_status_success)
    {
        cfm.status = AHI_STATUS_SUCCESS;
    }
    else
    {
        cfm.status = AHI_STATUS_REJECTED;
    }

    ahiSendAppBuildIdGetCfm(ahiGetTransportTask(), &cfm);
}


/******************************************************************************
NAME
    handleAppModeSetReboot

DESCRIPTION
    Reboot the device. Should only ever be triggered if the VM app signals
    that an AHI app_mode change requires a reboot.
    
    The reboot is delayed to allow the app_mode change confirmation
    to be sent to the Host.
*/
static void handleAppModeSetReboot(void)
{
    PRINT(("AHI: handleAppModeSetReboot 0x%x\n", ahiCanSendTransportData()));

    /* Only reboot if all messages sent to the AHI transport
       have been processed. */
    if (ahiCanSendTransportData())
    {
        BootSetMode(BootGetMode());
    }
    else
    {
        MessageSendLater(ahiTask(), AHI_INTERNAL_APP_MODE_SET_REBOOT, 0, 100);
    }
}

/******************************************************************************
NAME
    handleDelayedSendData

DESCRIPTION
    Re-send the given AHI data packet to the outgoing AHI transport.
*/
static void handleDelayedSendData(AHI_INTERNAL_DELAYED_SEND_DATA_T * msg)
{
    ahiSendDelayedSendData(msg);
}

/******************************************************************************
NAME
    handleVersionReq

DESCRIPTION
    Send the AHI protocol version supported by this device to the Host.
*/
static void handleVersionReq(void)
{
    AHI_VERSION_CFM_T cfm;
    
    cfm.status = AHI_STATUS_SUCCESS;
    cfm.major = AHI_VERSION_MAJOR;
    cfm.minor = AHI_VERSION_MINOR;
    ahiSendVersionCfm(ahiGetTransportTask(), &cfm);
}


/******************************************************************************
    Local functions 
*/
void ahiHandleMessage(Task task, MessageId id, Message message)
{
    PRINT(("AHI: ahiHandleMessage id 0x%x", id));
    UNUSED(task);

    switch (id)
    {
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_CONNECT_REQ):
        PRINT((" AHI_CONNECT_REQ\n"));
        handleConnectReq((AHI_CONNECT_REQ_T *)message);
        break;
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_DISCONNECT_REQ):
        PRINT((" AHI_DISCONNECT_REQ\n"));
        handleDisconnectReq((AHI_DISCONNECT_REQ_T *)message);
        break;
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_VERSION_REQ):
        PRINT((" AHI_VERSION_REQ\n"));
        handleVersionReq();
        break;
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_CONFIG_GET_REQ):
        PRINT((" AHI_CONFIG_GET_REQ\n"));
        handleConfigGetReq((AHI_CONFIG_GET_REQ_T *)message);
        break;
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_CONFIG_SET_REQ):
        PRINT((" AHI_CONFIG_SET_REQ\n"));
        handleConfigSetReq((AHI_CONFIG_SET_REQ_T *)message);
        break;
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_CONFIG_FILE_GET_REQ):
        PRINT((" AHI_CONFIG_FILE_GET_REQ\n"));
        handleConfigFileGetReq();
        break;
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_MODE_GET_REQ):
        PRINT((" AHI_MODE_GET_REQ\n"));
        handleModeGetReq();
        break;
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_MODE_SET_REQ):
        PRINT((" AHI_MODE_SET_REQ\n"));
        handleModeSetReq((AHI_MODE_SET_REQ_T *)message);
        break;
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_EVENT_INJECT_IND):
        PRINT((" AHI_EVENT_INJECT_IND\n"));
        handleEventInjectInd((AHI_EVENT_INJECT_IND_T *)message);
        break;
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_CONFIG_FILE_SIGNATURE_REQ):
        PRINT((" AHI_CONFIG_FILE_SIGNATURE_REQ\n"));
        handleConfigFileSignatureReq();
        break;
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_DISABLE_TRANSPORT_REQ):
        PRINT((" AHI_DISABLE_TRANSPORT_REQ\n"));
        handleDisableTransportReq();
        break;
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_PRODUCT_ID_GET_REQ):
        PRINT((" AHI_PRODUCT_ID_GET_REQ\n"));
        handleProductIdGetReq();
        break;
    case AHI_INTERNAL_MSG_ID_FROM_OPCODE(AHI_APP_BUILD_ID_GET_REQ):
        PRINT((" AHI_APP_BUILD_ID_GET_REQ\n"));
        handleAppBuildIdGetReq();
        break;
    case AHI_CONFIG_GET_CFM:
        PRINT((" AHI_CONFIG_GET_CFM\n"));
        ahiSendConfigGetCfm(ahiGetTransportTask(), (AHI_CONFIG_GET_CFM_T *)message);
        break;
    case AHI_CONFIG_FILE_GET_DATA_IND:
        PRINT((" AHI_CONFIG_FILE_GET_DATA_IND\n"));
        ahiSendConfigFileGetDataInd(ahiGetTransportTask(), (AHI_CONFIG_FILE_GET_DATA_IND_T *)message);
        break;
    case AHI_INTERNAL_CONFIG_GET_REQ_COMPLETE:
        PRINT((" AHI_INTERNAL_CONFIG_GET_REQ_COMPLETE\n"));
        handleConfigGetReqComplete((AHI_INTERNAL_CONFIG_GET_REQ_COMPLETE_T *)message);
        break;
    case AHI_INTERNAL_APP_MODE_SET_REBOOT:
        PRINT((" AHI_INTERNAL_APP_MODE_SET_REBOOT\n"));
        handleAppModeSetReboot();
        break;
    case AHI_INTERNAL_DELAYED_SEND_DATA:
        PRINT((" AHI_INTERNAL_DELAYED_SEND_DATA\n"));
        handleDelayedSendData((AHI_INTERNAL_DELAYED_SEND_DATA_T *)message);
        break;

    default:
        PRINT((" Unrecognised\n"));
    }
}

