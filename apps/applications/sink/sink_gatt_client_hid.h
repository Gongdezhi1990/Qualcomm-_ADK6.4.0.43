/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/



#ifndef _SINK_GATT_CLIENT_HID_H_
#define _SINK_GATT_CLIENT_HID_H_


#ifndef GATT_ENABLED
#undef GATT_HID_CLIENT
#endif

#include <csrtypes.h>
#include <gatt_hid_client.h>

/****************************************************************************
NAME    
    sinkGattHidClientSetupAdvertisingFilter
    
DESCRIPTION
    Adds HID service to the advertising filter
*/    
#ifdef GATT_HID_CLIENT
void sinkGattHidClientSetupAdvertisingFilter(void);
#else
#define sinkGattHidClientSetupAdvertisingFilter() ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkGattHidClientInit
    
DESCRIPTION
    Initialisation of HID client service
*/    
#ifdef GATT_HID_CLIENT
void sinkGattHidClientInit(void);
#else
#define sinkGattHidClientInit() ((void)(0))
#endif


/****************************************************************************
NAME    
    sinkGattHidClientAddService
    
DESCRIPTION
    Adds HID to list of client connection service.
    
PARAMETERS
    cid             The connection ID
    start           The start handle of the hid service
    end             The end handle of the hid service
    
RETURNS    
    TRUE if the HID service was successfully added, FALSE otherwise.
*/
#ifdef GATT_HID_CLIENT
bool sinkGattHidClientAddService(uint16 cid, uint16 start, uint16 end);
#else
#define sinkGattHidClientAddService(cid, start, end) (FALSE)
#endif


/****************************************************************************
NAME    
    sinkGattHIDClientRemoveService
    
DESCRIPTION
    Removes the HID service associated with the connection ID.
    
PARAMETERS
    ghidc           The HID client pointer
    cid             The connection ID
*/
#ifdef GATT_HID_CLIENT
void sinkGattHIDClientRemoveService(GHIDC_T *ghidc, uint16 cid);
#else
#define sinkGattHIDClientRemoveService(ghidc, cid) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkGattHidClientMsgHandler
    
DESCRIPTION
    Handle messages from the GATT Client Task library
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    message The message payload
*/
#ifdef GATT_HID_CLIENT
void sinkGattHidClientMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattHidClientMsgHandler(task, id, message) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattHidClientNotificationReqForReportId
    
DESCRIPTION
    Interface Function to Enable/Disable HID notification for a report ID
    
PARAMETERS
    cid       Connection ID for which connection the notification need to be enabled
    report_id Input Report Id for the notification enabled
    enable    TRUE Enable Notification False: Disable Notification
*/
#ifdef GATT_HID_CLIENT
void sinkGattHidClientNotificationReqForReportId(uint16 cid,uint16 report_id,bool enable);
#else
#define sinkGattHidClientNotificationReqForReportId(cid,report_id,enable)  ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkGattHidClientEnabled
    
DESCRIPTION
    Returns if hid client is enabled.
*/
#ifdef GATT_HID_CLIENT
#define sinkGattHidClientEnabled(void) (TRUE)
#else
#define sinkGattHidClientEnabled(void) (FALSE)
#endif

/*******************************************************************************
NAME
     sinkGattHidAddPriorityDevicesList

DESCRIPTION
     This function add the HId device into the Priority Devices List. if it is paired and 
     initialized HID remote control.

 PARAMETERS
     cid               The connection ID.

 RETURNS
     None 
 */
 #ifdef GATT_HID_CLIENT
 void sinkGattHidAddPriorityDevicesList ( uint16 cid);
 #else
 #define sinkGattHidAddPriorityDevicesList(cid) ((void)(0))
 #endif

 /*******************************************************************************
NAME
    sinkGattHidClientGetSize
    
DESCRIPTION
    Returns the required size of HID client if enabled.

*/
#ifdef GATT_HID_CLIENT
#define sinkGattHidClientGetSize(void) (sizeof(GHIDC_T))
#else
#define sinkGattHidClientGetSize(void) (0)
#endif

#endif /* _SINK_GATT_CLIENT_HID_H_ */


