/* Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************

FILE NAME
    sink_accessory.c

DESCRIPTION
    Support for accessory

    This feature is included in  an add-on installer 

*/

#include "sink_accessory.h"
#include "sink_configmanager.h"
#include "sink_device_id.h"

#ifdef ENABLE_IAP2
#define GAIA_TRANSPORT_IAP2

#include <bdaddr.h>
#include <sink.h>
#include <stdio.h>
#include <transport_manager.h>

#include "sink_a2dp.h"
#include "sink_debug.h"
#include "sink_hid.h"
#include "sink_iap2_transport.h"
#include "sink_iap2_remote.h"
#include "sink_iap2_rtt.h"
#include "sink_events.h"
#include "sink_main_task.h"
#include "sink_private_data.h"

/* Include config store and definition headers */
#include "sink_accessory_config_def.h"
#include <config_store.h>
#include <byte_utils.h>

/* PS keys */
#define PSKEY_USB_MANUF_STRING                 0x02C1
#define PSKEY_USB_SERIAL_NUMBER_STRING  0x02C3

#define VERSION_STRING_LEN                   6

#define IAP2_IOS_APP_SIZE   40

#define PSKEY_MAX_LEN                  32

#define FIRMWARE_VERSION_OFFSET         3

typedef struct __accessory_data
{
    accessory_info      info[MAX_IAP2_CONNECTIONS];
    iap2_transport_t    transport;
    iap2_config         *iap2_init_config;
    uint16              button_state;
    uint8               data_protocol_id;
    bool               iap_hid_present_in_config;
}accessory_data_t;

static accessory_data_t gAccessory;

#define ACCESSORY  gAccessory

static const char current_language[] = "en";
static const char hid_name[] ="Headset";

static const iap2_control_message messages_tx_native_hid[] =
{
    RequestAppLaunch,
};
/* Number of transmit control messages supported by accessory with native HID */
#define MESSAGES_TX_COUNT_NATIVE_HID (sizeof(messages_tx_native_hid) / sizeof(messages_tx_native_hid[0]))

static const iap2_control_message messages_rx_native_hid[] =
{
    StartExternalAccessoryProtocolSession,
    StopExternalAccessoryProtocolSession,
    StartNativeHID
};
/* Number of recieve control messages supported by accessory with native HID */
#define MESSAGES_RX_COUNT_NATIVE_HID (sizeof(messages_rx_native_hid) / sizeof(messages_rx_native_hid[0]))

static const iap2_control_message messages_tx[] =
{
    RequestAppLaunch,
    StartHID,
    AccessoryHIDReport,
    StopHID
};
/* Number of transmit control messages supported by accessory with HID over iAP2*/
#define MESSAGES_TX_COUNT (sizeof(messages_tx) / sizeof(messages_tx[0]))

static const iap2_control_message messages_rx[] =
{
    StartExternalAccessoryProtocolSession,
    StopExternalAccessoryProtocolSession,
    DeviceHIDReport
};
/* Number of receive control messages supported by accessory with HID over iAP2*/
#define MESSAGES_RX_COUNT (sizeof(messages_rx) / sizeof(messages_rx[0]))

static iap2_hid_component hid_params[] =
{
    {HID_COMPONENT_ID, NULL, iap2_hid_component_func_headset}
};

/*******************************************************************************
NAME
    getAccessoryDeviceState

DESCRIPTION
    Gets the Accessory Device state.

RETURNS
    The Accessory Device state.
*/
static sink_accessory_state_t getAccessoryDeviceState(accessory_info * info)
{
    IAP2_DEBUG(("getAccessoryDeviceState state=%d\n", info->device_state));
    return info->device_state;
}

/*******************************************************************************
NAME
    setAccessoryDeviceState

DESCRIPTION
    Sets the Accessory Device state.

RETURNS
    None
*/
static void setAccessoryDeviceState(accessory_info * info, sink_accessory_state_t state)
{
    info->device_state = state;
    IAP2_DEBUG(("setAccessoryDeviceState state=%d\n", info->device_state));
}

/*******************************************************************************
NAME
    getBcdString

DESCRIPTION
    gets the binary coded decimal string for integer value

RETURNS
    bcd string
*/
static char* getBcdString(uint16 value)
{
    char *buffer = PanicUnlessMalloc(VERSION_STRING_LEN + 1);
    sprintf(buffer, "%X.%X.%X", value >> 8, (value & 0x00F0) >> 4, value & 0x000F);
    return buffer;
}


/****************************************************************************
NAME 
 sinkIap2GetAppName

DESCRIPTION
 Function to get name of iOS application to launch.

RETURNS
   bool: False if read fails
           True on success read
*/
static bool sinkIap2GetAppName(char *iosapp)
{
    sink_accessory_ios_app_name_config_def_t *read_config_data = NULL;
    uint16 len_bytes = 0;
    bool ret_val = FALSE;

    if(configManagerGetReadOnlyConfig(SINK_ACCESSORY_IOS_APP_NAME_CONFIG_BLK_ID,
                         (const void **)&read_config_data)  > 0)
    {
       len_bytes = ByteUtilsGetPackedStringLen((const uint16*)read_config_data->app_name, sizeof(read_config_data->app_name) / sizeof(uint16));
       if(len_bytes > 0 && len_bytes <= IAP2_IOS_APP_SIZE)
       {
          ByteUtilsMemCpyUnpackString((uint8 *) iosapp, (const uint16*)read_config_data->app_name, len_bytes);
          iosapp[len_bytes] = '\0';
          ret_val = TRUE;
       }

       configManagerReleaseConfig(SINK_ACCESSORY_IOS_APP_NAME_CONFIG_BLK_ID);
    }

    return ret_val;
}


/*******************************************************************************
NAME
    updateIap2AccessoryIdentification

DESCRIPTION
    Updates Accessory identitification for HID over IAP2

RETURNS
    None
*/
static void updateIap2AccessoryIdentification(bool native_hid)
{
   if(native_hid)
   {
        /* TX messages */
        ACCESSORY.iap2_init_config->messages_tx_count = MESSAGES_TX_COUNT_NATIVE_HID;
        ACCESSORY.iap2_init_config->messages_tx = (iap2_control_message *)messages_tx_native_hid;

        /* RX messages */
        ACCESSORY.iap2_init_config->messages_rx_count = MESSAGES_RX_COUNT_NATIVE_HID;
        ACCESSORY.iap2_init_config->messages_rx = (iap2_control_message *)messages_rx_native_hid;

        /* HID components */
        ACCESSORY.iap2_init_config->iap2_hid_component_count = 0;
        ACCESSORY.iap2_init_config->iap2_hid_components = NULL;

        hid_params[0].name = hid_name;
        ACCESSORY.iap2_init_config->bt_hid_component = hid_params;
   }
   else
   {
        /* TX messages */
        ACCESSORY.iap2_init_config->messages_tx_count = MESSAGES_TX_COUNT;
        ACCESSORY.iap2_init_config->messages_tx = (iap2_control_message *)messages_tx;

       /* RX messages */
       ACCESSORY.iap2_init_config->messages_rx_count = MESSAGES_RX_COUNT;
       ACCESSORY.iap2_init_config->messages_rx = (iap2_control_message *)messages_rx;

       /* HID components */
       ACCESSORY.iap2_init_config->iap2_hid_component_count = sizeof(hid_params) / sizeof(hid_params[0]);
       hid_params[0].name = hid_name;

       if(ACCESSORY.iap2_init_config->iap2_hid_component_count)
          ACCESSORY.iap_hid_present_in_config = TRUE;

       ACCESSORY.iap2_init_config->iap2_hid_components = hid_params;

       ACCESSORY.iap2_init_config->bt_hid_component = NULL;
   }
}

/*******************************************************************************
NAME
    sinkIap2StartApplication

DESCRIPTION
    Launch an iOS application based on name in user pskey

RETURNS
    None
*/
static void sinkIap2StartApplication(void)
{
    accessory_info *info = &(ACCESSORY.info[iap2_primary_link]);
    char app_name[IAP2_IOS_APP_SIZE + 1];

    if(sinkIap2GetAppName(app_name))
    {
        IAP2_DEBUG(("IAP2 launch \"%s\"\n", app_name));
        Iap2AppLaunchReq(info->link, app_name);
    }
    else
    {
       IAP2_DEBUG(("App name reading failed\n"));
    }    
}


/****************************************************************************
NAME
    sinkAccessoryGetUsbInfo

DESCRIPTION
     Function to get USB Manufacturer name and USB Serial Number.
*/
static const char* sinkAccessoryGetUsbInfo(uint16 pskey)
{
    uint16 length = 0;
    char* buffer = NULL;

    length = PsFullRetrieve(pskey, NULL, 0)*sizeof(uint16);
    buffer = PanicUnlessMalloc(length + 1);
    PsFullRetrieve(pskey, buffer, length);
    buffer[length] = 0;

    return buffer;
}


/****************************************************************************
NAME
    sinkIap2AccessoryIdentification

DESCRIPTION
     Initialise the iAP2 accessory identification structure

RETURNS
     None
*/
static void sinkIap2AccessoryIdentification(void)
{
    sink_accessory_readonly_config_def_t *read_config_data = NULL;
    uint16 len_bytes = 0;
    uint16  pskey_val[PSKEY_MAX_LEN];

    if(configManagerGetReadOnlyConfig(SINK_ACCESSORY_READONLY_CONFIG_BLK_ID, 
                (const void **)&read_config_data) > 0)
    {
         /* Hardware Version */
         ACCESSORY.iap2_init_config->hardware_version = getBcdString(read_config_data->accessory_version);

         /* Model ID */
         len_bytes = ByteUtilsGetPackedStringLen((const uint16*)read_config_data->accessory_model_id,
                                  sizeof(read_config_data->accessory_model_id) / sizeof(uint16));
         if (len_bytes >0)
         {
             /* Allocate extra one word for storing NULL character at the end of the string. */
             ACCESSORY.iap2_init_config->model_id = (const char*) malloc(len_bytes + 1);
             /* Unpack and assign the Config Store model id string*/
             memset((void*)ACCESSORY.iap2_init_config->model_id, 0, (len_bytes + 1));
             ByteUtilsMemCpyUnpackString((uint8 *)ACCESSORY.iap2_init_config->model_id,
                    (const uint16*)read_config_data->accessory_model_id, len_bytes);
         }

         /* AppMatch Team ID */
         len_bytes = ByteUtilsGetPackedStringLen((const uint16*)read_config_data->accessory_bundle_id,
                                sizeof(read_config_data->accessory_bundle_id) / sizeof(uint16));
         if (len_bytes > 0)
         {
              /* Allocate extra one word for storing NULL character at the end of the string. */
              ACCESSORY.iap2_init_config->app_match_team_id = (const char*) malloc(len_bytes + 1);
              /* Unpack and assign the Config Store model id string*/
              memset((void*)ACCESSORY.iap2_init_config->app_match_team_id, 0, (len_bytes + 1));
              ByteUtilsMemCpyUnpackString((uint8 *)ACCESSORY.iap2_init_config->app_match_team_id,
                          (const uint16*)read_config_data->accessory_bundle_id, len_bytes);
         }

         /* Release the config block */
         configManagerReleaseConfig(SINK_ACCESSORY_READONLY_CONFIG_BLK_ID);

         /* Manufacturer */
         ACCESSORY.iap2_init_config->manufacturer = sinkAccessoryGetUsbInfo(PSKEY_USB_MANUF_STRING);

         /* Serial Number */
         ACCESSORY.iap2_init_config->serial_number = sinkAccessoryGetUsbInfo(PSKEY_USB_SERIAL_NUMBER_STRING);

         /* Languages */
         ACCESSORY.iap2_init_config->current_language = current_language;

         /* Power */
         ACCESSORY.iap2_init_config->power_capability = iap2_power_capability_none;
         ACCESSORY.iap2_init_config->max_current_draw = 0;

         /* Firmware Version */
         GetDeviceIdFullVersion(pskey_val);
         ACCESSORY.iap2_init_config->firmware_version = getBcdString( pskey_val[FIRMWARE_VERSION_OFFSET]);

         if(sinkHidIsEnabled())
         {
             /* native HID */
             updateIap2AccessoryIdentification(TRUE);
         }
         else
         {
             /* HID over IAP2*/
             updateIap2AccessoryIdentification(FALSE);
         }
    }
    else
    {
         IAP2_DEBUG(("Missing iAP2 configuration information in accessory definition file /PS keys; Stop\n"));
         Panic();
    }
}


/****************************************************************************
NAME
    sinkIapGetLinkFromBdAddr

DESCRIPTION
    Searches through any iAP2 connections looking for a match of bdaddr, if found
    returns the link associated with that bdaddr

PARAMS
    @pAddr

RETURNS
 iap2_link_priority
*/
static iap2_link_priority sinkIap2GetLinkFromBdAddr(const bdaddr *pAddr)
{
    iap2_link_priority device_id;

    /* search all possible iAP2 connections */
    for(device_id = 0; device_id < MAX_IAP2_CONNECTIONS; device_id ++)
    {
        accessory_info *info = &(ACCESSORY.info[device_id]);
        /* if bdaddr matches that passed then return the appropriate link id */
        if(BdaddrIsSame(&info->peer_bd_addr, pAddr))
            return(device_id);
    }

    IAP2_DEBUG(("IAP2 sinkIap2GetLinkFromBdAddr, No connections found \n"));
    /* no iap2 connections matching the passed in bdaddr */
    return(iap2_invalid_link);
}
/****************************************************************************
NAME
    sinkAccessorySetHidOverIap

DESCRIPTION
    Set the hid over iap registered/supported  status.

PARAMS
    @device_index
    @status

RETURNS

*/
static void sinkAccessorySetHidOverIap(int device_index,bool status)
{
   ACCESSORY.info[device_index].hid_over_iap2 = status;
}


/****************************************************************************
NAME
     sinkIap2AddDevice

DESCRIPTION
    Add a new device for iap2 connection;

PARAMS
    @bdaddr

RETURNS
 iap2_link_priority
*/
static iap2_link_priority sinkIap2AddDevice(const bdaddr *pAddr)
{
    iap2_link_priority priority = iap2_invalid_link;
    accessory_info *info; 

    /* check whether the device has been connected or connecting */
    priority = sinkIap2GetLinkFromBdAddr(pAddr);

    /* If this device has not been added, add it first */    
    if(priority == iap2_invalid_link)
    {
        uint8 device_id = 0;
        /* search for an unassigned iAP2 connection */
        for(device_id = 0; device_id < MAX_IAP2_CONNECTIONS; device_id++)
        {
            info = &(ACCESSORY.info[device_id]);
            /* if an unassigned iAP2 entry exists return link priority */
            if( getAccessoryDeviceState(info) == sink_accessory_state_idle )
            {
                priority = (iap2_link_priority)device_id;
                return priority;
            }
        }
    }
    /* this bdaddr already has a iap connection associated with it, check its
       current state return error */
    else
    {
        /* check current state of link */
        info = &(ACCESSORY.info[priority]);
        IAP2_DEBUG(("IAP2 sinkIap2AddDevice: Device state for %d :%d\n", priority, getAccessoryDeviceState(info)));
        /* if anything other than idle return error condition */
        if( getAccessoryDeviceState(info) > sink_accessory_state_idle )
        {
            /* This device has been connected or connecting, don't try to reconnect it */
            priority  = iap2_invalid_link;
        }
    }
    /* this connection already exists and is either in the process of connecting or
       is already connected, return its priority */
    return(priority);
}

/****************************************************************************
NAME
      sinkIap2ResetLink

DESCRIPTION
    Resets the accessory info for the remote bdaddr link

PARAMS
    @bdaddr

RETURNS
     None
*/
static void sinkIap2ResetLink(const bdaddr *pAddr)
{
    /*  Get device_id from the bd_addr */
    iap2_link_priority device_id = sinkIap2GetLinkFromBdAddr(pAddr);
    accessory_info *info = &(ACCESSORY.info[device_id]);

    BdaddrSetZero(&info->peer_bd_addr);
    info->busy = FALSE;
    info->data_session_id = 0;
    info->link = NULL;
    info->hid_over_iap2 = FALSE;
    setAccessoryDeviceState(info, sink_accessory_state_idle);
}

/****************************************************************************
NAME
      sinkIap2NativeHidSelected

DESCRIPTION
    Determines if the iap2 connection with remote device uses native HID or not

PARAMS
    @bdaddr

RETURNS
     TRUE if native HID, FALSE if not.
*/
static bool sinkIap2NativeHidSelected(const bdaddr* pAddr)
{
    iap2_link_priority device_id;
    accessory_info *info;

    /* Check if the HFP/A2DP device is also an IAP device */
    if (!BdaddrIsZero(pAddr) && (sinkAccessoryConnectedToPeer(pAddr)))
    {
         device_id = sinkIap2GetLinkFromBdAddr(pAddr);
         info = &(ACCESSORY.info[device_id]);
         if(!info->hid_over_iap2)
         {
              return TRUE;
         }
    }
    return FALSE;
}

/****************************************************************************
NAME
      sinkHandleIap2InitCfm

DESCRIPTION
    Handles the iap2 initialisation confirmation from iap2 library

RETURNS
     None
*/
static void sinkHandleIap2InitCfm(const IAP2_INIT_CFM_T *cfm)
{
    if (cfm->status == iap2_status_success)
    {
        transport_mgr_link_cfg_t trans_link_cfg;

        IAP2_DEBUG(("IAP2 Init success\n"));
        ConnectionSetSdpServerMtu(IAP_MAX_SDP_ATTRIBUTE_LEN);

        IAP2_DEBUG(("reg %p %d %d\n", &theSink.task, transport_mgr_type_accessory, ACCESSORY.data_protocol_id));

        trans_link_cfg.type = transport_mgr_type_accessory;
        trans_link_cfg.trans_info.non_gatt_trans.trans_link_id = ACCESSORY.data_protocol_id;
        TransportMgrRegisterTransport(&theSink.task, &trans_link_cfg);
    }
    else
    {
        IAP2_DEBUG(("IAP2 Init failed status=%d\n", cfm->status));
    }
}


/****************************************************************************
NAME
    handleLinkCreated

DESCRIPTION
    Handles a Link Created message from the transport manager

*/
static void handleLinkCreated(TRANSPORT_MGR_LINK_CREATED_CFM_T *cfm)
{
    uint16 link_id = cfm->link_cfg.trans_info.non_gatt_trans.trans_link_id;

    if (link_id == ACCESSORY.data_protocol_id)
    {
        sinkIap2RTTInternalDataInit(link_id);
    }
}


/****************************************************************************
NAME
    handleLinkDisconnected

DESCRIPTION
    Handles a Link Disconnected message from the transport manager

*/
static void handleLinkDisconnected(TRANSPORT_MGR_LINK_DISCONNECTED_CFM_T *cfm)
{
    if (cfm->trans_link_id == ACCESSORY.data_protocol_id)
    {
    }
}


/****************************************************************************
NAME
      sinkHandleIap2ConnectInd

DESCRIPTION
    Handles the connect indication from the remote device

RETURNS
     None

*/
static void sinkHandleIap2ConnectInd(const IAP2_CONNECT_IND_T *ind)
{
     /*  Get device_id from the bd_addr */
     iap2_link_priority device_id = sinkIap2GetLinkFromBdAddr(&ind->bd_addr);
     IAP2_DEBUG(("IAP2 sinkHandleIap2ConnectInd device_id=%d\n", device_id));
     if(device_id ==  iap2_invalid_link)
     {
         /* ensure there is a free accessory connection before attempting a new connection */
         if((device_id = sinkIap2AddDevice(&ind->bd_addr)) != iap2_invalid_link)
         {
               accessory_info *info = &(ACCESSORY.info[device_id]);
               /* Accept the connection */
               info->busy = TRUE;
               info->peer_bd_addr = ind->bd_addr;
               setAccessoryDeviceState(info, sink_accessory_state_connecting);
               sinkIap2ConnectResponse(ind->link, TRUE);
         }
         else
         {
              /*  We are not paired with the device,  reject this connection */
             sinkIap2ConnectResponse(ind->link, FALSE);
         }
     }
     else 
     {
          accessory_info *info = &(ACCESSORY.info[device_id]);
          if(info != NULL && (getAccessoryDeviceState(info) == sink_accessory_state_idle  || info->busy == FALSE))
          {
              /* Accept the connection */
              info->busy = TRUE;
              setAccessoryDeviceState(info, sink_accessory_state_connecting);
              sinkIap2ConnectResponse(ind->link, TRUE);
          }
     }
}


/****************************************************************************
NAME
      sinkHandleIap2ConnectCfm

DESCRIPTION
    Handles the connect confirmation with the remote device

RETURNS
     None

*/
static void sinkHandleIap2ConnectCfm(const IAP2_CONNECT_CFM_T *cfm)
{
    IAP2_DEBUG(("sinkHandleIap2ConnectCfm status=%d\n", cfm->status));
    if(cfm->status == iap2_status_success)
    {
         /*  Get device_id from the bd_addr */
         iap2_link_priority device_id = sinkIap2GetLinkFromBdAddr(&cfm->bd_addr);
         accessory_info *info = &(ACCESSORY.info[device_id]);
         IAP2_DEBUG(("IAP2 sinkHandleIap2ConnectCfm device_id=%d\n", device_id));
         info->link = cfm->link;
         info->busy = TRUE;
         info->peer_bd_addr = cfm->bd_addr;
         setAccessoryDeviceState(info, sink_accessory_state_connected);
         /* Start identification request */
         Iap2IdentificationRequest(cfm->link);
    }
    else
    {
         IAP2_DEBUG(("sinkHandleIap2ConnectCfm, Reset the link info\n"));
         /*  Reset the  accessory info for this address */
         sinkIap2ResetLink(&cfm->bd_addr);
    }
}


/****************************************************************************
NAME
     sinkHandleIap2DisconnectInd

DESCRIPTION
     Handles the disconnect indication from the remote device

RETURNS
     None

*/
static void sinkHandleIap2DisconnectInd(const IAP2_DISCONNECT_IND_T *ind)
{
    IAP2_DEBUG(("sinkHandleIap2DisconnectInd status=%d\n", ind->status));
    /*  Reset the  accessory info for this address */
    sinkIap2ResetLink(&ind->bd_addr);
}


/****************************************************************************
NAME
      sinkHandleIap2DisconnectCfm

DESCRIPTION
    Handles the disconnect confirmation with the remote device

RETURNS
     None

*/
static void sinkHandleIap2DisconnectCfm(const IAP2_DISCONNECT_CFM_T *cfm)
{
    IAP2_DEBUG(("sinkHandleIap2DisconnectCfm status=%d\n", cfm->status));
    if(cfm->status == iap2_status_success)
    {
        /*  Reset the  accessory info for this address */
        sinkIap2ResetLink(&cfm->bd_addr);
    }
}


/****************************************************************************
NAME
      sinkHandleIap2ControlMessageInd

DESCRIPTION
    Handles the Control Message Indication from the remote device

RETURNS
     None
*/
static void sinkHandleIap2ControlMessageInd(const IAP2_CONTROL_MESSAGE_IND_T *ind)
{
     switch(ind->message_id)
     {
         case DeviceHIDReport:
             IAP2_DEBUG(("DEVICE HID REPORT\n"));
             break;
         case StartNativeHID:
             IAP2_DEBUG(("StartNativeHID\n"));
             break;
         default:
             break;
     }
}

/****************************************************************************
NAME
      sinkHandleIap2IdentCfm

DESCRIPTION
    Handles identification accepted message.

RETURNS
     None

*/
static void sinkHandleIap2IdentCfm(const IAP2_IDENTIFICATION_CFM_T *cfm)
{
    IAP2_DEBUG(("sinkHandleIap2IdentCfm status=%d\n", cfm->status));

    if(cfm->status == iap2_status_success)
    {
        bdaddr bd_addr;
       if(iap2_status_success == Iap2GetBdaddrForLink(cfm->link,&bd_addr))
       {
            uint8 device_id = sinkIap2GetLinkFromBdAddr(&bd_addr);
            if(!sinkHidIsEnabled() && ACCESSORY.iap_hid_present_in_config)
            {
                if(sinkIap2RemoteInit(device_id))
                {
                    sinkAccessorySetHidOverIap(device_id,TRUE);
                }
            }
        }
    }
}

static void handleMoreData(TRANSPORT_MGR_MORE_DATA_T *msg)
{
    transport_mgr_type_t type = msg->type;

    if (type == transport_mgr_type_accessory)
    {
        uint16 link_id = msg->trans_link_info;

        if (link_id == ACCESSORY.data_protocol_id)
        {
            uint16 size_data = TransportMgrGetAvailableDataSize(type, link_id);
            const uint8 *data = TransportMgrReadData(type, link_id);

            sinkIap2RTTHandleIncomingData(link_id, data, size_data);
            TransportMgrDataConsumed(type, link_id, size_data);
        }
    }
}

/****************************************************************************
NAME
      sinkIap2GetActiveConnection

DESCRIPTION
    Gets the Active iAP2 connection.

*/
static iap2_link_priority sinkIap2GetActiveConnection(void)
{
    bdaddr addr;
    hfp_link_priority hfp_link = hfp_invalid_link;
    a2dp_index_t a2dp_link = a2dp_invalid;
    iap2_link_priority device_id = iap2_invalid_link;
    accessory_info info;

    BdaddrSetZero(&addr);
    if(sinkAudioGetRoutedVoiceSink() && 
           HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink()) != hfp_invalid_link)
    {
        hfp_link = HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink());
        /* Get address of HFP device with active audio */
        HfpLinkGetBdaddr(hfp_link, &addr);
    }
    else
    {
        a2dp_link = a2dpGetRoutedInstanceIndex();
        if (a2dp_link != a2dp_invalid)
        {
            /* Get address of A2DP device with active audio */
            A2dpDeviceGetBdaddr(getA2dpLinkDataDeviceId(a2dp_link), &addr);
        }
    }

    /* Check if the HFP/A2DP device is also an IAP device */
    if (!BdaddrIsZero(&addr) && (sinkAccessoryConnectedToPeer(&addr)))
    {
        /*  Get the device_id for this valid iAP2 device */
        device_id = sinkIap2GetLinkFromBdAddr(&addr);
        /* As the HFP/A2DP device is also an IAP device, send HID commands instead of Bluetooth commands */
        return device_id;
    }
    
    /* This is when both sinkAudioGetRoutedVoiceSink() & sinkAudioGetRoutedAudio() is zero i.e. there is no routed audio or ,
     *  routed voice then check the current active avrcp device and check if it is also an IAP device.*/ 
     if(avrcpAvrcpIsEnabled())
     	{
           sinkAvrcpGetActiveConnectionFromCurrentIndex(&addr);
           if (!BdaddrIsZero(&addr) && (sinkAccessoryConnectedToPeer(&addr)))
           {
                /*  Get the device_id for this valid iAP2 device */
                device_id = sinkIap2GetLinkFromBdAddr(&addr);
           }
     }
     else
     {
          /*  Get the first available iAP2 connected device */
          for (device_id=0; device_id<MAX_IAP2_CONNECTIONS; device_id++)
          {
               sinkAccessoryGetDeviceInfo(&info, device_id);
               if (info.device_state == sink_accessory_state_connected)
               {
                   break;
               }
          }
     }
    return device_id;
}

/****************************************************************************
NAME
    sinkAccesoryMsgHandler

DESCRIPTION
    The task handler function to receive Accessory library messages.

RETURNS
    None
*/
void sinkAccessoryMsgHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    switch (id)
    {
    case IAP2_INIT_CFM:
        IAP2_DEBUG(("IAP2_INIT_CFM\n"));
        sinkHandleIap2InitCfm((const IAP2_INIT_CFM_T *)message);
        break;

    case IAP2_CONNECT_IND:
        IAP2_DEBUG(("IAP2_CONNECT_IND\n"));
        sinkHandleIap2ConnectInd((const IAP2_CONNECT_IND_T *)message);
        break;
    case IAP2_CONNECT_CFM:
        IAP2_DEBUG(("IAP2_CONNECT_CFM\n"));
        sinkHandleIap2ConnectCfm((const IAP2_CONNECT_CFM_T *)message);
        break;
    case IAP2_DISCONNECT_IND:
        IAP2_DEBUG(("IAP2_DISCONNECT_IND\n"));
        sinkHandleIap2DisconnectInd((const IAP2_DISCONNECT_IND_T *)message);
        break;
    case IAP2_DISCONNECT_CFM:
        IAP2_DEBUG(("IAP2_DISCONNECT_CFM\n"));
        sinkHandleIap2DisconnectCfm((const IAP2_DISCONNECT_CFM_T *)message);
        break;
    case IAP2_CONTROL_MESSAGE_IND:
        IAP2_DEBUG(("IAP2_CONTROL_MESSAGE_IND\n"));
        sinkHandleIap2ControlMessageInd((const IAP2_CONTROL_MESSAGE_IND_T *)message);
        break;

    case IAP2_IDENTIFICATION_CFM:
        IAP2_DEBUG(("IAP2_IDENTIFICATION_CFM\n"));
        sinkHandleIap2IdentCfm((const IAP2_IDENTIFICATION_CFM_T *)message);
        break;

    case TRANSPORT_MGR_LINK_CREATED_CFM:
        IAP2_DEBUG(("TRANSPORT_MGR_LINK_CREATED_CFM\n"));
        handleLinkCreated((TRANSPORT_MGR_LINK_CREATED_CFM_T *) message);
        break;

    case TRANSPORT_MGR_LINK_DISCONNECTED_CFM:
        IAP2_DEBUG(("TRANSPORT_MGR_LINK_DISCONNECTED_CFM\n"));
        handleLinkDisconnected((TRANSPORT_MGR_LINK_DISCONNECTED_CFM_T *) message);
        break;
        
    case TRANSPORT_MGR_MORE_DATA:
        IAP2_DEBUG(("TRANSPORT_MGR_MORE_DATA\n"));
        handleMoreData((TRANSPORT_MGR_MORE_DATA_T *) message);
        break;

    default:
        IAP2_DEBUG(("sinkAccessoryMsgHandler unhandled 0x%04X\n", id));
        break;
    }
}

/****************************************************************************
NAME
      sinkAccessoryUpdateDeviceName

DESCRIPTION
    Updates the device name in accessory data structure

RETURNS
     None
*/
void sinkAccessoryUpdateDeviceName(const CL_DM_LOCAL_NAME_COMPLETE_T* local_name_msg)
{
    const uint8 *local_name = ((const CL_DM_LOCAL_NAME_COMPLETE_T *)local_name_msg)->local_name;
    uint8 size_local_name = (uint8) ((const CL_DM_LOCAL_NAME_COMPLETE_T *)local_name_msg)->size_local_name;
    char *buffer;

    buffer = PanicUnlessMalloc(size_local_name + 1);
    memmove(buffer, local_name, size_local_name);
    buffer[size_local_name] = 0;
    ACCESSORY.iap2_init_config->device_name = buffer;
}


/****************************************************************************
NAME
      sinkAccessoryConnectedToPeer

DESCRIPTION
    Checksif the accessory is connected to remote device

RETURNS
     None
*/
bool sinkAccessoryConnectedToPeer(const bdaddr *pAddr)
{
     if(sinkIap2GetLinkFromBdAddr(pAddr) != iap2_invalid_link)
     {
         return TRUE;
     }
     return FALSE;
}


/******************************************************************************
NAME
    sinkAccessoryInit

DESCRIPTION
    This function initialises accessory library

RETURNS
    None
*/
void sinkAccessoryInit(void)
{
    static bool done;

    if (!done)
    {
        uint16 count = 0;

        memset(&ACCESSORY, 0, sizeof(accessory_data_t));

        ACCESSORY.data_protocol_id = Iap2EARegisterProtocol("com.qtil.rtt", iap2_app_match_optional);

        if (ACCESSORY.data_protocol_id == 0)
        {
             IAP2_DEBUG(("Iap2EARegisterProtocol failed\n"));
             Panic();
        }

        /*  Allocate memory for iap2_init_config*/
        ACCESSORY.iap2_init_config = (iap2_config*)PanicNull(calloc(1, sizeof(iap2_config)));

        /* Remember local Bluetooth address */
        sinkDataGetLocalBdAddress(&ACCESSORY.iap2_init_config->local_bdaddr);

        IAP2_DEBUG(("Initialise Accessory..\n"));

        /* Initialise transport */
        ACCESSORY.transport = iap2_transport_bluetooth;

        while(count != MAX_IAP2_CONNECTIONS)
        {
            ACCESSORY.info[count].link = NULL;
            ACCESSORY.info[count].busy = FALSE;
            ACCESSORY.info[count].data_session_id = 0;
            count++;
        }

        IAP2_DEBUG(("sinkAccessoryInit1: Local address [%04X %02X %06lX]\n",
                ACCESSORY.iap2_init_config->local_bdaddr.nap, ACCESSORY.iap2_init_config->local_bdaddr.uap,
                ACCESSORY.iap2_init_config->local_bdaddr.lap));

        sinkIap2AccessoryIdentification();

        done = TRUE;
    }
}

void sinkAccessoryHwInit(void)
{
    /* Initialise iAP2 hardware */
    Iap2Init(&theSink.task, ACCESSORY.iap2_init_config);
    free((void *) ACCESSORY.iap2_init_config->hardware_version);
    free((void *) ACCESSORY.iap2_init_config->firmware_version);
    free(ACCESSORY.iap2_init_config);
}


/******************************************************************************
NAME
    sinkAccessoryConnectRequest

DESCRIPTION
    Handles the Accessory connection

RETURNS
    None
*/
void sinkAccessoryConnectRequest(bdaddr * pAddr)
{
    iap2_link_priority device_id;

    /* ensure there is a free map connection before attempting a new connection */
    if((device_id = sinkIap2AddDevice(pAddr)) != iap2_invalid_link)
    {
        accessory_info *info = &(ACCESSORY.info[device_id]);

        /* Store the address */
        info->peer_bd_addr = *pAddr;
        setAccessoryDeviceState(info, sink_accessory_state_connecting);

        sinkIap2ConnectRequest(pAddr);
    }
    else
    {
        IAP2_DEBUG(("IAP2:The connection has been started or\nThe number of connection has been MAX_IAP2_CONNECTIONS...\n"));
    }
}

/******************************************************************************
NAME
    sinkHandleAccessoryDisconnect

DESCRIPTION
     Disconnect all iAP2 connections

RETURNS
    None
*/
void sinkAccessoryDisconnect( void )
{
    iap2_link_priority device_id = 0;

    IAP2_DEBUG(("iAP2:Disconnect all\n"));

    for(device_id = 0; device_id < MAX_IAP2_CONNECTIONS; device_id ++)
    {
        accessory_info *info = &(ACCESSORY.info[device_id]);

        /* ensure connection is valid before attempting to disconnect */
        if(getAccessoryDeviceState(info) > sink_accessory_state_idle)
        {
            if(info->link == NULL)
            {
                  /* This is case where disconnect is issued immediately after connect*/
                  setAccessoryDeviceState(info, sink_accessory_state_idle);
                  Iap2ConnectCancelRequest(&info->peer_bd_addr);
            }
            else
            {
                  /* update state to disconnecting, post confirmation will be made to  idle */
                  setAccessoryDeviceState(info, sink_accessory_state_disconnecting);
                  /* Disconnect */
                  sinkIap2DisconnectRequest(info->link);
            }
        }
    }
}

/******************************************************************************
NAME
    sinkAccessoryDisconnectDevice

DESCRIPTION
     Disconnect Accessory connection with the provided device

RETURNS
    None
*/
void sinkAccessoryDisconnectDevice(const bdaddr *bd_addr)
{
    iap2_link_priority device_id = sinkIap2GetLinkFromBdAddr(bd_addr);

    IAP2_DEBUG(("iAP2:Disconnect with device\n"));

    if(device_id != iap2_invalid_link)
    {
        accessory_info *info = &(ACCESSORY.info[device_id]);

        /* ensure connection is valid before attempting to disconnect */
        if(getAccessoryDeviceState(info) > sink_accessory_state_idle)
        {
            if(info->link == NULL)
            {
                  /* This is case where disconnect is issued immediately after connect*/
                  setAccessoryDeviceState(info, sink_accessory_state_idle);
                  Iap2ConnectCancelRequest(&info->peer_bd_addr);
            }
            else
            	{
                  /* update state to disconnecting, post confirmation will be made to  idle */
                  setAccessoryDeviceState(info, sink_accessory_state_disconnecting);
                  /* Disconnect */
                  sinkIap2DisconnectRequest(info->link);
            }

            info->busy = FALSE;
            info->data_session_id = 0;
            info->link = NULL;
        }
    }
}

/******************************************************************************
NAME
    sinkAccessoryReportUserEvent

DESCRIPTION
    Handles the Accessory user event.
*/
bool sinkAccessoryReportUserEvent(uint16 id)
{
    bool native_hid = FALSE;
    bool indicateEvent = FALSE;
    iap2_link_priority active_accessory = sinkIap2GetActiveConnection();
    accessory_info info;

    if(active_accessory != iap2_invalid_link)
    {
        sinkAccessoryGetDeviceInfo(&info, active_accessory);

        if(sinkHidIsEnabled())
        {
             /*  Check if we negotiated for native hid with this remote device */
             native_hid = sinkIap2NativeHidSelected(&info.peer_bd_addr);
        }

        IAP2_DEBUG(("iAP2:native_hid:%d\n", native_hid));

        if(native_hid)
        {
             /* Send id to HID module for native HID */
             indicateEvent = sinkHidUserEvents(id);
        }
        else
        {
             /* HID over IAP2 */
             sinkIap2ReportUserEvent(id, active_accessory);
        }
    }
    return indicateEvent;
}

/******************************************************************************
NAME
    sinkAccessoryVolumeRoute

DESCRIPTION
    Handles the HID Volume to acessory device only, for non-accessory standard sink app 
    volume commands are sent.

RETURNS
    TRUE if HID volume is routed to peer device
*/
bool sinkAccessoryVolumeRoute(volume_direction direction)
{
      uint16 id;

      if(direction != same_volume)
      {
           id = (direction == increase_volume)? EventUsrMainOutVolumeUp: EventUsrMainOutVolumeDown;
           return sinkAccessoryReportUserEvent(id);
      }
      return FALSE;
}


/****************************************************************************
NAME
    sinkAccessoryGetDeviceInfo

DESCRIPTION
     Function to get Accessory Device info based on device index.

RETURNS
    None
*/
void sinkAccessoryGetDeviceInfo(accessory_info *info, int device_index)
{
    memcpy(info, &(ACCESSORY.info[device_index]), sizeof(accessory_info));
}

/****************************************************************************
NAME
    sinkAccessoryGetButtonState

DESCRIPTION
     Function to get Accessory Device button state.
*/
uint16 sinkAccessoryGetButtonState(void)
{
     return ACCESSORY.button_state;
}


/****************************************************************************
NAME
    sinkAccessorySetButtonState

DESCRIPTION
     Function to Set Accessory Device button state.
*/
void sinkAccessorySetButtonState(void)
{
     ACCESSORY.button_state = 1;
}


/****************************************************************************
NAME
    sinkAccessoResetButtonState

DESCRIPTION
     Function to Reset Accessory Device button state.
*/
void sinkAccessoryResetButtonState(void)
{
     ACCESSORY.button_state = 0;
}


/****************************************************************************
NAME
    sinkAccessoryGetTransport

DESCRIPTION
     Function to get current Transport type.
*/
iap2_transport_t sinkAccessoryGetTransport(void)
{
     return ACCESSORY.transport;
}

/*******************************************************************************
NAME
    sinkAccessoryLaunch

DESCRIPTION
    Launch an accessory-specific application

RETURNS
    void
*/
void sinkAccessoryLaunch(void)
{
    sinkIap2StartApplication();
}

#endif /* ENABLE_IAP2 */
