/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/


/****************************************************************************
    Header files
*/


#include "sink_private_data.h"
#include "sink_main_task.h"
#include "sink_scan.h"
#include "sink_statemanager.h"
#include "sink_device_id.h"
#include "sink_debug.h"
#include "sink_devicemanager.h"
#include "sink_hfp_data.h"
#include "sink_malloc_debug.h"

#include <string.h>
#include <stdlib.h>
#include <connection.h>
#include <hfp.h>

#ifdef ENABLE_PBAP
#include "sink_pbap.h"
#endif

#ifdef DEBUG_MAIN
#define MAIN_DEBUG(x) DEBUG(x)
    #define TRUE_OR_FALSE(x)  ((x) ? 'T':'F')   
#else
    #define MAIN_DEBUG(x) 
#endif


/****************************************************************************
    Definitions used in EIR data setup
*/

/* EIR tags */
#define EIR_TYPE_UUID16_PARTIAL             (0x02)
#define EIR_TYPE_UUID16_COMPLETE            (0x03)
#define EIR_TYPE_UUID32_PARTIAL             (0x04)
#define EIR_TYPE_UUID32_COMPLETE            (0x05)
#define EIR_TYPE_UUID128_PARTIAL            (0x06)
#define EIR_TYPE_UUID128_COMPLETE           (0x07)
#define EIR_TYPE_LOCAL_NAME_SHORT           (0x08)
#define EIR_TYPE_LOCAL_NAME_COMPLETE        (0x09)
#define EIR_TYPE_INQUIRY_TX                 (0x0A)

/* Device UUIDs */
#define BT_UUID_SERVICE_CLASS_HFP               (0x111E)
#define BT_UUID_SERVICE_CLASS_HSP               (0x1108)
#define BT_UUID_SERVICE_CLASS_HSP_HS            (0x1131)
#define BT_UUID_SERVICE_CLASS_A2DP              (0x110D)
#define BT_UUID_SERVICE_CLASS_AUDIO_SOURCE      (0x110A)
#define BT_UUID_SERVICE_CLASS_AUDIO_SINK        (0x110B)
#define BT_UUID_SERVICE_CLASS_AVRCP             (0x110E)
#define BT_UUID_SERVICE_CLASS_AVRCP_CONTROLLER  (0x110F)
#define BT_UUID_SERVICE_CLASS_PBAP              (0x1130)
#define BT_UUID_SERVICE_CLASS_PBAP_CLIENT       (0x112E)

/* Macro to acquire byte n from a multi-byte word w */
#define GET_BYTE(w, n) (((w) >> ((n) * 8)) & 0xFF)
#define EIR_UUID16(x)   GET_BYTE((x), 0),GET_BYTE((x), 1)
#define EIR_UUID32(x,y)   EIR_UUID16(x),EIR_UUID16(y)
#define EIR_UUID128(a,b,c,d,e,f,g,h)   EIR_UUID32(a,b),EIR_UUID32(c,d),EIR_UUID32(e,f),EIR_UUID32(g,h)

/* Size of fields */
#define EIR_TYPE_FIELD_SIZE       (0x01)
#define EIR_SIZE_FIELD_SIZE       (0x01)
#define EIR_NULL_SIZE             (0x01)
/* Size of data and data type */
#define EIR_DATA_SIZE(size)       ((uint8)(EIR_TYPE_FIELD_SIZE + (size)))
/* Size of data, type and length field */
#define EIR_DATA_SIZE_FULL(size)  ((uint8)(EIR_SIZE_FIELD_SIZE + EIR_DATA_SIZE(size)))
/* Size of all EIR data */
#define EIR_BLOCK_SIZE(size)      ((uint8)(EIR_NULL_SIZE + (size)))

/* UUIDs to list */
static const uint8 a2dp_uuids[] = {EIR_UUID16(BT_UUID_SERVICE_CLASS_A2DP),
                                   EIR_UUID16(BT_UUID_SERVICE_CLASS_AUDIO_SINK)
#ifdef ENABLE_PEER
                                   ,EIR_UUID16(BT_UUID_SERVICE_CLASS_AUDIO_SOURCE)
#endif
                                  };

#ifdef ENABLE_AVRCP
static const uint8 avrcp_uuids[] = {EIR_UUID16(BT_UUID_SERVICE_CLASS_AVRCP),
                                   EIR_UUID16(BT_UUID_SERVICE_CLASS_AVRCP_CONTROLLER)
                                  };
#endif

#ifdef ENABLE_PBAP
static const uint8 pbap_uuids[] = {EIR_UUID16(BT_UUID_SERVICE_CLASS_PBAP),
                                   EIR_UUID16(BT_UUID_SERVICE_CLASS_PBAP_CLIENT)
                                  };
#endif

static const uint8 hfp_uuids[]  = {EIR_UUID16(BT_UUID_SERVICE_CLASS_HFP)};
static const uint8 hsp_uuids[]  = {EIR_UUID16(BT_UUID_SERVICE_CLASS_HSP),
                                   EIR_UUID16(BT_UUID_SERVICE_CLASS_HSP_HS)
                                  };
#define SIZE_A2DP_UUIDS  ((sinkA2dpEnabled()) ? sizeof(a2dp_uuids) : 0)

#ifdef ENABLE_AVRCP
#define SIZE_AVRCP_UUIDS ((avrcpAvrcpIsEnabled()) ? sizeof(avrcp_uuids) : 0)
#else
#define SIZE_AVRCP_UUIDS 0
#endif

#ifdef ENABLE_PBAP
#define SIZE_PBAP_UUIDS  ((pbapIsEnabled()) ? sizeof(pbap_uuids): 0)
#else
#define SIZE_PBAP_UUIDS 0
#endif

#define SIZE_HFP_UUIDS   ((sinkHfpDataGetSupportedProfile() & hfp_handsfree_all) ? sizeof(hfp_uuids) : 0)
#define SIZE_HSP_UUIDS   ((sinkHfpDataGetSupportedProfile() & hfp_headset_all)   ? sizeof(hsp_uuids) : 0)

typedef enum
{
    sink_scan_no_reason         = 0,
    sink_scan_connectable       = 1 << 0,
    sink_scan_connectable_gatt  = 1 << 1
} sink_scan_connectable_reason_t;

#define SINK_SCAN_ALL_REASONS (sink_scan_connectable | sink_scan_connectable_gatt)

static hci_scan_enable scan;
sink_scan_connectable_reason_t connectable_reason;

#define setScanBit(bit)         scan |= (bit)
#define clearScanBit(bit)       scan &= ~(bit)
#define isScanBitSet(bit)       ((scan & (bit)) == (bit))
#define sinkIsDiscoverable()    isScanBitSet(hci_scan_enable_inq)

/******************************************************************************/
static void enableScanMode(hci_scan_enable scan_mode)
{
    setScanBit(scan_mode);
    ConnectionWriteScanEnable(scan);
}

/******************************************************************************/
static void disableScanMode(hci_scan_enable scan_mode)
{
    clearScanBit(scan_mode);
    ConnectionWriteScanEnable(scan);
}

/******************************************************************************/
static void configureRadio(void)
{
    radio_config_type radio_config;
    sinkInquiryGetRadioConfig(&radio_config);
    ConnectionWritePagescanActivity(radio_config.page_scan_interval, radio_config.page_scan_window);
}

/******************************************************************************/
static void enablePageScan(sink_scan_connectable_reason_t reason)
{
    configureRadio();
    enableScanMode(hci_scan_enable_page);
    connectable_reason |= reason;
}

/******************************************************************************/
static void disablePageScan(sink_scan_connectable_reason_t reason)
{   
    connectable_reason &= ~reason;

    /* On connectable timeout disable page scan even if page scan is active for GATT over BR/EDR.
       While stopping BLE advertisement, so not stop page scan if sink app is connectable */

    if((connectable_reason == sink_scan_no_reason) || (connectable_reason == sink_scan_connectable_gatt))
    {
        disableScanMode(hci_scan_enable_page);
    }
}

/******************************************************************************/
void sinkScanInit(void)
{
    scan = hci_scan_enable_off;
    connectable_reason = sink_scan_no_reason;
}

/****************************************************************************
NAME    
    sinkWriteEirData
    
DESCRIPTION
    Writes the local name, inquiry tx power and device UUIDs into device 
    EIR data

RETURNS
    void
*/
void sinkWriteEirData( const CL_DM_LOCAL_NAME_COMPLETE_T *message )
{
    uint16 size_uuids = 0;
    uint8  size = 0;
    
    uint8 *eir = NULL;
    uint8 *p = NULL;
    
    /* Determine length of EIR data */
    size_uuids = SIZE_A2DP_UUIDS + SIZE_AVRCP_UUIDS + SIZE_PBAP_UUIDS + SIZE_HFP_UUIDS + SIZE_HSP_UUIDS;
    
    size = GetDeviceIdEirDataSize() + EIR_BLOCK_SIZE(EIR_DATA_SIZE_FULL(size_uuids) + EIR_DATA_SIZE_FULL(message->size_local_name) + EIR_DATA_SIZE_FULL(sizeof(uint8)));    
    
    /* Allocate space for EIR data */
    eir = (uint8 *)mallocPanic(size);
    p = eir;

    /* Device Id Record */
    p += WriteDeviceIdEirData( p );
    
    /* Inquiry Tx Field */
    *p++ = EIR_DATA_SIZE(sizeof(int8));
    *p++ = EIR_TYPE_INQUIRY_TX;
    *p++ = sinkInquiryGetInquiryTxPower();
    
    /* UUID16 field */
    *p++ = EIR_DATA_SIZE(size_uuids);
    *p++ = EIR_TYPE_UUID16_PARTIAL;

    if(sinkA2dpEnabled())
    {
        memmove(p, a2dp_uuids, sizeof(a2dp_uuids));
        p += sizeof(a2dp_uuids);
    }  
    
#ifdef ENABLE_AVRCP 
    if (avrcpAvrcpIsEnabled())
    {
        memmove(p, avrcp_uuids, sizeof(avrcp_uuids));
        p += sizeof(avrcp_uuids);
    }
#endif      

#ifdef ENABLE_PBAP
    if (pbapIsEnabled())
    {
        memmove(p, pbap_uuids, sizeof(pbap_uuids));
        p += sizeof(pbap_uuids);
    }
#endif

    if(sinkHfpDataGetSupportedProfile() & hfp_handsfree_all)
    {
        memmove(p, hfp_uuids, sizeof(hfp_uuids));
        p += sizeof(hfp_uuids);
    }
    if(sinkHfpDataGetSupportedProfile() & hfp_headset_all)
    {
        memmove(p, hsp_uuids, sizeof(hsp_uuids));
        p += sizeof(hsp_uuids);
    }
    
    /* Device Name Field */
    *p++ = EIR_DATA_SIZE(message->size_local_name);  
    *p++ = EIR_TYPE_LOCAL_NAME_COMPLETE;
    memmove(p, message->local_name, message->size_local_name);
    p += message->size_local_name;
    
    /* NULL Termination */
    *p++ = 0x00; 
    
    /* Register and free EIR data */
    ConnectionWriteEirData(FALSE, size, eir);
    freePanic(eir);
}


/****************************************************************************
NAME    
    sinkEnableConnectable
    
DESCRIPTION
    Make the device connectable 

RETURNS
    void
*/
void sinkEnableConnectable( void )
{
    MAIN_DEBUG(("MP Enable Connectable %ci\n", sinkIsDiscoverable() ? '+' : '-'));
    enablePageScan(sink_scan_connectable);
}


/****************************************************************************
NAME    
    sinkDisableConnectable
    
DESCRIPTION
    Take device out of connectable mode.

RETURNS
    void
*/
void sinkDisableConnectable( void )
{
    MAIN_DEBUG(("MP Disable Connectable %ci\n", sinkIsDiscoverable() ? '+' : '-'));
    disablePageScan(sink_scan_connectable);
}

/******************************************************************************/
bool sinkIsConnectable(void)
{
    return isScanBitSet(hci_scan_enable_page);
}

/****************************************************************************
NAME    
    sinkEnableDiscoverable
    
DESCRIPTION
    Make the device discoverable. 

RETURNS
    void
*/
void sinkEnableDiscoverable( void )
{
    const uint32 giac = 0x9E8B33;
    const uint32 liac = 0x9E8B00;
    radio_config_type radioConfig;
    sinkInquiryGetRadioConfig(&radioConfig);
    MAIN_DEBUG(("MP Enable Discoverable %cp\n", sinkIsConnectable() ? '+' : '-'));
    
    /* Set inquiry access code to respond to */
    if ((sinkInquiryIsInqSessionNormal()) || (!peerUseLiacForPairing()))
    {
        ConnectionWriteInquiryAccessCode(&theSink.task, &giac, 1);
    }
    else
    {
        ConnectionWriteInquiryAccessCode(&theSink.task, &liac, 1);
    }
    
    /* Set the inquiry scan params */
    ConnectionWriteInquiryscanActivity(radioConfig.inquiry_scan_interval,radioConfig.inquiry_scan_window);

    enableScanMode(hci_scan_enable_inq);
}


/****************************************************************************
NAME    
    sinkDisableDiscoverable
    
DESCRIPTION
    Make the device non-discoverable. 

RETURNS
    void
*/
void sinkDisableDiscoverable( void )
{
    MAIN_DEBUG(("MP Disable Discoverable %cp\n", sinkIsConnectable() ? '+' : '-'));
    disableScanMode(hci_scan_enable_inq);
}


/****************************************************************************
NAME    
    sinkEnableMultipointConnectable
    
DESCRIPTION
    when in multi point mode check to see if device can be made connectable,
    this will be when only one AG is currently connected. this function will
    be called upon certain button presses which will reset the 60 second timer
    and allow a second AG to connect should the device have become non discoverable
    
RETURNS
    none
*/
void sinkEnableMultipointConnectable( void )
{    
    /* only applicable to multipoint devices and don't go connectable when taking or making
       an active call, allow connectable in streaming music state */
    if((sinkDataIsMultipointEnabled())&&(stateManagerGetState() != deviceLimbo))
    {
       /* if only one hfp instance is connected then set connectable to active */
       if(deviceManagerNumConnectedDevs() < 2)
       {
            MAIN_DEBUG(("MP Go Conn \n" ));
            
            /* make device connectable */
            sinkEnableConnectable();
         
            /* cancel any currently running timers that would disable connectable mode */
            MessageCancelAll( &theSink.task, EventSysConnectableTimeout );
            
            /* remain connectable for a further 'x' seconds to allow a second 
               AG to be connected if non-zero, otherwise stay connecatable forever */
            if(sinkDataGetConnectableTimeout())
            {
                MessageSendLater(&theSink.task, EventSysConnectableTimeout, 0, D_SEC(sinkDataGetConnectableTimeout()));
            }
       }
       /* otherwise do nothing */
    }
}

/******************************************************************************/
void sinkEnableGattConnectable(void)
{
    if(stateManagerGetState() != deviceLimbo)
    {
        /* Enable page scan only if max number of connections have not been reached */
        if(deviceManagerCanConnect())   
        {
            enablePageScan(sink_scan_connectable_gatt);
        }
    }
}

/******************************************************************************/
void sinkDisableGattConnectable(void)
{
    disablePageScan(sink_scan_connectable_gatt);
}

/******************************************************************************/
void sinkDisableAllConnectable(void)
{
    disablePageScan(SINK_SCAN_ALL_REASONS);
}
