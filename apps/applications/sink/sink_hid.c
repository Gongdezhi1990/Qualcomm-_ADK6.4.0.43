/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_hid.c

DESCRIPTION
    Interface file to the hid profile initialisation and control functions. 
*/

#include "sink_debug.h"
#include <panic.h>
#include <ps.h>

#ifdef ENABLE_HID

#include <bdaddr.h>

#include "sink_hid.h"
#include "sink_hid_device_service_record.h"
#include "sink_avrcp.h"
#include "sink_class_of_device.h"
#include "sink_main_task.h"
#include "sink_a2dp.h"
#include "sink_devicemanager.h"

#ifdef DEBUG_HID
    #define HID_DEBUG(x) DEBUG(x)
#else
    #define HID_DEBUG(x) 
#endif

#define PSKEY_DEVICE_CLASS               0x0003

#define HID_BIT(b)  (0x01 << (b))

#define VOLUME_UP         0      /*  Also configured as Selfie button*/
#define VOLUME_DOWN       1
#define CENTER            2

#define MAX_HID_CONNECTIONS 2
#define HID_INVALID_INDEX 0xFF

#define for_all_hid(idx)      for((idx) = 0; (idx) < MAX_HID_CONNECTIONS; (idx)++)

#define BUTTON_RELEASE    0x00

#define LIMITED_DISCOVERABLE_MODE (0x002000)

static const uint32 hid_diac[] =
{
    0x9E8B00, 0x9e8b33
};

static const uint8 hid_device_service_record[] = {HID_DEVICE_SERVICE_RECORD_HEADER, HID_DEVICE_SERVICE_RECORD_FOOTER};

/* SDP record configuration */
static const hid_config hid_device_config =
{
    sizeof(hid_device_service_record),
    hid_device_service_record
};

static  const uint16 hid_init_config[ ] =
{
    L2CAP_AUTOPT_SEPARATOR,                                                           /* START */
    L2CAP_AUTOPT_MTU_OUT,          0x0030,                                            /* Maximum outbound MTU - 48 bytes */
    L2CAP_AUTOPT_FLUSH_IN,         0x0000, 0x0000, 0xFFFF, 0xFFFF,                    /* Acceptable remote flush timeout 0ms - infinite */
    L2CAP_AUTOPT_FLUSH_OUT,        0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,                    /* Acceptable local flush timeout 0ms - infinite */
    L2CAP_AUTOPT_TERMINATOR                                                           /* END */
};

/*
    @denotes the current status of HID instances
*/
typedef struct __hid_info
{
    HID *hid;
    bdaddr bd_addr;
    Sink interrupt_sink;
    bool connected;
}hid_info;

typedef struct __sink_hid_global_data
{
    hid_info info[MAX_HID_CONNECTIONS];
    HID_LIB *hid_lib;
}sink_hid_data_t;


static sink_hid_data_t sink_hid_data;

#define HIDDATA sink_hid_data

/*************************************************************************
NAME    
    getHidIndexFromBdaddr

DESCRIPTION
    This function tries to find the hid device associated with the supplied 
    bleutooth address and returns the corresponding ID

PARAMETERS
    *id           return pointer to HID device ID
    *bdaddr   pointer to the bluetooth address of host device

RETURNS
    match status of true or false, 
    *id is a return parameter, where the HID device ID
    with bluetooth address as bdaddr is returned
*/

static bool getHidIndexFromBdaddr(uint8 *id, const bdaddr * bdaddr)
{
    uint8 idx = 0;

    if(BdaddrIsZero(bdaddr))
    {
        HID_DEBUG(("getHidIndexFromBdaddr: bdaddr is NULL\n"));
        *id =HID_INVALID_INDEX;
        return FALSE;
    }

    for_all_hid(idx)
    {
        if(BdaddrIsSame(&HIDDATA.info[idx].bd_addr, bdaddr))
        {
            HID_DEBUG(("getHidIndexFromBdaddr: bdaddr found\n"));
            *id = idx;
            return TRUE;
        }
    }

    HID_DEBUG(("getHidIndexFromBdaddr: bdaddr is not found\n"));
    *id =HID_INVALID_INDEX;
    return FALSE;

}

/*************************************************************************
NAME    
    getHidIndexFromHidInstance

DESCRIPTION
    This function tries to find the hid device associated with the supplied 
    HID instance and returns the corresponding ID

PARAMETERS
    *id     return pointer to HID device ID
    *hid   pointer to the HID instance returned from the library

RETURNS
    match status of true or false
    *id is a return parameter, where the HID device ID
    with bluetooth address as bdaddr is returned
*/

static bool getHidIndexFromHidInstance(uint8 *id, HID * hid)
{
    uint8 idx = 0;

    for_all_hid(idx)
    {
        if(HIDDATA.info[idx].hid == hid)
        {
            HID_DEBUG(("getHidIndexFromHidInstance: hid instance found\n"));
            *id = idx;
            return TRUE;
        }
    }

    HID_DEBUG(("getHidIndexFromHidInstance: hid instance is not found\n"));
    *id = HID_INVALID_INDEX;
    return FALSE;

}

/*************************************************************************
NAME
    getFreeHidIndex

DESCRIPTION
    This function tries to find the hid device ID which is available.

PARAMETERS
    *id   pointer to HID device ID

RETURNS
    match status of true or false
*/
static bool getFreeHidIndex(uint8 *id)
{
    uint8 idx = 0;

    for_all_hid(idx)
    {
        if(HIDDATA.info[idx].connected == FALSE)
        {
            *id = idx;
            return TRUE;
        }
    }

    HID_DEBUG(("getFreeHidIndex: Free index not found!\n"));
    *id =HID_INVALID_INDEX;
    return FALSE;
}

/*************************************************************************
NAME
    hidGetActiveConnection

DESCRIPTION
    Returns the active HID connection based on what is currently connected/Playing.

RETURNS
    Device index of the active connection
*/
static uint8 hidGetActiveConnection(void)
{
    uint8 index = 0;
    bdaddr addr;

    if(avrcpAvrcpIsEnabled())
    {
        sinkAvrcpGetActiveConnectionFromCurrentIndex(&addr);
        HID_DEBUG((" hidGetActiveConnection: nap=0x%x, uap=0x%x, lap=0x%lx, \n", addr.nap, addr.uap, addr.lap));
        /* This is current active device , get the bd_addr of it and update the active_hid based on it */
        if(getHidIndexFromBdaddr(&index, &addr) && HIDDATA.info[index].connected)
        {
             return index;
        }
    }

    /* Get the first available connected hid device */
    for_all_hid(index)
    {
        if (HIDDATA.info[index].connected)
        {
            /* We have found a hid connected device index , search no further */
            break;
        }
    }
    if(index == MAX_HID_CONNECTIONS)
    {
         index = HID_INVALID_INDEX;
    }
    HID_DEBUG(("hidGetActiveConnection, active_hid = %d\n", index));
    return index;
}


/*************************************************************************
NAME
    resetHidLinkData

DESCRIPTION
    This function resets the HID device data

PARAMETERS
    id   HID device ID

RETURNS
    void
*/
static void resetHidLinkData(uint8 id)
{
    HID_DEBUG(("resetHidLinkData id:%d\n", id));

    HIDDATA.info[id].hid = NULL;
    HIDDATA.info[id].connected = FALSE;
    BdaddrSetZero(&HIDDATA.info[id].bd_addr);
    HIDDATA.info[id].interrupt_sink = NULL;
}

/*************************************************************************
NAME
    sinkHidGetReportResponse
    
DESCRIPTION
    Handles  dummy HID_GET_REPORT packets for Input Report

RETURNS
    void
*/
static void sinkHidGetReportResponse(void)
{
      uint8 dummy_report[1];
     uint8 active_hid;

     /* Get the active HID connection index */
     active_hid = hidGetActiveConnection();

    if(active_hid != HID_INVALID_INDEX && HIDDATA.info[active_hid].hid)
    {
         /*  Dummy report on Control channel*/
        dummy_report[0] = 0x0;
        HidGetReportResponse(HIDDATA.info[active_hid].hid, hid_success, hid_report_input, sizeof(dummy_report), dummy_report);
    }
}


/*******************************************************************************
NAME
    sinkHidInitCfmHandleEvent

DESCRIPTION
     This function handles HID INIT confirmation from the stack

PARAMETERS
    cfm     confirmation  message received from the stack

RETURNS
    void
*/
static void sinkHidInitCfmHandleEvent(const  HID_INIT_CFM_T *cfm)
{
    HID_DEBUG(("sinkHidInitCfmHandleEvent:HID_INIT_CFM %x\n", cfm->status));

    if(hid_init_success == cfm->status)
    {
        HIDDATA.hid_lib = cfm->hid_lib;
    }
    else
    {
        HID_DEBUG(("sinkHidInitCfmHandleEvent Error:Init failed!!!\n"));
    }
}

/*******************************************************************************
NAME
    sinkHidConnectIndHandleEvent
    
DESCRIPTION
    This function handles HID connection indication from the host device
    
PARAMETERS
    ind     indication  message received from the host device

RETURNS
    void
*/
static void sinkHidConnectIndHandleEvent(const  HID_CONNECT_IND_T *ind)
{
    uint8 index;

    getFreeHidIndex(&index);

    if(index == HID_INVALID_INDEX)
    {
        HidConnectResponse(ind->hid, &theSink.task, FALSE, 0, NULL);
        HID_DEBUG(("sinkHidConnectIndHandleEvent Error: reject connection as reached Max HID connections!!\n"));
        return;
    }

    HIDDATA.info[index].hid  = ind->hid;
    HIDDATA.info[index].bd_addr = ind->bd_addr;

    HidConnectResponse(HIDDATA.info[index].hid, &theSink.task, TRUE, PS_SIZE_ADJ(sizeof(hid_init_config)), hid_init_config);

    HID_DEBUG(("sinkHidConnectIndHandleEvent:Connecting to host, nap=0x%x, uap=0x%x, lap=0x%lx\n, , Dev Id %d\n", 
                    ind->bd_addr.nap, 
                    ind->bd_addr.uap, 
                    ind->bd_addr.lap, 
                    index));
}

/*******************************************************************************
NAME
    sinkHidConnectCfmHandleEvent
    
DESCRIPTION
    This function handles HID connection confirmation received from remote device
    
PARAMETERS
    cfm     confirmation message received from the host device

RETURNS
    void
*/
static void sinkHidConnectCfmHandleEvent(const HID_CONNECT_CFM_T *cfm)
{
    uint8 index;

    HID_DEBUG(("sinkHidConnectCfmHandleEvent: status=%d\n", cfm->status));

    if (cfm->status == hid_connect_success)
    {
        if(!getHidIndexFromHidInstance(&index, cfm->hid))
        {
            getFreeHidIndex(&index);
        }
        HIDDATA.info[index].hid = cfm->hid;
        HIDDATA.info[index].connected = TRUE;
        HIDDATA.info[index].interrupt_sink = cfm->interrupt_sink;
        HID_DEBUG(("sinkHidConnectCfmHandleEvent: Connected to host, nap=0x%x, uap=0x%x, lap=0x%lx, Dev Id %d\n", HIDDATA.info[index].bd_addr.nap, HIDDATA.info[index].bd_addr.uap, HIDDATA.info[index].bd_addr.lap, index));
    }
    else
    {
        if(getHidIndexFromHidInstance(&index, cfm->hid))
        {
            HIDDATA.info[index].hid  = NULL;
            BdaddrSetZero(&HIDDATA.info[index].bd_addr);
        }
    }
}

/*******************************************************************************
NAME
    sinkHidDisconnectIndHandleEvent

DESCRIPTION
    This function handles HID disconnection indication received from remote device

PARAMETERS
    ind     indication  message received from the host device

RETURNS
    void
*/
static void sinkHidDisconnectIndHandleEvent(const HID_DISCONNECT_IND_T *ind)
{
    uint8 idx = 0;
    HID_DEBUG(("sinkHidDisconnectIndHandleEvent: status=%d\n", ind->status));

    if(getHidIndexFromHidInstance(&idx, ind->hid))
    {
        HID_DEBUG((" sinkHidDisconnectIndHandleEvent:Disconnected from nap=0x%x, uap=0x%x, lap=0x%lx\n, Dev Id %d\n", 
                    HIDDATA.info[idx].bd_addr.nap, 
                    HIDDATA.info[idx].bd_addr.uap, 
                    HIDDATA.info[idx].bd_addr.lap, 
                    idx));
        resetHidLinkData(idx);
    }
}

/*******************************************************************************
NAME
    sinkHidDisconnectCfmHandleEvent
    
DESCRIPTION
    This function handles HID disconnection confirmation received from remote device
    
PARAMETERS
    cfm     confirmation message received from the host device

RETURNS
    void
*/
static void sinkHidDisconnectCfmHandleEvent(const HID_DISCONNECT_CFM_T *cfm)
{
    uint8 idx = 0;
    HID_DEBUG(("sinkHidDisconnectCfmHandleEvent: status=%d\n", cfm->status));

    if(getHidIndexFromHidInstance(&idx, cfm->hid))
    {
        HID_DEBUG((" sinkHidDisconnectCfmHandleEvent:Disconnected from nap=0x%x, uap=0x%x, lap=0x%lx, Dev Id %d\n", 
                        HIDDATA.info[idx].bd_addr.nap, 
                        HIDDATA.info[idx].bd_addr.uap, 
                        HIDDATA.info[idx].bd_addr.lap, 
                        idx));
        resetHidLinkData(idx);
    }
}

/*******************************************************************************
NAME
    sinkHidSendInputReport
    
DESCRIPTION
    This function sends HID input report to HID host device
    
PARAMETERS
    report_length     length of the input report
    input_report       payload of input report

RETURNS
    void
*/
static void sinkHidSendInputReport(uint16 report_length, const uint8 *input_report)
{
    uint8 active_hid;

    if(input_report == NULL)
    {
        HID_DEBUG(("sendInputReport input_report is NULL\n"));
        return;
    }

    active_hid = hidGetActiveConnection();

    if(active_hid != HID_INVALID_INDEX && HIDDATA.info[active_hid].hid)
    {
        HidInterruptReport(HIDDATA.info[active_hid].hid, hid_report_input, report_length, input_report);
    }
}

/*************************************************************************
NAME    
    sinkHidGetReportIndHandleEvent
DESCRIPTION
    Handles HID_GET_REPORT packets.
RETURNS
    void     
*/
static void sinkHidGetReportIndHandleEvent(const HID_GET_REPORT_IND_T *ind)
{
     uint8 active_hid;
     HID_DEBUG(("appHandleHidGetReportInd\n"));

     /* Get the active HID connection index */
     active_hid = hidGetActiveConnection();

    if(active_hid != HID_INVALID_INDEX && HIDDATA.info[active_hid].hid)
    {
        if(ind->report_type == hid_report_input)
        {
             /* The function below sends the Get Report Response for Input Report Type for PTS qualification */
             sinkHidGetReportResponse();
        }
        else if(ind->report_type == hid_report_other)
        {
             /* Invalid report type*/
             HidGetReportResponse(HIDDATA.info[active_hid].hid, hid_invalid_id, ind->report_type, 0, NULL);
        }
        else
        {
             /* Return invalid param for any other report type */
             HidGetReportResponse(HIDDATA.info[active_hid].hid, hid_invalid_param, ind->report_type, 0, NULL);
        }
    }
}

/*************************************************************************
NAME
    sinkHidSetReportIndHandleEvent
DESCRIPTION
    Handles HID_SET_REPORT packets.
RETURNS
    void
*/
static void sinkHidSetReportIndHandleEvent(const HID_SET_REPORT_IND_T *ind)
{
     uint8 active_hid;
     HID_DEBUG(("appHandleHidSetProtocolInd\n"));

     /* Get the active HID connection index */
     active_hid = hidGetActiveConnection();

    if(active_hid != HID_INVALID_INDEX && HIDDATA.info[active_hid].hid)
    {
        if(ind->report_type == hid_report_input)
        {
             HidSetReportResponse(HIDDATA.info[active_hid].hid, hid_invalid_id);
        }
        else
        {
             /* Send response  - default to invalid param as we don't support any SET_REPORTS */
             HidSetReportResponse(HIDDATA.info[active_hid].hid, hid_invalid_param);
        }
    }
}

/*************************************************************************
NAME    
    sinkHidSetProtocolIndHandleEvent
DESCRIPTION
    Handles HID_SET_PROTOCOL packets.
RETURNS
    void
*/
static void sinkHidSetProtocolIndHandleEvent(const HID_SET_PROTOCOL_IND_T *ind)
{
     uint8 active_hid;
     HID_DEBUG(("appHandleHidSetProtocolInd\n"));
     UNUSED(*ind);

     /* Get the active HID connection index */
     active_hid = hidGetActiveConnection();

    if(active_hid != HID_INVALID_INDEX && HIDDATA.info[active_hid].hid)
    {
        /* Send response - unsupported */
        HidSetProtocolResponse(HIDDATA.info[active_hid].hid, hid_unsupported);
    }
}


/*************************************************************************
NAME
    sinkHidGetProtocolIndHandleEvent
DESCRIPTION
    Handles HID_GET_PROTOCOL packets.
RETURNS
    void
*/
static void sinkHidGetProtocolIndHandleEvent(const HID_GET_PROTOCOL_IND_T *ind)
{
    uint8 active_hid;
    HID_DEBUG(("appHandleHidGetProtocolInd\n"));
    UNUSED(*ind);

     /* Get the active HID connection index */
     active_hid = hidGetActiveConnection();

    /* Send response - unsupported */
    HidGetProtocolResponse(HIDDATA.info[active_hid].hid, hid_unsupported, 0);
}

static void sinkHidQualificationLimitedDiscoverable(void)
{
    uint32 device_class = 0 ;

    PsFullRetrieve(PSKEY_DEVICE_CLASS, (uint16*)&device_class , sizeof(uint32));

    if(device_class == 0)
    {
        device_class = (AUDIO_MAJOR_SERV_CLASS | AV_MAJOR_DEVICE_CLASS);

        if(sinkA2dpEnabled())
        {
            device_class |= (AV_COD_RENDER | AV_MINOR_HEADPHONES);
        }
        else
        {
            device_class |= (AV_MINOR_HEADSET);
        }
    }

    /* HID Limited Discoverable */
    device_class |= LIMITED_DISCOVERABLE_MODE;

    ConnectionWriteClassOfDevice(device_class);

    /* Response to HID DIAC(GIAC +LIAC) enable */
    ConnectionWriteInquiryAccessCode(&theSink.task, hid_diac, 2);
}

static void sinkHidQualificationConnectRequest(void)
{
    typed_bdaddr  ag_addr;
    sink_attributes attributes;

    /* attempt to get the bd_addr of the first connected device. We don't need the attributes
    * This is for qualification purpose and only a single device should pe paired for qualification test
    */
    if(deviceManagerGetIndexedAttributes(0, &attributes, &ag_addr))
    {
        sinkHidConnectRequest(&ag_addr.addr);
    }
}


bool sinkHidUserEvents(MessageId Id)
{
    uint8 hid = 0x00;
    uint8 send_release_report = TRUE; /* send key release immediately for a press event */
    uint8 payload_data[HID_HEADSET_REMOTE_REPORT_LEN];
    bool indicateEvent = FALSE;

    HID_DEBUG(("sinkHidUserEvents id:%x\n",Id));
    switch(Id)
    {
        case EventUsrHidSelfieRelease:
        {
             send_release_report  = FALSE;
        }
        break;
        case EventUsrHidSelfiePress:
        {
             send_release_report  = FALSE;
        }
        /* Fall through*/
        case EventUsrMainOutVolumeUp:
        {
            hid = HID_BIT(VOLUME_UP);
        }
        break;
        case EventUsrMainOutVolumeDown:
        {
            hid = HID_BIT(VOLUME_DOWN);
        }
        break;
        case EventUsrSensorUp:
        {
            hid = HID_BIT(CENTER);
        }
        break;
        case EventHidQualificationConnect:
        {
            sinkHidQualificationConnectRequest();
            return indicateEvent;
        }
        case EventHidQualificationLimitedDiscoverable:
        {
            sinkHidQualificationLimitedDiscoverable();
            return indicateEvent;
        }
    }

    payload_data[0] = hid;
    sinkHidSendInputReport(HID_HEADSET_REMOTE_REPORT_LEN, payload_data);

    if(send_release_report)
    {
        payload_data[0] = BUTTON_RELEASE;
        sinkHidSendInputReport(HID_HEADSET_REMOTE_REPORT_LEN, payload_data);
    }
    return indicateEvent;
}

/*********************************************************************************/

void sinkHidHandleMessage(Task task, MessageId Id, Message msg)
{
    UNUSED(task);
    switch(Id)
    {
        case HID_INIT_CFM:
        {
            HID_DEBUG(("HID_INIT_CFM received\n"));
            sinkHidInitCfmHandleEvent((const HID_INIT_CFM_T*)msg);
        }
        break;

        case HID_CONNECT_IND:
        {
            HID_DEBUG(("HID_CONNECT_IND received\n"));
            sinkHidConnectIndHandleEvent((const HID_CONNECT_IND_T *)msg);
        }
        break;

        case HID_CONNECT_CFM:
        {
            HID_DEBUG(("HID_CONNECT_CFM received\n"));
            sinkHidConnectCfmHandleEvent((const HID_CONNECT_CFM_T *)msg);
        }
        break;

        case HID_DISCONNECT_IND:
        {
            HID_DEBUG(("HID_DISCONNECT_IND received\n"));
            sinkHidDisconnectIndHandleEvent((const HID_DISCONNECT_IND_T *)msg);
        }
        break;

        case HID_DISCONNECT_CFM:
        {
            HID_DEBUG(("HID_DISCONNECT_CFM received\n"));
            sinkHidDisconnectCfmHandleEvent((const HID_DISCONNECT_CFM_T *)msg);
        }
        break;

        case HID_GET_REPORT_IND:
        {
            HID_DEBUG(("HID_GET_REPORT_IND received\n"));
            sinkHidGetReportIndHandleEvent((const HID_GET_REPORT_IND_T *)msg);
        }
        break;
        case HID_SET_PROTOCOL_IND:
        {
            HID_DEBUG(("HID_SET_PROTOCOL_IND received\n"));
            sinkHidSetProtocolIndHandleEvent((const HID_SET_PROTOCOL_IND_T *)msg);
        }
        break;
        case HID_GET_PROTOCOL_IND:
        {
            HID_DEBUG(("HID_GET_PROTOCOL_IND received\n"));
            sinkHidGetProtocolIndHandleEvent((const HID_GET_PROTOCOL_IND_T *)msg);
        }
        break;
        case HID_SET_REPORT_IND:
        {
            HID_DEBUG(("HID_SET_PROTOCOL_IND received\n"));
            sinkHidSetReportIndHandleEvent((const HID_SET_REPORT_IND_T *)msg);
        }
        break;
        case HID_SET_IDLE_IND:
        case HID_GET_IDLE_IND:
        case HID_DATA_IND:
        case HID_CONTROL_IND:
        case HID_MESSAGE_TOP:
            HID_DEBUG(("sinkHidHandleMessage: message id %x received\n", Id));
        break;

        default:
            HID_DEBUG(("sinkHidHandleMessage: Unhandled message id %x received\n", Id));
        break;

    }
}

/*********************************************************************************/

void sinkHidConnectRequest(bdaddr * pAddr)
{
    uint8 index;

    if(getHidIndexFromBdaddr(&index, pAddr))
    {
        HID_DEBUG(("sinkHidConnectRequest: bdaddr already connected!\n"));
        return;
    }
    else if(getFreeHidIndex(&index))
    {
        HID_DEBUG(("sinkHidConnectRequest: Free HID index:%d\n", index));
        HIDDATA.info[index].bd_addr.nap= pAddr->nap;
        HIDDATA.info[index].bd_addr.uap= pAddr->uap;
        HIDDATA.info[index].bd_addr.lap= pAddr->lap;
        HidConnect(HIDDATA.hid_lib, &theSink.task, pAddr, PS_SIZE_ADJ(sizeof(hid_init_config)), hid_init_config);
    }
}

/*********************************************************************************/

void sinkHidDisconnectAllLinks(void)
{
    uint8 index;
    HID_DEBUG(("sinkHidDisconnectRequest Disconnect all\n"));

    for_all_hid(index)
    {
        if(HIDDATA.info[index].connected == TRUE)
        {
            HID_DEBUG(("sinkHidDisconnectRequest sending disconnect request to index:%d\n", index));
            HidDisconnect(HIDDATA.info[index].hid);
        }
    }
}

/*********************************************************************************/

void sinkHidDisconnectRequest(const bdaddr * pAddr)
{
    uint8 index;

    if(getHidIndexFromBdaddr(&index, pAddr))
    {
        if(HIDDATA.info[index].connected == TRUE)
        {
            HID_DEBUG((" sinkHidDisconnectRequest sending disconnect request to id\n"));
            HidDisconnect(HIDDATA.info[index].hid);
        }
    }

}

/*********************************************************************************/

void sinkHidInit(void)
{
    HID_DEBUG(("sinkHidInit\n"));

    memset(&HIDDATA, 0, sizeof(sink_hid_data_t));
    HidInit(&theSink.task, &hid_device_config);
}

#endif /*ENABLE_HID*/

