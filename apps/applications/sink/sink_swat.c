/*
Copyright (c) 2012 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    Interface with the Subwoofer Audio Transfer Protocol and subwoofer 
    support
*/

/* Application includes */
#include "sink_audio_routing.h"

#include "sink_swat.h"
#include "sink_debug.h"
#include "sink_inquiry.h"
#include "sink_scan.h"
#include "sink_states.h"
#include "sink_statemanager.h"
#include "sink_devicemanager.h"
#include "sink_config.h"
#include "sink_link_policy.h"
#include "sink_main_task.h"
#include "sink_private_data.h"

#ifdef ENABLE_SUBWOOFER
#include "sink_swat_config_def.h"
#include "connection_no_ble.h"
/* Library includes */
#include <swat.h>
#include <connection.h>
#include <source.h>
#include <bdaddr.h>
#include <audio.h>
#include <audio_plugin_if.h>
#include <audio_plugin_music_params.h>
#include <gain_utils.h>
#include <config_store.h>

/* Firmware includes */
#include <bdaddr.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include <ps.h>
#include <stdio.h>
#include <print.h>

#define NUM_SUB_TRIMS 11

/* The SWAT sub trim gain table used to send dB values to the subwoofer */
static const uint8 swat_sub_trim_table[ NUM_SUB_TRIMS ] = {
    20,  /* Sub gain 0, -10 dB */
    18,  /* Sub gain 1,  -9 dB */
    16,  /* Sub gain 2,  -8 dB */
    14,  /* Sub gain 3,  -7 dB */
    12,  /* Sub gain 4,  -6 dB */
    10,  /* Sub gain 5,  -5 dB */
    8,   /* Sub gain 6,  -4 dB */
    6,   /* Sub gain 7,  -3 dB */
    4,   /* Sub gain 8,  -2 dB */
    2,   /* Sub gain 9,  -1 dB */
    0    /* Sub gain 10,  0 dB */

};

#define MAKE_SWAT_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);

typedef struct __sink_swat_global_data_t
{
        sync_config_params  esco_params;
        bdaddr           bd_addr;
        unsigned        sub_trim:8;         /* 8 bits used to keep track of the subwoofer trim gain */
        unsigned        swat_volume:8;      /* 8 bits used to keep track of the SWAT system volume */ 
        unsigned        inquiry_attempts:8; /* number of times to try an inquiry scan */   
        unsigned        sub_trim_idx:8;     /* Used to store the current sub_trim index value */
        unsigned        dev_id:4;           /* Store the subwoofer device ID for SWAT */
        unsigned        check_pairing:1;    /* Flag set when a subwoofer exists as the only PDL so start pairing */
        unsigned        unused: 11;
}sink_swat_global_data_t;

static sink_swat_global_data_t swat_global_data;

#define GSWAT swat_global_data


/****************************************************************************
NAME    
    sinkSwatCheckPairing
    
DESCRIPTION
    Helper function to check SubWoofer Pairing.
*/
bool sinkSwatCheckPairing(void)
{
    return (bool)GSWAT.check_pairing;
}

/****************************************************************************
NAME    
    sinkSwatSetCheckPairing
    
DESCRIPTION
    Function to set Subwoofer pairing status.
*/
void sinkSwatSetCheckPairing(bool value)
{
     GSWAT.check_pairing = value;
}

/****************************************************************************
NAME    
    sinkSwatGetInquiryAttempts
    
DESCRIPTION
    Helper function to get SubWoofer Inquiry Attempts.
*/
uint8 sinkSwatGetInquiryAttempts(void)
{
    return GSWAT.inquiry_attempts;
}

/****************************************************************************
NAME    
    sinkSwatSetInquiryAttempts
    
DESCRIPTION
    Helper function to set SubWoofer Inquiry Attempts.
*/
void sinkSwatSetInquiryAttempts(uint8 value)
{
    GSWAT.inquiry_attempts = value;
}

/****************************************************************************
NAME    
    sinkSwatGetDevId
    
DESCRIPTION
    Helper function to get SubWoofer Device Id.
*/
uint8 sinkSwatGetDevId(void)
{
    return GSWAT.dev_id;
}

/****************************************************************************
NAME    
    sinkSwatSetDevId
    
DESCRIPTION
    Helper function to set SubWoofer Device Id.
*/
void sinkSwatSetDevId(uint8 value)
{
    GSWAT.dev_id = value;
}

/****************************************************************************
NAME    
    sinkSwatGetSubtrimIdx
    
DESCRIPTION
    Helper function to get SubWoofer Trim Id.
*/
uint8 sinkSwatGetSubtrimIdx(void)
{
    return GSWAT.sub_trim_idx;
}

/****************************************************************************
NAME    
    sinkSwatSetSubtrimIdx
    
DESCRIPTION
    Helper function to set SubWoofer Trim Id.
*/
void sinkSwatSetSubtrimIdx(uint8 value)
{
    GSWAT.sub_trim_idx = value;
}

/****************************************************************************
NAME    
    sinkSwatGetSubtrim
    
DESCRIPTION
    Helper function to get SubWoofer Trim.
*/
uint8 sinkSwatGetSubtrim(void)
{
    return GSWAT.sub_trim;
}

/****************************************************************************
NAME    
    sinkSwatSetSubtrim
    
DESCRIPTION
    Helper function to set SubWoofer Trim.
*/
void sinkSwatSetSubtrim(uint8 value)
{
    GSWAT.sub_trim = value;
}

/****************************************************************************
NAME    
    sinkSwatGetVolume
    
DESCRIPTION
    Helper function to get SubWoofer volume.
*/
uint8 sinkSwatGetVolume(void)
{
    return GSWAT.swat_volume;
}

/****************************************************************************
NAME    
    sinkSwatSetVolume
    
DESCRIPTION
    Helper function to set SubWoofer volume.
*/
void sinkSwatSetVolume(uint8 value)
{
    GSWAT.swat_volume = value;
}

/****************************************************************************
NAME    
    sinkSwatGetConfigParams
    
DESCRIPTION
    Helper function to get SubWoofer Sync Config Params.

RETURNS
   sync_config_params
*/
sync_config_params * sinkSwatGetSyncConfigParams(void)
{
    return &GSWAT.esco_params;
}
/****************************************************************************
NAME    
    sinkSwatGetEscoTxBandwidth
    
DESCRIPTION
    Helper function to get SubWoofer Tx parameters.
RETURNS
    uint32
*/
uint32 sinkSwatGetEscoTxBandwidth(void)
{
    return GSWAT.esco_params.tx_bandwidth;
}
/****************************************************************************
NAME    
    sinkSwatGetEscoRxBandwidth
    
DESCRIPTION
    Helper function to get SubWoofer Rx parameters.
RETURNS
    uint32
*/
uint32 sinkSwatGetEscoRxBandwidth(void)
{
    return GSWAT.esco_params.rx_bandwidth;
}

/****************************************************************************
NAME    
    sinkSwatGetEscoMaxLatency
    
DESCRIPTION
    Helper function to get SubWoofer esco max latency.
RETURNS
    uint16
*/
uint16 sinkSwatGetEscoMaxLatency(void)
{
    return GSWAT.esco_params.max_latency;
}
/****************************************************************************
NAME    
    sinkSwatGetEscoVoiceSettings
    
DESCRIPTION
    Helper function to get SubWoofer esco voice settings.
RETURNS
    uint16
*/
uint16 sinkSwatGetEscoVoiceSettings(void)
{
    return GSWAT.esco_params.voice_settings;
}

/****************************************************************************
NAME    
    sinkSwatGetBdaddr
    
DESCRIPTION
    Helper function to get SubWoofer Bd Addr.
RETURNS
    bdaddr
*/
bdaddr* sinkSwatGetBdaddr(void)
{
    return &GSWAT.bd_addr;
}

/****************************************************************************
NAME    
    sinkSwatInit

DESCRIPTION
    This is init function for SubWoofer.

RETURNS
    bool
*/
bool sinkSwatInit(void)
{
    /* Read the subwoofer configuration data */
    bool result = FALSE;

    sink_swat_readonly_config_def_t *ro_config = NULL;
    sink_swat_writeable_config_def_t *rw_config = NULL;

    updateSwatVolume(0);

    /* Tidy up the Global data structure */ 
    memset(&GSWAT,0,sizeof(sink_swat_global_data_t));

    /* Retrieve the Sink SWAT ReadOnly & Writeable configuration and update it in SWAT Global data structure. */
    if (configManagerGetReadOnlyConfig(SINK_SWAT_READONLY_CONFIG_BLK_ID, (const void**)&ro_config))
    {
        GSWAT.esco_params.tx_bandwidth = (((uint32)ro_config->esco_params.tx_bandwidth_high << 16) | ((uint32) (ro_config->esco_params.tx_bandwidth_low) & 0x0000FFFFUL));
        GSWAT.esco_params.rx_bandwidth = (((uint32)ro_config->esco_params.rx_bandwidth_high << 16) | ((uint32) (ro_config->esco_params.rx_bandwidth_low) & 0x0000FFFFUL));
        GSWAT.esco_params.max_latency = ro_config->esco_params.swat_max_latency;
        GSWAT.esco_params.voice_settings = ro_config->esco_params.voice_settings;
        GSWAT.esco_params.retx_effort = ro_config->esco_params.retx_effort;
        GSWAT.esco_params.packet_type = ro_config->esco_params.sync_packet_type;
        
        SWAT_DEBUG(("CONF: eSCO[%lu, %lu, %x, %x, %x, %x]\n",
                        (((uint32)ro_config->esco_params.tx_bandwidth_high << 16) | ((uint32) (ro_config->esco_params.tx_bandwidth_low) & 0x0000FFFFUL)),
                        (((uint32)ro_config->esco_params.rx_bandwidth_high << 16) | ((uint32) (ro_config->esco_params.rx_bandwidth_low) & 0x0000FFFFUL)),
                        ro_config->esco_params.swat_max_latency,
                        ro_config->esco_params.voice_settings,
                        ro_config->esco_params.retx_effort,
                        ro_config->esco_params.sync_packet_type ));
        configManagerReleaseConfig(SINK_SWAT_READONLY_CONFIG_BLK_ID);
        result = TRUE;
    }
    
    if (configManagerGetReadOnlyConfig(SINK_SWAT_WRITEABLE_CONFIG_BLK_ID, (const void**)&rw_config))
    {
        GSWAT.bd_addr.lap = (((uint32)rw_config->bd_addr.lap_msb << 16) | ((uint32) (rw_config->bd_addr.lap_lsb) & 0x0000FFFFUL));
        GSWAT.bd_addr.uap = rw_config->bd_addr.uap;
        GSWAT.bd_addr.nap = rw_config->bd_addr.nap;
        SWAT_DEBUG(("CONF: Subwoofer addr[%04x %02x %04x%04x]\n", 
                            rw_config->bd_addr.nap, rw_config->bd_addr.uap, rw_config->bd_addr.lap_msb, rw_config->bd_addr.lap_lsb));
        configManagerReleaseConfig(SINK_SWAT_WRITEABLE_CONFIG_BLK_ID);
        result = TRUE;
    }
    else
    {
        result = FALSE;
    }

    return result;
}

/****************************************************************************
Prototypes for helper functions
*/


/****************************************************************************
NAME    
    sortSubwooferInquiryResults
    
DESCRIPTION
    Helper function to sort the inquiry results once inquiry search has
    completed.
*/
static void sortSubwooferInquiryResults(void);


/****************************************************************************
NAME    
    subwooferStartInqConnection
    
DESCRIPTION
    Helper function to start the connection process to the Inquiry results
*/
static void subwooferStartInqConnection(void);


/****************************************************************************
NAME    
    subwooferInqNextConnection
    
DESCRIPTION
    Helper function to connect to the next result in the list
*/
static void subwooferInqNextConnection(void);


/****************************************************************************
NAME    
    handleSwatSignallingConnectInd
    
DESCRIPTION
    Helper function to handle when a SWAT connection request is made by a
    remote device
*/
static void handleSwatSignallingConnectInd(SWAT_SIGNALLING_CONNECT_IND_T * ind);

        
/****************************************************************************
NAME    
    handleSwatSignallingConnectCfm
    
DESCRIPTION
    Helper function to handle when a SWAT connection attempt has completed
*/
static void handleSwatSignallingConnectCfm(SWAT_SIGNALLING_CONNECT_CFM_T * cfm);


/****************************************************************************
NAME    
    handleSwatMediaOpenCfm
    
DESCRIPTION
    Helper function to handle when a SWAT media connection attempt has 
    completed
*/
static void handleSwatMediaOpenCfm(SWAT_MEDIA_OPEN_CFM_T * cfm);


/****************************************************************************
NAME    
    handleSwatMediaCloseCfm
    
DESCRIPTION
    Helper function to handle when a SWAT media close has completed
*/
static void handleSwatMediaCloseCfm(SWAT_MEDIA_CLOSE_CFM_T * cfm);


/****************************************************************************
NAME    
    handleSwatMediaStartCfm
    
DESCRIPTION
    Helper function to handle when a SWAT media START has completed
*/
static void handleSwatMediaStartCfm(SWAT_MEDIA_START_CFM_T * cfm);


/****************************************************************************
NAME    
    handleSwatMediaSuspendCfm
    
DESCRIPTION
    Helper function to handle when a SWAT media SUSPEND has completed
*/
static void handleSwatMediaSuspendCfm(SWAT_MEDIA_SUSPEND_CFM_T * cfm);


/****************************************************************************
NAME    
    handleSwatSetVolumeCfm
    
DESCRIPTION
    Helper function to handle when a SWAT volume message has confirmed a
    volume change request or handle when the remote device has changed the
    SWAT volume.
*/
static void handleSwatSetVolumeCfm(SWAT_SET_VOLUME_CFM_T * cfm);


/****************************************************************************
NAME    
    handleSwatSignallingDisconnectCfm
    
DESCRIPTION
    Helper function to handle when a SWAT signalling channel has disconnected
*/
static void handleSwatSignallingDisconnectCfm(SWAT_SIGNALLING_DISCONNECT_CFM_T * message);


/****************************************************************************
NAME    
    handleSwatSampleRateCfm
    
DESCRIPTION
    Helper function to handle when a remote device responds to a sample rate
    command
*/
static void handleSwatSampleRateCfm(SWAT_SAMPLE_RATE_CFM_T * message);


/*************************************************************************/
void handleSwatMessage(Task task, MessageId id, Message message)
{
    switch(id)
    {
        case SWAT_INIT_CFM:
        {
            SWAT_DEBUG(("SWAT_INIT_CFM\n"));
            
            /* Init the SWAT volume */
            sinkSwatSetVolume(0xFF);
            
            /* Init the subwoofer trim volume */
            sinkSwatSetSubtrimIdx(DEFAULT_SUB_TRIM_INDEX);
            sinkSwatSetSubtrim(swat_sub_trim_table[sinkSwatGetSubtrimIdx()]);
        }
        break;
        case SWAT_SIGNALLING_CONNECT_IND:
        {
            handleSwatSignallingConnectInd( (SWAT_SIGNALLING_CONNECT_IND_T *)message );
        }
        break;
        case SWAT_SIGNALLING_CONNECT_CFM:
        {
            handleSwatSignallingConnectCfm( (SWAT_SIGNALLING_CONNECT_CFM_T *)message );
        }
        break;
        case SWAT_SET_VOLUME_CFM:
        {
            handleSwatSetVolumeCfm( (SWAT_SET_VOLUME_CFM_T *)message );
        }
        break;
        case SWAT_MEDIA_OPEN_CFM:
        {
            handleSwatMediaOpenCfm( (SWAT_MEDIA_OPEN_CFM_T *)message );
        }
        break;
        case SWAT_MEDIA_START_IND:
        {
            SWAT_DEBUG(("SW : SWAT_MEDIA_START_IND\n"));
        }
        break;
        case SWAT_MEDIA_START_CFM:
        {
            handleSwatMediaStartCfm( (SWAT_MEDIA_START_CFM_T *)message );
        }
        break;
        case SWAT_MEDIA_SUSPEND_IND:
        {
            SWAT_DEBUG(("SW : SWAT_MEDIA_SUSPEND_IND\n"));
        }
        break;
        case SWAT_MEDIA_SUSPEND_CFM:
        {
            handleSwatMediaSuspendCfm( (SWAT_MEDIA_SUSPEND_CFM_T *)message );
        }
        break;
        case SWAT_MEDIA_CLOSE_IND:
        {
            SWAT_DEBUG(("SW : SWAT_MEDIA_CLOSE_IND\n"));
        }
        break;
        case SWAT_MEDIA_CLOSE_CFM:
        {
            handleSwatMediaCloseCfm( (SWAT_MEDIA_CLOSE_CFM_T *)message );
        }
        break;
        case SWAT_SIGNALLING_DISCONNECT_CFM:
        {
            handleSwatSignallingDisconnectCfm( (SWAT_SIGNALLING_DISCONNECT_CFM_T *)message );
        }
        break;
        case SWAT_SAMPLE_RATE_CFM:
        {
            handleSwatSampleRateCfm( (SWAT_SAMPLE_RATE_CFM_T *)message );
        }
        break;
        default:
        {
            SWAT_DEBUG(("SW : Unhandled SWAT message ID[%x]\n",id));
        }
    }
}


/*************************************************************************/
void handleSubwooferGetAuthDevice(const CL_SM_GET_AUTH_DEVICE_CFM_T * cfm)
{
    /* If the subwoofer device bdaddr matches that of the PDL entry, Subwoofer is only device in PDL */
    if ( (sinkSwatCheckPairing()) && (cfm->status == success) )
    {
        if (BdaddrIsSame(&cfm->bd_addr, sinkSwatGetBdaddr()))
        {
            SWAT_DEBUG(("SW : Subwoofer is only device in PDL\n"));
            MessageSend(&theSink.task, EventSysEnterPairingEmptyPDL, 0);
        }
    }
}


/*************************************************************************/
void handleEventUsrSubwooferStartInquiry(void)
{
    sink_link_type profile = sink_swat;
    typed_bdaddr dev_addr;

    if (sinkInquiryGetInquiryState() == inquiry_searching)
    {
    /*  First stop the ongoing RSSI inquiry, then start subwoofer inquiry if needed.*/
        inquiryStop();
    }
    
    /* If there is already a subwoofer device in the PDL do not proceed with a new inquiry.
     * User must first delete the existing subwoofer since we only allow one!*/
    if(deviceManagerGetAddrFromProfile(profile, &dev_addr))
    {
    	SWAT_DEBUG(("SW : In order for SW Inquiry to start, first delete existing SW from PDL!\n"));
    }
    else
    {
        /* Setup the inquiry state to indicate Sink is inquiring for a subwoofer device */
        sinkInquirySetInquiryAction(rssi_subwoofer);
        sinkInquirySetInquiryState(inquiry_searching);

        /* ensure AG not able to pair/connect whilst trying to inquire for a subwoofer device */
        sinkDisableConnectable();
        sinkDisableDiscoverable();
        
        /* Allocate memory to store the inquiry results */
        sinkInquiryAllocInquiryResults(SW_MAX_INQUIRY_DEVS);

        /* Inquire for devices with device class matching SW_CLASS_OF_DEVICE */
        ConnectionWriteInquiryMode(&theSink.task, inquiry_mode_eir);
        ConnectionInquire(&theSink.task, INQUIRY_LAP, SW_MAX_INQUIRY_DEVS, SW_MAX_INQUIRY_TIME, SUBWOOFER_CLASS_OF_DEVICE);

        SWAT_DEBUG(("SW : SW inquiry started\n"));
    }
    
}


/*************************************************************************/
void handleSubwooferInquiryResult( CL_DM_INQUIRE_RESULT_T* result )
{
    uint8 counter;
    uint8 inquiry_attempts;
    inquiry_result_t *inqResult = sinkinquiryGetInquiryResults();

    SWAT_DEBUG(("SW : SW inquiry result status = %x\n",result->status));

    /* Is the search complete? */
    if (result->status == inquiry_status_ready)
    {
        SWAT_DEBUG(("SW : SW inquiry result status READY\n"));

        /* once scan attempt is now complete, update attempt counter */
        inquiry_attempts = sinkSwatGetInquiryAttempts();
        if(inquiry_attempts) 
           sinkSwatSetInquiryAttempts(inquiry_attempts - 1);

        /* Sort the list */
        sortSubwooferInquiryResults();
        
        /* Make connection attempts to the inquiry results */
        subwooferStartInqConnection();
    }
    else
    {
        SWAT_DEBUG(("SW : SW inquiry result status OTHER %04x,%02x,%06lx Level [%d]\n", result->bd_addr.nap, result->bd_addr.uap, result->bd_addr.lap, result->rssi));
        
        /* Expect SW_MAX_INQUIRY_DEV number of unique inquiry results; process each one */
        for (counter=0; counter<SW_MAX_INQUIRY_DEVS; counter++)
        {           
            /* If the discovered device is not of a Subwoofer type device class, ignore it */
            if(result->dev_class != SUBWOOFER_CLASS_OF_DEVICE)
            {
                continue;
            }
            /* If the discovered device already exists in the list, ignore it */
            if(BdaddrIsSame(&result->bd_addr, (const bdaddr *)&inqResult[counter]))
            {           
              return;
            }    
            
            /* If the current entry is empty, validate and add the device */
            if (BdaddrIsZero((const bdaddr *)&inqResult[counter]))
            {
                /* Check the RSSI value meets minimum required (if configured) */
                if (sinkSwatIsEnabledRSSIpairing())
                {
                    if (result->rssi > (int16)sinkInquiryGetRssiThreshold())
                    {
                        /* Add the device (RSSI meets threshold requirements */
                        SWAT_DEBUG(("SW : INQ Found SW device\n"));
                        sinkInquiryAddDeviceBdaddr(counter, result->bd_addr);
                        sinkInquiryAddDeviceRssi(counter, result->rssi);
                        /* once device has been added exit */        
                        return;
                    }
                    else
                    {
                        SWAT_DEBUG(("SW : INQ - Device signal too low %04x,%02x,%06lx Level [%d]\n", result->bd_addr.nap, result->bd_addr.uap, result->bd_addr.lap, result->rssi));
                    }
                }
                else
                {
                    /* Add the device (RSSI restriction is disabled) */
                    SWAT_DEBUG(("SW : INQ Found SW device\n"));
                    sinkInquiryAddDeviceBdaddr(counter, result->bd_addr);
                    sinkInquiryAddDeviceRssi(counter, result->rssi);

                    /* once device has been added exit */        
                    return;
                }
            }
        }
    }
}


/*************************************************************************/
void handleEventSysSubwooferCheckPairing(void)
{
    /* Is there a paired subwoofer device? If not, kick off a subwoofer inquiry search to try and discover one */
    if ( BdaddrIsZero(sinkSwatGetBdaddr()) )
    {
        SWAT_DEBUG(("SW : No paired SW - Starting SW inquiry search\n"));
        
        /* set the number of times to try an inquiry scan */            
         sinkSwatSetInquiryAttempts(SW_INQUIRY_ATTEMPTS);

        /* Reset inquiry status after powerOn */
        sinkInquirySetInquiryState(inquiry_idle);
        
        /* inquiry starting, make soundbar non connectable until completed scan */
        sinkDisableConnectable();
        
        /* start inquiry for sub woofer devices */
        MessageSend(&theSink.task, EventUsrSubwooferStartInquiry, 0);
    }
    else
    {
        SWAT_DEBUG(("SW : Wait for paired sub to connect\n"));
    }
}


/*************************************************************************/
void handleEventSysSubwooferOpenLLMedia(void)
{
    /* Ensure there is a signalling channel before sending media open req */
    if (SwatGetSignallingSink(sinkSwatGetDevId()))
    {
        if (SwatGetMediaSink(sinkSwatGetDevId()))
        {
            if (SwatGetMediaType(sinkSwatGetDevId()) == SWAT_MEDIA_STANDARD)
            {
                SWAT_DEBUG(("SW : Close SWAT_MEDIA_STANDARD & \n"));
                
                /* close STD latency media connection */
                handleEventSysSubwooferCloseMedia();

                SWAT_DEBUG(("SW : open SWAT_MEDIA_LOW_LATENCY\n"));
                
                /* open a low latency media connection */
                SwatMediaOpenRequest(sinkSwatGetDevId(), SWAT_MEDIA_LOW_LATENCY);
            }
            else
            {
                SWAT_DEBUG(("Low Latency MEDIA already open\n"));
            }
        }
        /* don't attempt to open if already in the processing of open a media connection */
        else if(SwatGetMediaLLState((sinkSwatGetDevId())) != swat_media_opening)
        {
            /* No media open to the subwoofer so can just open a low latency channel */
            SWAT_DEBUG(("SW : Open SWAT_MEDIA_LOW_LATENCY, media state [%d]\n",SwatGetMediaState((sinkSwatGetDevId()))));
            SwatMediaOpenRequest(sinkSwatGetDevId(), SWAT_MEDIA_LOW_LATENCY);
        }
        else
        {
            SWAT_DEBUG(("SW : Open SWAT_MEDIA_LOW_LATENCY - already in process of opening\n"));           
        }
    }
    else
    {
        SWAT_DEBUG(("SW : EventSysSubwooferOpenLLMedia (SW Not yet connected)\n"));
    }
}


/*************************************************************************/
void handleEventSysSubwooferOpenStdMedia(void)
{
    /* Ensure there is a signalling channel before sending media open req */
    if (SwatGetSignallingSink(sinkSwatGetDevId()))
    {
        if (SwatGetMediaSink(sinkSwatGetDevId()))
        {
            if (SwatGetMediaType(sinkSwatGetDevId()) == SWAT_MEDIA_LOW_LATENCY)
            {
                SWAT_DEBUG(("SW : Close ESCO & open L2CAP\n"));
                /* close low latency media connection */
                handleEventSysSubwooferCloseMedia();
                /* open a standard media connection */
                SwatMediaOpenRequest(sinkSwatGetDevId(), SWAT_MEDIA_STANDARD);
            }
            else
            {
                SWAT_DEBUG(("SW : L2CAP is already open\n"));
            }
        }
        /* don't attempt to open if already in the processing of open a media connection */
        else if(SwatGetMediaState((sinkSwatGetDevId())) != swat_media_opening)
        {
            /* No media open to the subwoofer so can just open a standard latency channel */
            SWAT_DEBUG(("SW : Open L2CAP\n"));
            SwatMediaOpenRequest(sinkSwatGetDevId(), SWAT_MEDIA_STANDARD);
        }
        else
        {
            SWAT_DEBUG(("SW : Open L2CAP - already in process of opening\n"));           
        }
    }
    else
    {
        SWAT_DEBUG(("SW : EventSysSubwooferOpenStdMedia (SW Not Yet Connected)\n"));
    }
}


/*************************************************************************/
void handleEventSysSubwooferCloseMedia(void)
{
    /* Ensure there is an open media channel before sending media close req */
    if (SwatGetMediaSink(sinkSwatGetDevId()))
    {
        SWAT_DEBUG(("SW : EventSysSubwooferCloseMedia dev_id = %x media_type = %d\n",sinkSwatGetDevId(), SwatGetMediaType(sinkSwatGetDevId())));
        SwatMediaCloseRequest(sinkSwatGetDevId(), SwatGetMediaType(sinkSwatGetDevId()));
    }
}


/*************************************************************************/
void handleEventSysSubwooferStartStreaming(void)
{
    /* Ensure there is an open media channel before sending START req */
    if (SwatGetMediaSink(sinkSwatGetDevId()))
    {
        /* obtain the sample rate of the plugin before starting to stream audio, it is
           important that the sample rate is sent over swat prior to starting to stream
           sub audio */
        if (!AudioGetA2DPSubwooferSampleRate())
        {
            SWAT_DEBUG(("SW : sample rate not available yet\n"));
            /* dsp plugin not ready yet, try again shortly */
            if(stateManagerGetState() != deviceLimbo)
                MessageSendLater(&theSink.task, EventSysSubwooferStartStreaming, 0, 500);
        }
        /* plugin not loaded yet, sample rate unavailable, wait for it to be available */
        else
        {
            SWAT_DEBUG(("SW : Send sample rate then start, rate is %ld\n",AudioGetA2DPSubwooferSampleRate()));

            /* send sample rate to sub */
            sendSampleRateToSub(AudioGetA2DPSubwooferSampleRate());
        
            /* start streaming */
            SwatMediaStartRequest(sinkSwatGetDevId(), SwatGetMediaType(sinkSwatGetDevId()));
        }
    }
}


/*************************************************************************/
void handleEventSysSubwooferSuspendStreaming(void)
{
    /* Ensure there is an open media channel before sending SUSPEND req */
    if (SwatGetMediaSink(sinkSwatGetDevId()))
    {
        SWAT_DEBUG(("SW : EventSysSubwooferSuspendStreaming\n"));
        SwatMediaSuspendRequest(sinkSwatGetDevId(), SwatGetMediaType(sinkSwatGetDevId()));
    }
}


/*************************************************************************/
void handleEventSysSubwooferDisconnect(void)
{
    SWAT_DEBUG(("SW : EventSysSubwooferDisconnect\n"));
    
    /* If there is a subwoofer connected, disconnect it */
    if (SwatGetSignallingSink(sinkSwatGetDevId()))
    {
        SwatSignallingDisconnectRequest(sinkSwatGetDevId());
    }
}


/*************************************************************************/
void handleEventUsrSubwooferVolumeUp(void)
{
    if (SwatGetSignallingSink(sinkSwatGetDevId()))
    {
        SWAT_DEBUG(("SW : Volume Up: TrimIdx = [%d]\n", sinkSwatGetSubtrimIdx()));
        /* Don't allow sub trim to go above maximum allowed value, otherwise increment and set the new sub trim gain */
        if (sinkSwatGetSubtrimIdx() < (NUM_SUB_TRIMS-1))
        {   
            /* increase sub trim volume level */
            sinkSwatSetSubtrimIdx(sinkSwatGetSubtrimIdx() + 1);
    
            SWAT_DEBUG(("SW : Volume Up: Update: TrimIdx = [%d]\n",sinkSwatGetSubtrimIdx()));

            /* update the new sub trim value */
            SwatSetVolume(sinkSwatGetDevId(), sinkSwatGetVolume(), swat_sub_trim_table[sinkSwatGetSubtrimIdx()]);
        }
    }
}


/*************************************************************************/
void handleEventUsrSubwooferVolumeDown(void)
{
    if (SwatGetSignallingSink(sinkSwatGetDevId()))
    {
        SWAT_DEBUG(("SW : Volume Down: TrimIdx = [%d]\n",sinkSwatGetSubtrimIdx()));
        /* Don't allow sub trim index to go below zero, otherwise decrement and set the new sub trim gain */
        if (sinkSwatGetSubtrimIdx() > 0)
        {
            /* decrease the sub trim value */
            sinkSwatSetSubtrimIdx(sinkSwatGetSubtrimIdx() - 1);

            SWAT_DEBUG(("SW : Volume Down: Update: TrimIdx = [%d]\n",sinkSwatGetSubtrimIdx()));
            
            /* Only update the sub trim, don't update the SYSTEM_VOLUME */
            SwatSetVolume(sinkSwatGetDevId(), sinkSwatGetVolume(), swat_sub_trim_table[sinkSwatGetSubtrimIdx()]);
        }
    }
}


/*************************************************************************/
void handleEventUsrSubwooferDeletePairing(void)
{
    sink_link_type profile = sink_swat;
    typed_bdaddr dev_addr;
    
    if(deviceManagerGetAddrFromProfile(profile, &dev_addr))
    {
    	/* Since subwoofer is a "protected" device we must first un-set
    	 * the protection flag.*/
    	ConnectionAuthSetPriorityDevice(&dev_addr.addr, FALSE);

    	/* Delete only the subwoofer device from the PDL */
        deleteSubwooferPairing(&dev_addr, TRUE);
    }
    else
    {
    	SWAT_DEBUG(("SW : No Subwoofer found in PDL!\n"));
    }

}

/*************************************************************************/
void handleEventUsrSubwooferDisconnect(void)
{
    /* Disconnect the subwoofer if it is connected */
    if (SwatGetSignallingSink(sinkSwatGetDevId()))
    {
        handleEventSysSubwooferDisconnect();
    }
}
/*************************************************************************/
/* set volume by message such that its order with respect to the audio plugin loading and 
   unloading is maintained */
/*************************************************************************/
void updateSwatVolume(int16 new_volume)
{
    /* send volume change change data to message handler */
    MAKE_SWAT_MESSAGE(SWAT_VOLUME_CHANGE_MSG);
    /* set new volume level */
    message->new_volume = new_volume;        
    /* send message to ensure correct place in queue */
    MessageSendConditionallyOnTask(&theSink.task, EventSysSubwooferSetVolume, message, AudioBusyPtr());
 
    SWAT_DEBUG(("SW : Update SWAT volume Msg created = %d\n", new_volume));
   
}
/*************************************************************************/
/* set volume from message handler */
/*************************************************************************/
void updateSwatVolumeNow(int16 new_volume)
{
    /* convert the volume in -(dB * 60) to swat volume scaled -(-db * 2) */
    int16 swat_volume;
    volume_group_config_t gain_utils_vol;

    /* limit check */
    if(new_volume > sinkVolumeGetMaxVolumeStep(audio_output_group_main))
    {
        new_volume = sinkVolumeGetMaxVolumeStep(audio_output_group_main);
    }

    sinkVolumeGetGroupConfig(audio_output_group_main, &gain_utils_vol);
    swat_volume = VolumeConvertStepsToDB(new_volume, &gain_utils_vol);
    swat_volume /= 30;
    swat_volume = 0-swat_volume;

    if (SwatGetSignallingSink(sinkSwatGetDevId()))
    {
        /* Only update if new_volume is different to current volume */
        if (swat_volume != (int16)sinkSwatGetVolume())
        {
            SWAT_DEBUG(("SW : Update SWAT volume from[%d] to[%d], trim[%d]\n", sinkSwatGetVolume(), swat_volume,sinkSwatGetSubtrim()));
            SwatSetVolume(sinkSwatGetDevId(), swat_volume, sinkSwatGetSubtrim());
        }
    }
    else
    {
        SWAT_DEBUG(("SW : FAILED to Update SWAT volume from[%d] to[%d]\n", sinkSwatGetVolume(), swat_volume));
        /* Update the volume so when subwoofer connects we know what volume to send it */
        sinkSwatSetVolume(swat_volume);
    }
}


/*************************************************************************/
void deleteSubwooferPairing(typed_bdaddr* dev_addr, bool delete_link_key)
{

    /* Disconnect the subwoofer if it is connected */
    if (SwatGetSignallingSink(sinkSwatGetDevId()))
    {
        handleEventSysSubwooferDisconnect();
    }

    /* Delete the subwoofer from the paired device list? */
    if (delete_link_key)
    {
        ConnectionSmDeleteAuthDevice(&dev_addr->addr);
    }

    /* Delete the subwoofers Bluetooth address from PS and clear from memory */
    sinkSwatWriteSubwooferBdaddr(NULL);

    SWAT_DEBUG(("SW : SW pairing deleted\n"));

}


/*************************************************************************/
void sendSampleRateToSub(uint16 sample_rate)
{
    if (SwatGetSignallingSink(sinkSwatGetDevId()))
    {
        SWAT_DEBUG(("SW : Send Sample rate[%u] to sub\n", sample_rate));
        SwatSendSampleRateCommand(sinkSwatGetDevId(), sample_rate);
    }
}


/****************************************************************************
Subwoofer helper functions
*/


/****************************************************************************/
static void handleSwatSignallingConnectInd(SWAT_SIGNALLING_CONNECT_IND_T * ind)
{
    /* Should the connection request be allowed? Reject connection if a sub is already connected or if in limbo state */
    if (SwatGetSignallingSink(sinkSwatGetDevId()))
    {
        SWAT_DEBUG(("SW : Reject incoming SWAT connection request\n"));
        SwatSignallingConnectResponse(ind->device_id, ind->connection_id, ind->identifier, FALSE);
    }
    else
    {
        SWAT_DEBUG(("SW : Accept incoming SWAT connection request\n"));
        SwatSignallingConnectResponse(ind->device_id, ind->connection_id, ind->identifier, TRUE);

        /* Store the subwoofers Bluetooth address to PS if not already done so */
        if((BdaddrIsZero(sinkSwatGetBdaddr()))||(!BdaddrIsSame(sinkSwatGetBdaddr(), &ind->bd_addr)))
        {
            /* update subwoofer configuration with sub woofer bluetooth adddress to connect to */
            sinkSwatWriteSubwooferBdaddr(&ind->bd_addr);
        }
    }
}


/****************************************************************************/
static void handleSwatSignallingConnectCfm(SWAT_SIGNALLING_CONNECT_CFM_T * cfm)
{
    A2dpPluginConnectParams *audio_connect_params = getAudioPluginConnectParams();
    
    /* indicate a subwoofer is not present in the system, the audio plugin can use this to determine how best
       to connect and set volume levels to give the best user experience with regards to pops and clicks */
    audio_connect_params->sub_is_available = FALSE; 

    /* Was the connection successful? */
    if (cfm->status == swat_success)
    {
        sink_attributes attributes;

        SWAT_DEBUG(("SW : SWAT_SIGNALLING_CONNECT_CFM\n"));

        /* try to obtain the Subwoofer version number */
        SwatSendGetVersionNoCommand(cfm->device_id);
                
        /* Is this connection result part of subwoofer inquiry? If so, end the subwoofer inquiry */
        if (sinkInquiryCheckInqActionSub())
        {
            /* get number of paired devices EXCLUDING the subwoofer */
            uint16 lNumDevices = (ConnectionTrustedDeviceListSize() - 1);
            
            sinkInquirySetInquiryAction(rssi_none);
            sinkInquiryReleaseAndResetInquiryResults();

            /* inquiry complete, make soundbar connectable again */
            sinkEnableConnectable();
            
            /* If subwoofer link is enabled, RSSI inquiry needs to be initiated after subwoofer inquiry has been finalised */ 
            /* Check if we want to start RSSI pairing */
            if((sinkDataGetPDLLimitforRSSIPairing())&&( lNumDevices < sinkDataGetPDLLimitforRSSIPairing()))
            {
                /* send event to enter pairing mode, that event can be used to play a tone if required */
                MessageSend(&theSink.task, EventSysEnterPairingEmptyPDL, 0);
                MessageSend(&theSink.task, EventUsrRssiPair, 0);
                
            }
            
            /* continue connecting to AG's */
            MessageSend(&theSink.task, EventUsrEstablishSLC, 0);
        }
        
        /* Store the (SWAT assigned) subwoofer device ID & the SWAT signalling sink  */
        sinkSwatSetDevId(cfm->device_id);
                
        /* recheck the audio routing to see if the sub needs to be utilised */
        audioUpdateAudioRouting();
          
        /* Set role for this connection */
        ConnectionSetRole(&theSink.task, cfm->sink, hci_role_master);            

        /* Restore the Set link supervision timeout to 1 second as this is reset after a role change */
        ConnectionSetLinkSupervisionTimeout(SwatGetSignallingSink(sinkSwatGetDevId()), SUBWOOFER_LINK_SUPERVISION_TIMEOUT);

        /* update the link policy settings for a subwoofer link */
        linkPolicyUpdateSwatLink();
        
        /* get the stored sub trim value, Use default attributes if none exist is PS */
        deviceManagerGetDefaultAttributes(&attributes, dev_type_sub);
        /* check attributes exist, if not create them */        
        if(!deviceManagerGetAttributes(&attributes, SwatGetBdaddr(sinkSwatGetDevId())))
        {
            /* Setup some default attributes for the subwoofer */
            deviceManagerStoreDefaultAttributes(SwatGetBdaddr(sinkSwatGetDevId()), dev_type_sub);
        }
        sinkSwatSetSubtrimIdx(attributes.sub.sub_trim_idx);
        sinkSwatSetSubtrim(swat_sub_trim_table[sinkSwatGetSubtrimIdx()]);
        
        /* Sync audio gains with subwoofer */
        SwatSetVolume(sinkSwatGetDevId(), sinkSwatGetVolume(), sinkSwatGetSubtrim());   
    
        /* indicate a subwoofer is present in the system, the audio plugin can use this to determine how best
           to connect and set volume levels to give the best user experience with regards to pops and clicks */
        audio_connect_params->sub_is_available = TRUE; 

    }
    /* Is the failed connection part of the subwoofer inquiry? If so, try next device*/
    else if (sinkInquiryCheckInqActionSub())
    {
        SWAT_DEBUG(("SW : Sig Connect Failure, status: %d \n",cfm->status));
        /* Connection did not succeed, try connecting next inquiry result */
        subwooferInqNextConnection();
        return;
    }
    /* Was there an L2CAP connection error? */
    else if (cfm->status == swat_l2cap_error)
    {
        SWAT_DEBUG(("SW : Signalling L2CAP connection error : try connection again\n"));
    }
    /* Did the connection "just fail"? */
    else
    {
        SWAT_DEBUG(("SW : Subwoofer connection failed [%x]\n", cfm->status));
    }
}


/****************************************************************************/
static void handleSwatSignallingDisconnectCfm(SWAT_SIGNALLING_DISCONNECT_CFM_T * cfm)
{   
    A2dpPluginConnectParams *audio_connect_params = getAudioPluginConnectParams();
    
    if ((cfm->status == swat_success) || (cfm->status == swat_disconnect_link_loss))
    {
        SWAT_DEBUG(("Subwoofer disconnected\n"));
        
        /* If media was streaming to subwoofer, update DSP app that sub has disconnected */
        AudioConfigureSubWoofer(AUDIO_SUB_WOOFER_NONE, SwatGetMediaSink(sinkSwatGetDevId()));
 
        /* Make device connectable so subwoofer can reconnect */
        sinkEnableConnectable();

        /* Update/Store the attributes in PS */
        deviceManagerUpdateAttributes(SwatGetBdaddr(sinkSwatGetDevId()), sink_swat, 0, 0);   

        /* indicate a subwoofer is not present in the system, the audio plugin can use this to determine how best
           to connect and set volume levels to give the best user experience with regards to pops and clicks */
        audio_connect_params->sub_is_available = FALSE; 
    }
    else
    {
        SWAT_DEBUG(("SW : SWAT_SIGNALLING_DISCONNECT_CFM failed STATUS[%x]\n",cfm->status));
    }
}


/****************************************************************************/
static void handleSwatMediaOpenCfm(SWAT_MEDIA_OPEN_CFM_T * cfm)
{
    if (cfm->status == swat_success)
    {
        SWAT_DEBUG(("SW : SWAT media connected\n"));
               
        /* Disable rate Matching for low latency media channels */
        if (SwatGetMediaType(sinkSwatGetDevId()) == SWAT_MEDIA_LOW_LATENCY)
        {
            Sink mediaSink = SwatGetMediaSink(sinkSwatGetDevId());
            if (mediaSink && SinkIsValid(mediaSink))
            {
                Source streamSource = StreamSourceFromSink(mediaSink);
                if (streamSource && SourceIsValid(streamSource))
                {
                    PanicFalse(SourceConfigure(streamSource, VM_SOURCE_SCO_RATEMATCH_ENABLE, 0));
                }
            }
        }
        
        /* Request to START streaming audio data to the subwoofer */
        MessageSend(&theSink.task, EventSysSubwooferStartStreaming, 0);
    }
    /* Media failed to open */
    else
    {
        SWAT_DEBUG(("SW : SWAT media TYPE[%x] failed to open [%x]\n", cfm->media_type, cfm->status));
        /* media failed to open, try again */        
        subwooferCheckConnection(TRUE);
    }
}


/****************************************************************************/
static void handleSwatMediaCloseCfm(SWAT_MEDIA_CLOSE_CFM_T * cfm)
{
    if (cfm->status == swat_success)
    {
        SWAT_DEBUG(("SW : SWAT_MEDIA_CLOSE_CFM\n"));
        
        /* Set link supervision timeout to 1 second */
        ConnectionSetLinkSupervisionTimeout(SwatGetSignallingSink(sinkSwatGetDevId()), SUBWOOFER_LINK_SUPERVISION_TIMEOUT);

        /* Unroute the audio */
        AudioConfigureSubWoofer(AUDIO_SUB_WOOFER_NONE, SwatGetMediaSink(sinkSwatGetDevId()));
    }
}


/****************************************************************************/
static void handleSwatMediaStartCfm(SWAT_MEDIA_START_CFM_T * cfm)
{
    /* If the START was successful, route the audio */
    if (cfm->status == swat_success)
    {
        /* Which media channel has started? Route appropriatley */
        if (cfm->media_type == SWAT_MEDIA_LOW_LATENCY)
        {
            SWAT_DEBUG(("SW : Connect Kalimba to ESCO\n"));
            if(!AudioConfigureSubWoofer(AUDIO_SUB_WOOFER_ESCO, SwatGetMediaSink(sinkSwatGetDevId())))
                SWAT_DEBUG(("ACSW FAIL\n"));
        }   
        else if (cfm->media_type == SWAT_MEDIA_STANDARD)
        {
            SWAT_DEBUG(("SW : Connect Kalimba to L2CAP\n"));
            if(!AudioConfigureSubWoofer(AUDIO_SUB_WOOFER_L2CAP, SwatGetMediaSink(sinkSwatGetDevId())))
                SWAT_DEBUG(("ACSW FAIL\n"));
        }
        else
        {
            /* Should never get here, but handle by not routing any audio */
            SWAT_DEBUG(("SW : SWAT_MEDIA_START_CFM ERROR : media TYPE[%x]\n", cfm->media_type));
            AudioConfigureSubWoofer(AUDIO_SUB_WOOFER_NONE, SwatGetMediaSink(sinkSwatGetDevId()));
        }
    }
    /* failed to start subwoofer audio, try again */
    else
    {
        SWAT_DEBUG(("SW : handleSwatMediaStartCfm FAILED = %d\n",cfm->status));
        
        /* try to start streaming again in half a second */
        MessageSendLater(&theSink.task, EventSysSubwooferStartStreaming, 0, 500);

    }
}


/****************************************************************************/
static void handleSwatMediaSuspendCfm(SWAT_MEDIA_SUSPEND_CFM_T * cfm)
{
    if (cfm->status == swat_success)
    {
        SWAT_DEBUG(("SW : Stop forwarding audio to subwoofer\n"));
        AudioConfigureSubWoofer(AUDIO_SUB_WOOFER_NONE, SwatGetMediaSink(sinkSwatGetDevId()));    
    }
}


/****************************************************************************/
static void handleSwatSetVolumeCfm(SWAT_SET_VOLUME_CFM_T * cfm)
{
    SWAT_DEBUG(("SW : SWAT_VOLUME[%d] SUB_TRIM[%d] status[%x]\n", cfm->volume, cfm->sub_trim, cfm->status));
    
    /* ensure the volume set command was accepted successfully otherwise it is necessary to 
       try again later */
    if(cfm->status == swat_success)
    {
        /* Store the synchronised volume (dB level NOT index) */
        sinkSwatSetVolume(cfm->volume);
    
        /* Was the subwoofer trim volume modified? */
        if (sinkSwatGetSubtrim() != cfm->sub_trim)
        {
            /* Store the new subwoofer trim gain */
            sinkSwatSetSubtrim(cfm->sub_trim);
            
            /* Update/Store the attributes in PS */
            deviceManagerUpdateAttributes(SwatGetBdaddr(sinkSwatGetDevId()), sink_swat, 0, 0);   
        }        
    }
}


/****************************************************************************/
static void sortSubwooferInquiryResults(void)
{
        uint8 counter, sub_counter;
        inquiry_result_t temp;
        inquiry_result_t *inqResult = sinkinquiryGetInquiryResults();
        
        /* Sort the list based on RSSI */
        for (counter=0; counter<SW_MAX_INQUIRY_DEVS; counter++)
        {
            /* Check there's actually an item in the list to compare */
            if (!BdaddrIsZero((const bdaddr *)&inqResult[counter].bd_addr))
            {
                /* Compare current result against other results and change position if necessary */
                for (sub_counter=0; sub_counter<SW_MAX_INQUIRY_DEVS; sub_counter++)
                {
                    /* Only compare against real items in the list */
                    if (BdaddrIsZero((const bdaddr *)&inqResult[sub_counter].bd_addr) == FALSE)
                    {
                        /* Don't compare result against itself */
                        if ((inqResult[counter].rssi > inqResult[sub_counter].rssi) && (sub_counter != counter))
                        {
                            /* Current result has a lesser RSSI with compared result so swap their positions in list */
                            temp = inqResult[counter];
                            inqResult[counter] = inqResult[sub_counter];
                            inqResult[sub_counter] = temp;
                        }
                    }
                    else
                    {
                        /* No other results to compare against */
                        break;
                    }
                }
            }
            else
            {
                /* No more results to sort */
                break;
            }
        }
#ifdef DEBUG_SWAT
        for (counter=0; counter<SW_MAX_INQUIRY_DEVS; counter++)
        {
            SWAT_DEBUG(("SW : theSink.inquiry.results[%d] = ADDR[%04x %02x %06lx] RSSI[%d]\n", counter, inqResult[counter].bd_addr.nap, inqResult[counter].bd_addr.uap, inqResult[counter].bd_addr.lap, inqResult[counter].rssi));
        }
#endif
}


/****************************************************************************/
static void subwooferStartInqConnection(void)
{
    inquiry_result_t *inqResult = sinkinquiryGetInquiryResults();
    /* Check a subwoofer device was found by the inquiry search */
    if (BdaddrIsZero((const bdaddr *)&inqResult[0].bd_addr))
    {
        SWAT_DEBUG(("No subwoofer device found by inquiry\n"));
        sinkInquirySetInquiryAction(rssi_none);
        sinkInquiryReleaseAndResetInquiryResults();

        MessageCancelFirst(&theSink.task, EventUsrSubwooferStartInquiry);
        /* are there any more scan attempts available? */            
        if(sinkSwatGetInquiryAttempts()) 
        {
            /* try another sub woofer inquiry search at a later time in case sub
               wasn't available at this time */
            MessageSendLater(&theSink.task, EventUsrSubwooferStartInquiry, 0, SW_INQUIRY_RETRY_TIME);
        }
        /* inquiry now complete, continue with connecting to AG's */
        else
        {
            uint16 lNumDevices =  ConnectionTrustedDeviceListSize();
            
            /* inquiry complete, make soundbar connectable again */
            sinkEnableConnectable();
            
            /* If subwoofer link is enabled, RSSI inquiry needs to be initiated after subwoofer inquiry has been finalised */ 
            /* Check if we want to start RSSI pairing */
            if((sinkDataGetPDLLimitforRSSIPairing())&&( lNumDevices < sinkDataGetPDLLimitforRSSIPairing()))
            {
                /* send event to enter pairing mode, that event can be used to play a tone if required */
                MessageSend(&theSink.task, EventSysEnterPairingEmptyPDL, 0);
                MessageSend(&theSink.task, EventUsrRssiPair, 0);
                
            }
            
            /* now attempt to connect AG's */
            MessageSend(&theSink.task, EventUsrEstablishSLC, 0);
        }
    }
    else
    {
        /* Attempt to connect to the first result */
        sinkInquiryResetInquiryAttempt();
        SwatSignallingConnectRequest(&inqResult[0].bd_addr);
    }
}


/****************************************************************************/
static void subwooferInqNextConnection(void)
{
    inquiry_result_t *inqResult = sinkinquiryGetInquiryResults();
    sinkInquirySetInquiryAttempt(sinkInquiryGetConnAttemptingIndex() + 1);
    
    /* Check there is another result to make a connection request to */
    if ((sinkInquiryGetConnAttemptingIndex() >= SW_MAX_INQUIRY_DEVS)||
        (BdaddrIsZero((const bdaddr*)&inqResult[sinkInquiryGetConnAttemptingIndex()].bd_addr)))
    {
        SWAT_DEBUG(("No more subwoofer devices found by inquiry\n"));
        sinkInquirySetInquiryAction(rssi_none);
        sinkInquiryReleaseAndResetInquiryResults();

        /* are there any more scan attempts available? */            
        if(sinkSwatGetInquiryAttempts()) 
        {
            /* try another sub woofer inquiry search at a later time in case sub
               wasn't available at this time */
            MessageSendLater(&theSink.task, EventUsrSubwooferStartInquiry, 0, SW_INQUIRY_RETRY_TIME);
        }
        /* inquiry now complete, continue with connecting to AG's */
        else
        {
            uint16 lNumDevices = ConnectionTrustedDeviceListSize();
            
            /* inquiry complete, make soundbar connectable again */
            sinkEnableConnectable();
            
            /* If subwoofer link is enabled, RSSI inquiry needs to be initiated after subwoofer inquiry has been finalised */ 
            /* Check if we want to start RSSI pairing */
            if((sinkDataGetPDLLimitforRSSIPairing())&&( lNumDevices < sinkDataGetPDLLimitforRSSIPairing()))
            {
                /* send event to enter pairing mode, that event can be used to play a tone if required */
                MessageSend(&theSink.task, EventSysEnterPairingEmptyPDL, 0);
                MessageSend(&theSink.task, EventUsrRssiPair, 0);
                
            }
            
            MessageSend(&theSink.task, EventUsrEstablishSLC, 0);
        }
    }
    else
    {
        /* Attempt to connect to the result */
        SwatSignallingConnectRequest(&inqResult[sinkInquiryGetConnAttemptingIndex()].bd_addr);
    }
}

/****************************************************************************/
static void handleSwatSampleRateCfm(SWAT_SAMPLE_RATE_CFM_T * cfm)
{
    SWAT_DEBUG(("SWAT_SAMPLE_RATE_CFM\n"));
    
    /* ensure the sample rate commands was accepted by the subwoofer */
    if(cfm->status != swat_success)
    {
           /* resend sample rate to sub */
           sendSampleRateToSub(AudioGetA2DPSubwooferSampleRate());        
    }
}

/*************************************************************************
NAME    
    sendMuteToSubwoofer
    
DESCRIPTION
    Function to set the subwoofer to mute state to reduce pops when changing sources
*/
void sendMuteToSubwoofer(void)
{
    /* If the subwoofer is connected, send digital MUTE */
    if (SwatGetSignallingSink(sinkSwatGetDevId()))
    {
        SWAT_DEBUG(("SW : Send MUTE to sub\n"));
        SwatSetVolume(sinkSwatGetDevId(), 0xFF, swat_sub_trim_table[sinkSwatGetSubtrimIdx()]);
    }
}

/****************************************************************************
NAME
    subwooferCheckConnection

DESCRIPTION
	check the sub woofer connection and streaming status and reconnect/start streaming
    if necessary

RETURNS
    None
*/
void subwooferCheckConnection(bool okToConnectSub)
{
    /* If subwoofer is connected but media is closed and not opening, open a media channel (based on the audio source) */
    if ((okToConnectSub)&&(SwatGetSignallingSink(sinkSwatGetDevId())) &&
        (SwatGetMediaState((sinkSwatGetDevId())) != swat_media_opening)&&
        (SwatGetMediaLLState((sinkSwatGetDevId())) != swat_media_opening))
    {
       /* Open the SWAT media channel */
       switch(subwooferGetLatencyBasedOnSource(sinkAudioGetRoutedAudioSource()))
       {
          case subwoofer_latency_low:
          {
              SWAT_DEBUG(("AUD [SW] : Connect eSCO media\n"));
              MessageCancelAll(&theSink.task, EventSysSubwooferOpenLLMedia);
              MessageCancelAll(&theSink.task, EventSysSubwooferOpenStdMedia);
              MessageSend(&theSink.task, EventSysSubwooferOpenLLMedia, 0);
          }
          break;
          case subwoofer_latency_standard:
          {
              SWAT_DEBUG(("AUD [SW] : Connect L2CAP media\n"));
              MessageCancelAll(&theSink.task, EventSysSubwooferOpenLLMedia);
              MessageCancelAll(&theSink.task, EventSysSubwooferOpenStdMedia);
              MessageSend(&theSink.task, EventSysSubwooferOpenStdMedia, 0);
          }
          break;
          case subwoofer_latency_not_available:
          default:
              SWAT_DEBUG(("AUD [SW] : CheckSub No Source Defined\n"));
          break;
      }
   }

    SWAT_DEBUG(("AUD [SW] : CheckSub Sink [%p] MediaType [%x] State [%x]\n",
                (void *) SwatGetSignallingSink(sinkSwatGetDevId()),
                SwatGetMediaType(sinkSwatGetDevId()), 
                SwatGetMediaState(sinkSwatGetDevId()) ));
}



subwoofer_latency_type_t subwooferGetLatencyBasedOnSource(audio_sources source)
{
    switch (source)
    {
        case audio_source_FM:
        case audio_source_ANALOG:
        case audio_source_USB:
        case audio_source_I2S:
        case audio_source_SPDIF:
            return subwoofer_latency_low;

        case audio_source_a2dp_1:
        case audio_source_a2dp_2:
            return subwoofer_latency_standard;

        case audio_source_none:
        case audio_source_end_of_list:
        default:
            return subwoofer_latency_not_available;

    }

    return subwoofer_latency_not_available;
}

void subwooferResetSubwooferLinkType(void)
{
    /* set the sub woofer link type prior to passing to audio connect */
    A2dpPluginConnectParams *audio_connect_params = getAudioPluginConnectParams();
    audio_connect_params->sub_woofer_type  = AUDIO_SUB_WOOFER_NONE;  
    audio_connect_params->sub_sink  = NULL;  
}

/****************************************************************************
NAME 
      sinkSwatWriteSubwooferBdaddr

DESCRIPTION
    Writes the Subwoofer BDADDR to Subwoofer config Block

RETURNS
    True if data was written to they PSKEY
    
NOTE
    This function will not check the value of the BDADDR requested to
    be written. If the *addr pointer is NULL, this function will write
    a zero padded bdaddr to the PS store.
*/
 bool sinkSwatWriteSubwooferBdaddr( const bdaddr * addr )
{
    sink_swat_writeable_config_def_t *rw_config;

    if (configManagerGetWriteableConfig(SINK_SWAT_WRITEABLE_CONFIG_BLK_ID, (void**)&rw_config, 0))
    {
        /* Populate the Bluetooth address to be written */
        if (addr == NULL)
        {
            /* No address passed in, so zero pad the addr part of the key */
            memset(&rw_config->bd_addr, 0, sizeof(rw_config->bd_addr));
            memset(&GSWAT.bd_addr, 0, sizeof(GSWAT.bd_addr));
        }
        else
        {
            rw_config->bd_addr.lap_msb = (addr->lap>>16);
            rw_config->bd_addr.lap_lsb = (uint16) addr->lap;
            rw_config->bd_addr.nap = addr->nap;
            rw_config->bd_addr.uap = addr->uap;

            GSWAT.bd_addr.lap = addr->lap;
            GSWAT.bd_addr.uap = addr->uap;
            GSWAT.bd_addr.nap = addr->nap;
        }

        configManagerUpdateWriteableConfig(SINK_SWAT_WRITEABLE_CONFIG_BLK_ID);
        return TRUE;
    }
    else
    {
        SWAT_DEBUG(("CONF: Error writing Subwoofer config block\n"));
        return FALSE;
    }
}

/****************************************************************************
NAME    
    sinkSwatIsEnabledRSSIpairing
    
DESCRIPTION
    Helper function to check if subwoofer RSSI pairing is enabled or not to ensure
	signal strenght is higher than the configured threshold.

RETURNS
   TRUE or FALSE
*/
bool sinkSwatIsEnabledRSSIpairing(void)
{
    bool ret_val = FALSE;
    sink_swat_readonly_config_def_t *ro_config;

    if (configManagerGetReadOnlyConfig(SINK_SWAT_READONLY_CONFIG_BLK_ID, (const void**)&ro_config))
    {
       ret_val = ro_config->LimitRssiSubwooferPairing;
       configManagerReleaseConfig(SINK_SWAT_READONLY_CONFIG_BLK_ID);
    }

    return ret_val;
}

#else /* ENABLE_SUBWOOFER */




#endif /* ENABLE_SUBWOOFER */
