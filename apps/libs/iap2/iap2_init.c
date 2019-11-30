/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_init.c
DESCRIPTION
    This file contains the initialisation code for the iAP2 profile library.
*/

#include "iap2_init.h"
#include "iap2_cp.h"

iap2_lib_data *iap2_lib = NULL;

static const uint8 iap2_service_record[] =
{
    /* ServiceClassIDList(0x0001) */
    0x09,
        0x00, 0x01,
    /* DataElSeq 17 bytes */
    0x35,
    0x11,
        /* 16 byte uuid  0x00000000decafadedecadeafdecacaff */
        0x1c,
        0x00, 0x00, 0x00, 0x00, 0xde, 0xca, 0xfa, 0xde,
        0xde, 0xca, 0xde, 0xaf, 0xde, 0xca, 0xca, 0xff,
    /* ProtocolDescriptorList(0x0004) */
    0x09,
        0x00, 0x04,
    /* DataElSeq 12 bytes */
    0x35,
    0x0c,
        /* DataElSeq 3 bytes */
        0x35,
        0x03,
            /* uuid L2CAP(0x0100) */
            0x19,
            0x01, 0x00,
        /* DataElSeq 5 bytes */
        0x35,
        0x05,
            /* uuid RFCOMM(0x0003) */
            0x19,
            0x00, 0x03,
            /* uint8 IAP2_DEFAULT_CHANNEL */
            0x08,
                IAP2_DEFAULT_CHANNEL
};

/* Append string src to dst, return pointer one character beyond resulting dst */
static char *append(char *dst, const char *src)
{
    while (*src)
    {
        *dst++ = *src++;
    }
    
    *dst++ = '\0';
    return dst;
}

static bool packConfig(iap2_config *config)
{
    if (config)
    {
        char *ptr;
        size_t config_data_size = 0;
        
        iap2_lib->config = PanicNull(malloc(sizeof (iap2_config)));
        
        iap2_lib->config->device_name = config->device_name;
        iap2_lib->config->local_bdaddr = config->local_bdaddr;
        
        config_data_size    = strlen(config->model_id) + 1
                            + strlen(config->manufacturer) + 1
                            + strlen(config->serial_number) + 1
                            + strlen(config->firmware_version) + 1
                            + strlen(config->hardware_version) + 1
                            + strlen(config->app_match_team_id) + 1
                            + strlen(config->current_language) + 1;
                            
    /*  Concatenate strings with terminators */
        ptr = PanicNull(malloc(config_data_size));                   
        
        iap2_lib->config->model_id = ptr;
        ptr = append(ptr, config->model_id);
        
        iap2_lib->config->manufacturer = ptr;
        ptr = append(ptr, config->manufacturer);
        
        iap2_lib->config->serial_number = ptr;
        ptr = append(ptr, config->serial_number);
        
        iap2_lib->config->firmware_version = ptr;
        ptr = append(ptr, config->firmware_version);
        
        iap2_lib->config->hardware_version = ptr;
        ptr = append(ptr, config->hardware_version);
        
        iap2_lib->config->app_match_team_id = ptr;
        ptr = append(ptr, config->app_match_team_id);
        
        iap2_lib->config->current_language = ptr;
        ptr = append(ptr, config->current_language);
        
        iap2_lib->config->power_capability = config->power_capability;
        iap2_lib->config->max_current_draw = config->max_current_draw;
        
    /*  Concatenate TX and RX messages */
        config_data_size = (config->messages_tx_count + config->messages_rx_count) * sizeof (iap2_control_message);
        iap2_lib->config->messages_tx = PanicNull(malloc(config_data_size));
        iap2_lib->config->messages_rx = iap2_lib->config->messages_tx + config->messages_tx_count;
        
        iap2_lib->config->messages_tx_count = config->messages_tx_count;
        iap2_lib->config->messages_rx_count = config->messages_rx_count;
        
        memcpy(iap2_lib->config->messages_tx, config->messages_tx, config->messages_tx_count * sizeof (iap2_control_message));
        memcpy(iap2_lib->config->messages_rx, config->messages_rx, config->messages_rx_count * sizeof (iap2_control_message));
        
    /*  Store HID component config  */
        iap2_lib->config->iap2_hid_component_count = config->iap2_hid_component_count;
        iap2_lib->config->iap2_hid_components = config->iap2_hid_components;
        iap2_lib->config->bt_hid_component = config->bt_hid_component;
        
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* Send IAP2_INIT_CFM with the given status to the application task */
static void sendInitCfm(iap2_status_t status)
{
    MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INIT_CFM, status);
    MessageSend(iap2_lib->app_task, IAP2_INIT_CFM, msg);
}

void Iap2Init(Task task, iap2_config *config)
{
    UNUSED(task);

    if (iap2_lib)
    {
        DEBUG_PRINT_INIT(("iAP2 library already initialised.\n"));
    }
    else
    {
        iap2_lib =(iap2_lib_data *)PanicNull(calloc(1, sizeof(iap2_lib_data)));

        iap2_lib->task.handler = iap2LibHandler;
        iap2_lib->app_task = task;
        iap2_lib->server_channel = IAP2_DEFAULT_CHANNEL;
        iap2_lib->links = NULL;

        if (packConfig(config))
        {
            DEBUG_PRINT_INIT(("iAP2 library coprocessor init\n"));
            iap2CpInit();
        }
        else
        {
            DEBUG_PRINT_INIT(("iAP2 bad config\n"));
            sendInitCfm(iap2_status_invalid_param);
        }
    }
}

void iap2HandleInternalCpInitCfm(const IAP2_INTERNAL_CP_INIT_CFM_T *cfm)
{
    DEBUG_PRINT_INIT(("iAP2 library coprocessor init cfm status=%d\n", cfm->status));

    if (cfm->status != iap2_status_success)
    {
        /* Coprocessor init failed, no need to continue */
        sendInitCfm(cfm->status);
    }
    else
    {
        /* Allocate RFCOMM channel now */
        DEBUG_PRINT_INIT(("iAP2 library allocate RFCOMM channel\n"));
        ConnectionRfcommAllocateChannel(&iap2_lib->task, iap2_lib->server_channel);
    }
}

void iap2HandleRfcommRegisterCfm(const CL_RFCOMM_REGISTER_CFM_T *cfm)
{
    DEBUG_PRINT_INIT(("iAP2 library RFCOMM register cfm status=%d channel=%d\n", cfm->status, cfm->server_channel));

    if (cfm->status == success)
    {
        uint8 *service_record = NULL;

        /* Remember server channel */
        iap2_lib->server_channel = cfm->server_channel;

        if (cfm->server_channel == IAP2_DEFAULT_CHANNEL)
        {
            /* Default server channel is in use, use constant */
            service_record = (uint8 *)iap2_service_record;
        }
        else
        {
            service_record = MALLOC(sizeof(iap2_service_record));

            if (service_record == NULL)
            {
                sendInitCfm(iap2_status_sdp_reg_fail);
            }
            else
            {
                memmove(service_record, iap2_service_record, sizeof(iap2_service_record));
                /* Last byte is server channel */
                service_record[sizeof(iap2_service_record) - 1] = cfm->server_channel;
            }
        }

        if (service_record)
        {
            DEBUG_PRINT_INIT(("iAP2 library register service record\n"));
            ConnectionRegisterServiceRecord(&iap2_lib->task, sizeof(iap2_service_record), service_record);
        }
    }
    else
    {
        /* Failed to register RFCOMM channel */
        sendInitCfm(iap2_status_rfcomm_reg_fail);
    }
}

void iap2HandleClSdpRegisterCfm(const CL_SDP_REGISTER_CFM_T *cfm)
{
    DEBUG_PRINT_INIT(("iAP2 library service record register cfm status=%d\n", cfm->status));

    if (cfm->status == sds_status_success)
    {
        /* Service record registered, we've finished the initialisazion */
        sendInitCfm(iap2_status_success);
        DEBUG_PRINT_INIT(("iAP2 library init completed\n"));
    }
    else if (cfm->status != sds_status_pending)
    {
        /* Service record register failed, report to app task */
        sendInitCfm(iap2_status_sdp_reg_fail);
    }
}

