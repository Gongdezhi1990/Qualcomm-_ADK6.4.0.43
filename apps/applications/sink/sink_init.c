/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/


/****************************************************************************
    Header files
*/
#ifdef ACTIVITY_MONITORING
#include "sink_sport_health.h"
#endif

#include "sink_private_data.h"
#include "sink_main_task.h"
#include "sink_init.h"
#include "sink_config.h"
#include "sink_statemanager.h"
#include "sink_led_manager.h"
#include "sink_tones.h"
#include "sink_dut.h"
#include "charger.h"
#include "sink_a2dp.h"
#include "sink_audio.h"
#include "sink_audio_prompts.h"
#include "sink_audio_routing.h"
#include "sink_buttons.h"
#include "sink_slc.h"
#include "sink_hfp_data.h"
#include "sink_callmanager.h"
#include "sink_display.h"
#include "sink_device_id.h"
#include "sink_powermanager.h"
#include "sink_wired.h"
#include "sink_debug.h"
#include "sink_volume.h"
#include "sink_peer.h"
#include "sink_pio_debounce.h"
#include "audio_plugin_common.h"
#include "sink_swat.h"
#include "sink_anc.h"
#include "sink_accessory.h"
#include "sink_hid.h"
#include "sink_ba.h"
#include "sink_usb.h"
#include "sink_bredr.h"
#include "sink_malloc_debug.h"
#include "sink_class_of_device.h"
#include "sink_audio_clock.h"
#include "sink_main_task.h"
#include "sink_va_if.h"
#ifdef ENABLE_BISTO
#include <sink2bisto.h>
#endif /* ENABLE_BISTO */

#ifdef ENABLE_GAIA
#include "sink_gaia.h"
#endif
#ifdef ENABLE_MAPC
#include "sink_mapc.h"
#endif
#ifdef ENABLE_PBAP
#include "sink_pbap.h"
#endif


#ifdef ENABLE_FAST_PAIR
#include "sink_fast_pair.h"
#endif

#include <ps.h>
#include <stdio.h>
#include <connection.h>
#include <source.h>
#include <hfp.h>
#include <pio.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include <vm.h>
#include <audio_plugin_voice_variants.h>
#include <audio.h>
#ifdef ENABLE_SUBWOOFER
#include <swat.h>
#endif /* ENABLE_SUBWOOFER */
#include "sink_ble.h"

#ifdef ENABLE_SPEECH_RECOGNITION
#include "sink_speech_recognition.h"
#endif

#ifdef DEBUG_INIT
#define INIT_DEBUG(x) DEBUG(x)
#else
#define INIT_DEBUG(x) 
#endif


/* PS keys */
#define PS_BDADDR           (0x001)
#define PS_DEVICE_CLASS     (0x0003)
#define PS_HFP_POWER_TABLE  (0x360)

typedef enum
{
    pskey_av_minor_headphones,
    pskey_av_minor_hifi,
    pskey_av_minor_speaker,
    pskey_av_minor_portable
} pskey_av_minor_class;

#define AV_MINOR_DEFAULT_A2DP_CLASS  AV_MINOR_HEADPHONES

/****************************************************************************
NAME    
    SetupPowerTable

DESCRIPTION
    Attempts to obtain a low power table from the Ps Key store.  If no table 
    (or an incomplete one) is found in Ps Keys then the default is used.
    
RETURNS
    void
*/
void SetupPowerTable( void )
{
    uint16 size_ps_key = 0;
    power_table *PowerTable;
    
    /* The PSKEY PS_HFP_POWER_TABLE is not supported on HYDRACORE platforms */
#ifndef HYDRACORE
    /* obtain the size of memory in words required to hold the contents of the pskey */
    size_ps_key = PsFullRetrieve(PS_HFP_POWER_TABLE, NULL, 0);
    size_ps_key = size_ps_key * sizeof(uint16);
#endif

    /* initialise user power table */
    sinkDataSetUserPowerTable(NULL);

    /* check whether any pskey data exists */    
    if (size_ps_key)
    {
        /* malloc storage for power table entries */ 
        PowerTable = (power_table*)mallocPanic(size_ps_key);

        size_ps_key = PsFullRetrieve(PS_HFP_POWER_TABLE, (uint16*)PowerTable, size_ps_key);
        size_ps_key = size_ps_key * sizeof(uint16);

        /* Having retrieved whatever happens to be in the key, 
           make sure tthat the length of the pskey data matches the 
           content of the data read */
        if(size_ps_key == (sizeof(lp_power_table)*(  PowerTable->normalEntries
                                                    + PowerTable->SCOEntries 
                                                    + PowerTable->A2DPStreamEntries
                                                    - 1)) +
                           sizeof(*PowerTable)
          )
        {   
            sinkDataSetUserPowerTable(PowerTable);

            /* pskey format is correct */
            INIT_DEBUG(("User Power Table - Norm[%x] Sco[%x] Stream[%x]\n",PowerTable->normalEntries,PowerTable->SCOEntries,PowerTable->A2DPStreamEntries));
        }
        else
        {   /* No/incorrect power table defined in Ps Keys - use default table */
            freePanic(PowerTable);
            PowerTable = NULL;
            INIT_DEBUG(("No User Power Table\n"));
        }
    }
    
}


/****************************************************************************
NAME    
    sinkHfpInit
    
DESCRIPTION
    Initialise HFP library

RETURNS
    void
*/

void sinkHfpInit( void )
{
    hfp_init_params hfp_params;
    
    memset(&hfp_params, 0, sizeof(hfp_init_params));
    
	/* initialise the hfp library with parameters read from config*/  
    sinkHfpDataInit(&hfp_params);

    /* Get features config so HFP can be initialised correctly depending on mSBC support */
    configManagerInitFeatures();
    
    sinkClearQueueudEvent(); 

    /* If the current audio plugin has no mSBC support but HFP1.7 is configured,
         disable it and enable HFP1.5 */
    if(!AudioPluginVoiceVariantsGetHfpPlugin(wide_band, sinkHfpDataGetAudioPlugin()) &&
       hfp_params.supported_profile & hfp_handsfree_107_profile)

    {
        /* remove HFP1.7 support */
        hfp_params.supported_profile &= ~hfp_handsfree_107_profile;
        /* add HFP1.5 support */
        hfp_params.supported_profile |= hfp_handsfree_profile;		
        /* ensure mSBC is removed from the supported codecs */
        hfp_params.supported_wbs_codecs &= hfp_wbs_codec_mask_cvsd;
    }

    sinkHfpDataUpdateSupportedProfile(hfp_params.supported_profile);

    /* Ensure all mandatory sco, esco, esco edr packet types are enabled as per HFP 1.7 spec */
    if(sinkHfpDataGetFeaturesAdditionalAudioParams() &&
           (hfp_params.supported_profile & hfp_handsfree_107_profile))		
    {
        sinkHfpDataUpdateSyncPktTypes();
    }
    
    /* store the link loss time in the app */
    sinkHfpDataSetLinkLossReminderTime(hfp_params.link_loss_interval);

    /* initialise hfp library with pskey read configuration */
    HfpInit(&theSink.task, &hfp_params, NULL);
    
    /* initialise the audio library, uses one malloc slot */
    AudioLibraryInit();

    if(audioRouteIsMixingOfVoiceAndAudioEnabled())
    {
        AudioSetMaximumConcurrentAudioConnections(2);
    }
}

/****************************************************************************
NAME    
    disableAutomaticMicBias
    
DESCRIPTION
    Disables automatic Mic bias

RETURNS
    void
*/
static void disableAutomaticMicBias(void)
{
    AudioPluginInitMicBiasDrive(sinkAudioGetMic1Params());
    AudioPluginInitMicBiasDrive(sinkAudioGetMic2Params());
    AudioPluginInitMicBiasDrive(sinkAudioGetMic3Params());
    AudioPluginInitMicBiasDrive(sinkAudioGetMic4Params());
    AudioPluginInitMicBiasDrive(sinkAudioGetMic5Params());
    AudioPluginInitMicBiasDrive(sinkAudioGetMic6Params());
}

/****************************************************************************
NAME    
    sinkInitComplete
    
DESCRIPTION
    Initialization specific to Sink App 

RETURNS
    void
*/
void sinkAppInit( const HFP_INIT_CFM_T *cfm )
{
    /* Make sure the profile instance initialisation succeeded. */
    if (cfm->status == hfp_init_success)
    {
        uint8 i;
        /* initialise connection status for this instance */
        for(i=0;i<MAX_MULTIPOINT_CONNECTIONS;i++)
        {
            sinkHfpDataSetProfileListIdStatus(INVALID_LIST_ID,i);
            setA2dpPdlListId(i, INVALID_LIST_ID);
        }

        /* Disable SDP security */
        ConnectionSmSetSecurityLevel(protocol_l2cap,1,ssp_secl4_l0,TRUE,FALSE,FALSE);
        
        /* WAE - no ACL, Debug keys - off, Legacy pair key missing - on */
        ConnectionSmSecModeConfig(&theSink.task, cl_sm_wae_acl_none, FALSE, TRUE);

        /* Require MITM on the MUX (incomming and outgoing)*/
        if(sinkDataIsMITMRequired())
        {
            ConnectionSmSetSecurityLevel(0,3,ssp_secl4_l3,TRUE,TRUE,FALSE);
        }

        RegisterDeviceIdServiceRecord();

#ifdef ENABLE_PEER
        peerInit();
        /* clear any low battery condition reported by the peer device */
        powerManagerSetPeerSlaveLowBat(FALSE);
#endif
        
        /* Initialise Inquiry Data to NULL */
        sinkInquiryResetInquiryResults();
        
#ifdef ENABLE_AVRCP     
        /* initialise the AVRCP library */
        sinkAvrcpInit();
#endif

#ifdef ENABLE_SUBWOOFER
        /* Initialise the swat the library - Use library default service record - Library does not auto handles messages */
        if ( (sinkSwatGetEscoTxBandwidth() == 0) &&
             (sinkSwatGetEscoRxBandwidth() == 0) &&
             (sinkSwatGetEscoMaxLatency() == 0) )

        {
            /* Use library default eSCO parameters */
            SwatInit(&theSink.task, SW_MAX_REMOTE_DEVICES, swat_role_source, FALSE, 0, 0, NULL);
        }
        else
        {
            /* Use custom eSCO parameters from configuration data */
            SwatInit(&theSink.task, SW_MAX_REMOTE_DEVICES, swat_role_source, FALSE, 0, 0, sinkSwatGetSyncConfigParams());
        }
        {
            A2dpPluginConnectParams *audio_connect_params = getAudioPluginConnectParams();

            /* flag to indicate to the A2DP decoder plugin that subwoofer functinality is compiled into 
               the VM application, this in turn forces the A2DP decoder common plugin to set its
               output rate at a fixed rate of 48KHz*/
            audio_connect_params->sub_functionality_present = TRUE;
        }
#endif

        /* No audio sources are gated at startup */
        sinkAudioReSetGatedAudio();
        
        /*if we receive the init message in the correct state*/
        if ( stateManagerGetState() == deviceLimbo )
        {                                           
#ifdef ENABLE_PBAP                      
            /* If hfp has been initialised successfully, start initialising PBAP */
            INIT_DEBUG(("INIT: PBAP Init start\n"));
            initPbap();
#else
            /*init the configurable parameters*/
            InitUserFeatures();
#endif        
        }

        /* try to get a power table entry from ps if one exists after having read the user features as
           A2DP enable state is used to determine size of power table entry */
        SetupPowerTable();
        
        
#if defined(ENABLE_MAPC) && defined(ENABLE_PEER)
        #error MAP incompatible with Peer device support (see B-177050)
#elif defined(ENABLE_MAPC)
        /* if the map feature is enabled, start the map notification service at initialisation time */
        initMap();
#endif

        /* disable automatic mic bias control as this is handled by the audio plugins */
        disableAutomaticMicBias();
    }
    else
        /* If the profile initialisation has failed then things are bad so panic. */
        Panic();
    
    /* Perform BLE Initialisation */
    sinkBleInitialiseDevice();

     sinkBroadcastAudioInitialise();
  
    /* Initialise ANC */
    sinkAncInit();

    /* Initialise HID */
    sinkHidInit();

    /* Initialise Accessory */
    sinkAccessoryInit();

    /* Initialise paging count, used to harmonise paging and inquiry */
    sinkInquiryClearPagingCount();

    /* initialisation is complete */  
    sinkDataSetSinkInitialisingStatus(FALSE);

    /* Initialize GAIA */
#ifdef ENABLE_GAIA
    initGaia();
#endif
    /* Initialize Fast Pair */
#ifdef ENABLE_FAST_PAIR
    sinkFastPairInit();
#endif
    /* Initialize VA module */
    SinkVaInit();

#ifdef ENABLE_BISTO
    BistoBtInitialiseChannels();
    /* Initialize Bisto module */
    BistoInitialise();
#endif /* ENABLE_BISTO */
}


/*************************************************************************
NAME    
    InitPreAmp
    
DESCRIPTION
    Enable the Pre Amp if configured to do so
    
    channel contains the audio channel that you want to enable the pre amp on 
    
RETURNS

*/
static void initPreAmp( audio_channel channel )
{
    Source source = (Source)NULL ;
    source = StreamAudioSource(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_0, channel);
    
    if( ((channel == AUDIO_CHANNEL_A) && (sinkAudioMicAPreAmpEnabled())) || 
        ((channel == AUDIO_CHANNEL_B)&& (sinkAudioMicBPreAmpEnabled())) )
    {
        /* Use the microphone pre-amp */
        if(!SourceConfigure(source, STREAM_CODEC_MIC_INPUT_GAIN_ENABLE,TRUE ))
        {
        }
   }
   
    /* Close the Source*/
    SourceClose(source);
}

#ifdef LOCAL_NAME_CONFIGURATION_UNSUPPORTED
/*************************************************************************
NAME    
    writeLocalName
    
DESCRIPTION
    Write a default local device name when there is no other configuration mechanism supported.
*/
static void writeLocalName(void)
{
    char new_name[] = "@@@QTIL_sinkApp@@@";

    ConnectionChangeLocalName(strlen(new_name), (uint8 *)new_name);
}
#endif

/*************************************************************************
NAME    
    InitEarlyUserFeatures
    
DESCRIPTION
    This function initialises the configureation that is required early 
    on in the start-up sequence. 

RETURNS

*/
void InitEarlyUserFeatures ( void ) 
{
    bool useButtonDebounceConfiguration;
    
    PowerChargerSuppressLed0(TRUE);


    /* Initialise PIO debounce groups */
    useButtonDebounceConfiguration = !PioConfigureDebounceGroups();
        
    /* Initialise the Button Manager */
    buttonManagerInit(useButtonDebounceConfiguration) ;

    /* Once system Managers are initialised, load up the configuration */
    configManagerInit(TRUE);   
    
    sinkAudioClockInit();
    
    /* Init wired before USB or wired audio gets routed before init */
    wiredAudioInit(); 

    /* USB init can be done once power lib initialised */
    usbInit();
    
    /* initialise the display */
    displayInit();
    
    /* initialise DUT */
    dutInit();
    
    /*configure the audio Pre amp if enabled */
    initPreAmp( AUDIO_CHANNEL_A) ;
    initPreAmp( AUDIO_CHANNEL_B) ;

    /* Enter the limbo state as we may be ON due to a charger being plugged in or SYS_CTRL has been pressed. */
    stateManagerEnterLimboState(limboWakeUp);
}

/*************************************************************************
NAME    
    InitUserFeatures
    
DESCRIPTION
    This function initialises all of the user features - this will result in a
    poweron message if a user event is configured correctly and the device will 
    complete the power on

RETURNS

*/
void InitUserFeatures ( void ) 
{
    /* Set to a known value*/
    sinkHfpDataSetVoiceRecognitionIsActive(hfp_invalid_link) ;
    BMSetButtonsLocked(FALSE);
    sinkHfpDataSetLastOutgoingAg(hfp_primary_link);


    #ifdef ENABLE_SPEECH_RECOGNITION
    speechRecSetIsActive(FALSE);
    #endif

    if (sinkVolumeIsVolButtonInverted())
    {
        MessageSend ( &theSink.task , EventUsrVolumeOrientationInvert , 0 ) ;
    }
    
    /* set the LED enable disable state which now persists over a reset */
    if (LedManagerIsEnabled())
    {
        LedManagerEnableLEDS () ;
    }
    else
    {
        LedManagerDisableLEDS () ;
    }

    /* Set inquiry tx power and RSSI inquiry mode */
    ConnectionWriteInquiryTx((int8)sinkInquiryGetInquiryTxPower());
    ConnectionWriteInquiryMode(&theSink.task, inquiry_mode_eir);   /* RSSI with EIR data */
    
    /* Check if we're here as result of a watchdog timeout */
    powerManagerCheckPanic();

    /*automatically power on the heasdet as soon as init is complete*/
    if(sinkDataGetPanicReconnect())
    {
    	INIT_DEBUG(("INIT: Power On Panic\n"));
        MessageSend( &theSink.task , EventSysPowerOnPanic , NULL ) ;
    }
    else if((sinkDataIsAutoPoweronAfterInit() && !powerManagerIsChargerConnected()))
    {
        INIT_DEBUG(("INIT: Power On\n"));
        MessageSend( &theSink.task , EventUsrPowerOn , NULL ) ;
    }
    
    /* set task to receive system messages */
    MessageSystemTask(&theSink.task);
    
#ifdef LOCAL_NAME_CONFIGURATION_UNSUPPORTED
    writeLocalName();
#endif

    INIT_DEBUG(("INIT: complete\n"));
}

static uint32 getAvMinorClass(void)
{
    uint32 class = AV_MINOR_DEFAULT_A2DP_CLASS;
    switch(sinkA2dpGetDeviceClass())
    {
        case pskey_av_minor_speaker:
            class = (AV_MINOR_SPEAKER);
            break;
        case pskey_av_minor_portable:
            class = (AV_MINOR_PORTABLE);
            break;
        case pskey_av_minor_hifi:
            class = (AV_MINOR_HIFI);
            break;
        case pskey_av_minor_headphones:
            class = (AV_MINOR_HEADPHONES);
            break;
        default:
            class = (AV_MINOR_DEFAULT_A2DP_CLASS);
            break;
    }
    return class;
}

uint32 sinkInitGetDeviceClassConfig(void)
{
    uint32 device_class = 0 ;

    PsFullRetrieve(PS_DEVICE_CLASS, (uint16*)&device_class , sizeof(uint32));

    if(device_class == 0)
    {
        device_class = (AUDIO_MAJOR_SERV_CLASS | AV_MAJOR_DEVICE_CLASS);

        if(sinkA2dpEnabled())
        {
            device_class |= (AV_COD_RENDER | getAvMinorClass());
        }
        else
        {
            device_class |= (AV_MINOR_HEADSET);
        }
    }
    INIT_DEBUG(("INIT: DeviceClass:0x%ld\n",device_class));
    return device_class;
}

void sinkInitConfigureDeviceClass(void)
{
    uint32 device_class = sinkInitGetDeviceClassConfig();
    ConnectionWriteClassOfDevice(device_class);
}
