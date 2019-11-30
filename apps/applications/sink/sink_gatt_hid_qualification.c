/*
Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
\file    
\ingroup sink_app
\brief   
   Interface to the PTS qualification for HID over GATT qualification, which includes:
  - Enabling notification for all external report reference characteristics.
*/
#include "sink_gatt_hid_qualification.h"
#include "sink_gatt_client.h"
#include "sink_debug.h"

#ifdef GATT_HID_CLIENT

#ifdef DEBUG_GATT_HID_QUALIFICATION
#define GATT_HID_QUALIFICATION_DEBUG(x) DEBUG(x)
#else
#define GATT_HID_QUALIFICATION_DEBUG(x) 
#endif

/********************************************************************************/
static void gattHidReadCCDReq(uint16 cid)
{
    gatt_client_connection_t *connection;
    gatt_client_services_t *service;

    GATT_HID_QUALIFICATION_DEBUG(("GattHid:gattHidReadCCDReq \n"));
    /*Get connection from cid */
    connection = gattClientFindByCid(cid);
    service = gattClientGetServiceData(connection);
    
    if(service && service->hidc_instance1)
    {
        /* Read CCD for Input Report for 1st instance*/
        GattHidReadCCD(service->hidc_instance1, hid_client_report_mode);
    }
    if(service && service->hidc_instance2)
    {
        /* Read CCD for Input Report for 2nd instance*/
        GattHidReadCCD(service->hidc_instance2, hid_client_report_mode);
    }
}

/*******************************************************************************/
static void gattHidEnableInputReportNotifications(uint16 cid)
{
    gatt_client_connection_t *connection;
    gatt_client_services_t *service;

    GATT_HID_QUALIFICATION_DEBUG(("GattHid:gattHidEnableInputReportNotifications \n"));
    /*Get connection from cid */
    connection = gattClientFindByCid(cid);
    service = gattClientGetServiceData(connection);
    
    if(service && service->hidc_instance1)
    {
      /* Enable Input Report Notifications for 1st instance*/
      GattHidRegisterForNotification(service->hidc_instance1, TRUE, hid_client_report_mode);
    }
    if(service && service->hidc_instance2)
    {
      /* Enable Input Report Notifications for 2nd instance if available */
      GattHidRegisterForNotification(service->hidc_instance2, TRUE, hid_client_report_mode);
    }
}

/****************************************************************************/
void sinkGattHIDExtraConfig(void)
{
    uint16 index = 0;
    gatt_client_services_t *data = NULL;
    
    for (index = 0; index < MAX_BLE_CONNECTIONS; index++)
    {
        data = gattClientGetServiceData(gattClientConnection(index));
        if (data )
        {
            /* Read external report reference characteristic value for each of the discovered instance, 
                this is required for qualification purpose.
            */
            if(data->hidc_instance1)
            {
                GattHidReadExternalReportReference(data->hidc_instance1);
            }
            if (data->hidc_instance2)
            {
                GattHidReadExternalReportReference(data->hidc_instance2);
            }
        }
    }
}

/******************************************************************************/
void sinkGattHIDQualificationMsgHandler (Task task, MessageId id, Message message)
{
    UNUSED(task);
    GATT_HID_QUALIFICATION_DEBUG(("GattHid:sinkGattHidClientQualificationMsgHandler %x\n",id));
    switch(id)
    {
        case GATT_HID_CLIENT_READ_EXT_REF_CFM:
        {
            GATT_HID_QUALIFICATION_DEBUG(("GattHid:GATT_HID_CLIENT_READ_EXT_REF_CFM status[%x]\n", ((GATT_HID_CLIENT_READ_EXT_REF_CFM_T*)message)->status));
            /* Enable notifications for Input reports for qualification purpose */
            gattHidEnableInputReportNotifications(((GATT_HID_CLIENT_READ_EXT_REF_CFM_T*)message)->cid);
        }
        break;
        case GATT_HID_CLIENT_NOTIFICATION_REG_CFM:
        {
            GATT_HID_QUALIFICATION_DEBUG(("GattHid:GATT_HID_CLIENT_NOTIFICATION_REG_CFM status[%x]\n", ((GATT_HID_CLIENT_NOTIFICATION_REG_CFM_T*)message)->status));
            /* Send Read request for Input report ccd for qualification purpose*/
            gattHidReadCCDReq(((GATT_HID_CLIENT_NOTIFICATION_REG_CFM_T*)message)->cid);
        }
        case GATT_HID_CLIENT_READ_CCD_CFM:
        {
            GATT_HID_QUALIFICATION_DEBUG(("GattHid:GATT_HID_CLIENT_READ_CCD_CFM status[%x]\n", ((GATT_HID_CLIENT_READ_CCD_CFM_T*)message)->status));
        }
        break;
        default:
        {
            GATT_HID_QUALIFICATION_DEBUG(("GattHid:Unhandled HIDC msg[%x]\n", id));
        }
        break;
    }
}

#endif /* ENABLE_AVRCP*/

