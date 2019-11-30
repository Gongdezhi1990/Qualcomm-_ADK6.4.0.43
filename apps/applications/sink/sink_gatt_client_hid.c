/****************************************************************************
Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_client_hid.c

DESCRIPTION
    Routines to handle the GATT HID Service.
*/

#include <stdlib.h>
#include <connection.h>
#include <gatt.h>
#include <vm.h>

/* Include dependent Headers */
#include "sink_ble.h"
#include "sink_gatt_client.h"
#include "sink_utils.h"
#include "sink_debug.h"
#include "sink_gatt_common.h"
#include "sink_gatt_hid_qualification.h"

/* Include Module Header */
#include "sink_gatt_client_hid.h"

#ifdef GATT_HID_CLIENT

static const uint8 hid_ble_advertising_filter[] = {GATT_SERVICE_UUID_HUMAN_INTERFACE_DEVICE & 0xFF, GATT_SERVICE_UUID_HUMAN_INTERFACE_DEVICE >> 8};

#ifdef DEBUG_GATT_HID_CLIENT
#define GATT_HID_CLIENT_DEBUG(x) DEBUG(x)
#else
#define GATT_HID_CLIENT_DEBUG(x) 
#endif


/*******************************************************************************
NAME
    gattHidFindConnection
    
DESCRIPTION
    Finds a client connection associated with a hid instance.
    
PARAMETERS
    ghidc    The hid client instance pointer
    
RETURNS    
    The client connection pointer associated with the hid instance. NULL if not found.    
    
*/
static gatt_client_connection_t *gattHidFindConnection(const GHIDC_T *ghidc)
{
    uint16 index = 0;
    gatt_client_services_t *data = NULL;
    
    if (ghidc == NULL)
    {
        return NULL;
    }

    for (index = 0; index < MAX_BLE_CONNECTIONS; index++)
    {
        data = gattClientGetServiceData(&GATT_CLIENT.connection);
        if (data && ((data->hidc_instance1== ghidc) || (data->hidc_instance2 == ghidc)))
        {
            return &GATT_CLIENT.connection;
        }
    }
    return NULL;
}

/*******************************************************************************
NAME
    gattHIDServiceInitialised
    
DESCRIPTION
    Called when the hid service is initialised.
    
PARAMETERS
    ghidc    The hid client instance pointer
    
*/
static void gattHidServiceInitialised(const GHIDC_T *ghidc)
{
    gatt_client_connection_t *conn = gattHidFindConnection(ghidc);
    
    if (conn != NULL)
    {
        gattClientDiscoveredServiceInitialised(conn);
    }        
}

/*******************************************************************************
NAME
    gattHidInitCfm
    
DESCRIPTION
    Handle the GATT_HID_CLIENT_INIT_CFM message.
    
PARAMETERS
    cfm    The GATT_HID_CLIENT_INIT_CFM message
    
*/
static void gattHidInitCfm(const GATT_HID_CLIENT_INIT_CFM_T *cfm)
{

    GATT_HID_CLIENT_DEBUG(("GattHid: GATT_HID_CLIENT_INIT_CFM status[%u]\n",
                                       cfm->status));
    if ((cfm->status == gatt_hid_client_status_success) &&
        (gattHidFindConnection(cfm->hid_client) != NULL))
    {
        /*Reset the report map parser */
        sinkGattHidRcResetParser();
        /* Read Report Map descriptors */
        GattHidReadReportMap((GHIDC_T*)cfm->hid_client);
        /* Add Hid remote */
        sinkGattHidRcAddRemote(cfm->cid);
    }
    /* The service initialisation is complete */
    gattHidServiceInitialised(cfm->hid_client);
    
}

/*******************************************************************************
NAME
    gattHidSetNotificationReportIdRegCfm
    
DESCRIPTION
    Handle the GATT_HID_CLIENT_NOTIFICATION_REPROTID_REG_CFM message.
    
PARAMETERS
    cfm    The GATT_HID_CLIENT_NOTIFICATION_REPROTID_REG_CFM_T message
    
*/
static void  gattHidSetNotificationReportIdRegCfm(const GATT_HID_CLIENT_NOTIFICATION_REPROTID_REG_CFM_T* cfm)
{
    UNUSED(cfm);
    GATT_HID_CLIENT_DEBUG(("GattHid:GATT_HID_CLIENT_NOTIFICATION_REPROTID_REG_CFM_T status[%u]\n", cfm->status));
}

/*******************************************************************************
NAME
    gattHidCtrlReqCfm
    
DESCRIPTION
    Handle the GATT_HID_CLIENT_CONTROL_REQ_CFM message.
    
PARAMETERS
    cfm    The GATT_HID_CLIENT_NOTIFICATION_REG_CFM_T message
    
*/
static void gattHidCtrlReqCfm(const GATT_HID_CLIENT_CONTROL_REQ_CFM_T*cfm)
{
    UNUSED(cfm);
    GATT_HID_CLIENT_DEBUG(("GattHid:GATT_HID_CLIENT_CONTROL_REQ_CFM_T status[%u]\n", cfm->status));
}

/*******************************************************************************
NAME
    gattHidReadReportMapCfm
    
DESCRIPTION
    Handle the GATT_HID_CLIENT_READ_REPORT_MAP_CFM message.
    
PARAMETERS
    cfm    The GATT_HID_CLIENT_READ_REPORT_MAP_CFM message
    
*/
static void gattHidReadReportMapCfm(const GATT_HID_CLIENT_READ_REPORT_MAP_CFM_T* cfm)
{
    GATT_HID_CLIENT_DEBUG(("GattHid:GATT_HID_CLIENT_READ_REPORT_MAP_CFM status[%u]\n", cfm->status));
    if(cfm->status == gatt_hid_client_status_success)
    {
        /* Invoke remote control module interface for parsing the report map descriptor */
        sinkGattHidRcProcessReportMapDescriptor(cfm->cid,cfm->size_value,(uint8*)cfm->value,cfm->more_to_come);
    }
}

/*******************************************************************************
NAME
    gattHidNotificationInd
    
DESCRIPTION
    Handle the GATT_HID_CLIENT_NOTIFICATION_IND message.
    
PARAMETERS
    ind    The GATT_HID_CLIENT_NOTIFICATION_IND_T message
    
*/
static void gattHidNotificationInd(const GATT_HID_CLIENT_NOTIFICATION_IND_T *ind)
{

#ifdef DEBUG_GATT_HID_CLIENT
    uint16 index = 0;
#endif
    GATT_HID_CLIENT_DEBUG(("GattHid:GATT_HID_CLIENT_NOTIFICATION_IND_T level[%u]\n", ind->report_id));
#ifdef DEBUG_GATT_HID_CLIENT
    for(index = 0;index< ind->size_value;index++)
    {
        GATT_HID_CLIENT_DEBUG(("GattHid:GATT_HID_CLIENT_NOTIFICATION_IND_T level[%u]\n", ind->value[index]));
    }
#endif
    /* Call remote control module to process the button presses */
    sinkGattHidRcProcessButtonPress(ind->size_value,(uint8*)ind->value,ind->cid);
}

/*******************************************************************************
NAME
    gattHidSetInstance
    
DESCRIPTION
    Sets the appropriate HID client instance
    
PARAMETERS
    cid             The connection ID
    
RETURNS    
    The HID Client Instance if set successfully, NULL otherwise.
    
*/
static GHIDC_T* gattHidSetInstance(uint16 cid)
{
    gatt_client_services_t *client_services = NULL;
    gatt_client_connection_t *connection;
    uint16 *service;
    
    connection = gattClientFindByCid(cid);
    service = gattClientGetServicePtr(connection, sizeof(GHIDC_T));

    if(service)
    {
        client_services = gattClientGetServiceData(connection);

        /* Check for free instance */
        if(!client_services->hidc_instance1)
        {
             client_services->hidc_instance1 = (GHIDC_T *)service;
             return client_services->hidc_instance1;
        }
        else if(!client_services->hidc_instance2)
        {
             client_services->hidc_instance2 = (GHIDC_T *)service;
              return client_services->hidc_instance2;
        }
    }
    return NULL;
}

/****************************************************************************/
/* Interface Functions                                                      */     
/****************************************************************************/

/****************************************************************************/
void sinkGattHidClientSetupAdvertisingFilter(void)
{
    GATT_HID_CLIENT_DEBUG(("GattHid: Add HID scan filter\n"));
    ConnectionBleAddAdvertisingReportFilter(ble_ad_type_more_uuid16, sizeof(hid_ble_advertising_filter), sizeof(hid_ble_advertising_filter), hid_ble_advertising_filter);
    ConnectionBleAddAdvertisingReportFilter(ble_ad_type_complete_uuid16, sizeof(hid_ble_advertising_filter), sizeof(hid_ble_advertising_filter), hid_ble_advertising_filter);
}


/****************************************************************************/

/****************************************************************************/
void sinkGattHidClientNotificationReqForReportId(uint16 cid,uint16 report_id,bool enable)
{
    gatt_client_connection_t *connection;
    gatt_client_services_t *service;
    bool hidc_inst1_notifi_enabled = FALSE;
    bool hidc_inst2_notifi_enabled = FALSE;

    GATT_HID_CLIENT_DEBUG(("GattHid:GattHidClient: Func(),sinkGattHidClientNotificationReqForReportId reportId[%d]\n",report_id));
    /*Get connection from cid */
    connection = gattClientFindByCid(cid);
    service = gattClientGetServiceData(connection);
    
    if(service && service->hidc_instance1)
    {
        /* Enable Notification for 1st instance*/
        hidc_inst1_notifi_enabled = GattHidRegisterNotificationForReportID(service->hidc_instance1,enable,report_id,hid_client_report_mode);
    }
    if(service && service->hidc_instance2)
    {
        /* Enable Notification for 2nd instance if available */
        hidc_inst2_notifi_enabled = GattHidRegisterNotificationForReportID(service->hidc_instance2,enable,report_id,hid_client_report_mode);
    }
    if(hidc_inst1_notifi_enabled || hidc_inst2_notifi_enabled)
    {
        sinkGattHidAddPriorityDevicesList(cid);
    }
}

/****************************************************************************/
void sinkGattHidClientInit(void)
{
    /* Init remote task */
    sinkGattHidRcInit();
}

/****************************************************************************/
bool sinkGattHidClientAddService(uint16 cid, uint16 start, uint16 end)
{
    GHIDC_T *ghidc = NULL;
    GATT_HID_CLIENT_INIT_PARAMS_T ghidc_init;

    GATT_HID_CLIENT_DEBUG(("GattHid:sinkGattHidClientAddService() num connection [%u]",BLE.number_connections));

    ghidc = gattHidSetInstance(cid);
    if (ghidc)
    {
        ghidc_init.cid = cid;
        ghidc_init.start_handle = start;
        ghidc_init.end_handle = end;
        if (GattHidClientInit(sinkGetBleTask(),ghidc, &ghidc_init, NULL))
        {    
            GATT_HID_CLIENT_DEBUG(("GattHid:Init Success\n"));           
            return TRUE;
        }
    }

    GATT_HID_CLIENT_DEBUG(("GattHid:Init Failed\n"));
    return FALSE;
}

/****************************************************************************/
void sinkGattHIDClientRemoveService(GHIDC_T *ghidc, uint16 cid)
{
    UNUSED(cid);
    /* Deinit GATT HID client */
    GattHidClientDeInit(ghidc);
}

/***************************************************************************/
void sinkGattHidAddPriorityDevicesList( uint16 cid)
{
    typed_bdaddr public_taddr;
    tp_bdaddr current_bd_addr;
    gattHidRcDevData_t *remote = sinkGattHidRcGetConnectedRemote(cid);

    /* Set the priority flag only if the connected device is HID RC */
    if (remote)
    {
        /*Retrieves the tp_bdaddr value from the given CID value. */
        if(VmGetBdAddrtFromCid(cid, &current_bd_addr))
        {
            /* Check whether given bluetooth address of the remote client device is in the paired device list. */
            if(sinkBleCheckBdAddrtIsBonded(&current_bd_addr.taddr, &public_taddr))
            {
                if(!ConnectionAuthIsPriorityDevice((const bdaddr *)&public_taddr.addr))
                {
                    /*  Set the priority flag of a device stored in the trusted device list. */
                    ConnectionAuthSetPriorityDevice((const bdaddr *)&public_taddr.addr, TRUE);
                    GATT_HID_CLIENT_DEBUG(("GattHid: Added the HId device into the Priority Devices List\n"));
                }
            }
        }
     }
}

/******************************************************************************/
void sinkGattHidClientMsgHandler (Task task, MessageId id, Message message)
{
    UNUSED(task);
    GATT_HID_CLIENT_DEBUG(("GattHid:sinkGattHidClientMsgHandler %x\n",id));
    switch(id)
    {
        case GATT_HID_CLIENT_INIT_CFM:
        {
            gattHidInitCfm((GATT_HID_CLIENT_INIT_CFM_T*)message);
        }
        break;
        case GATT_HID_CLIENT_NOTIFICATION_IND:
        {
            gattHidNotificationInd((GATT_HID_CLIENT_NOTIFICATION_IND_T*)message);
        }
        break;
        case GATT_HID_CLIENT_CONTROL_REQ_CFM:
        {
            gattHidCtrlReqCfm((GATT_HID_CLIENT_CONTROL_REQ_CFM_T*)message);
        }
        break;
        case GATT_HID_CLIENT_READ_REPORT_MAP_CFM:
        {   
            gattHidReadReportMapCfm((GATT_HID_CLIENT_READ_REPORT_MAP_CFM_T*)message);
        }
        break;
        case GATT_HID_CLIENT_NOTIFICATION_REPROTID_REG_CFM:
        {
            gattHidSetNotificationReportIdRegCfm((GATT_HID_CLIENT_NOTIFICATION_REPROTID_REG_CFM_T*)message);
        }
        break;

        /* Below messages are for HID qualification purpose, so shall be handled in the HID qualification module */
        case GATT_HID_CLIENT_READ_EXT_REF_CFM:
        case GATT_HID_CLIENT_NOTIFICATION_REG_CFM:
        case GATT_HID_CLIENT_READ_CCD_CFM:
        {
            sinkGattHIDQualificationMsgHandler(task, id, message);
        }
        break;
        default:
        {
            GATT_HID_CLIENT_DEBUG(("GattHid:Unhandled HIDC msg[%x]\n", id));
        }
        break;
    }
}

#endif /* GATT_HID_CLIENT */

