/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    Configuration manager for the device - resoponsible for extracting user information out of the 
    PSKEYs and initialising the configurable nature of the sink device components
    
*/
#include "sink_configmanager.h"
#include "sink_config.h"
#include "sink_buttonmanager.h"
#include "sink_led_manager.h"
#include "sink_statemanager.h"
#include "sink_powermanager.h"
#include "sink_volume.h"
#include "sink_devicemanager.h"
#include "sink_private_data.h"
#include "sink_main_task.h"
#include "sink_events.h"
#include "sink_tones.h"
#include "sink_audio_prompts.h"
#include "sink_audio.h"
#include "sink_debug.h"
#include "sink_pio.h"
#include "sink_auth.h"
#include "sink_device_id.h"
#include "sink_ble_gap.h"
#include "sink_leds.h"
#include "sink_multi_channel.h"
#include "sink_hfp_data.h"
#include "sink_ba.h"
#include "sink_fm.h"
#include "sink_usb.h"
#include "sink_malloc_debug.h"
#include "sink_wired.h"
#include "sink_bredr.h"
#include <bdaddr.h>

#if (defined(GATT_HID_CLIENT) || defined(ENABLE_IR_REMOTE))
#include "sink_input_manager.h"
#endif

#ifdef ENABLE_SPEECH_RECOGNITION
#include "sink_speech_recognition.h"
#endif

#ifdef ENABLE_SUBWOOFER
#include "sink_swat.h"
#endif

#ifdef ENABLE_IR_REMOTE
#include "sink_ir_remote_control.h"
#endif

#include <csrtypes.h>
#include <ps.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <panic.h>
#include <boot.h>
#include <audio_i2s_common.h>
#include <audio_output.h>
#include <audio.h>
#include <pblock.h>
#include <config_store.h>

#ifdef DEBUG_CONFIG
#define CONF_DEBUG(x) DEBUG(x)
#else
#define CONF_DEBUG(x)
#endif

/****************************************************************************
NAME
      configManagerFeatureBlock

DESCRIPTION
      Read the system feature block and configure system accordingly

RETURNS
      void
*/
static void configManagerFeatureBlock( void )
{
    uint8 i;

#ifdef ENABLE_PEER
    ValidatePeerUseDeviceIdFeature();
#endif

    /*Set the default volume level*/
    for(i=0;i<MAX_PROFILES;i++)
    {
        sinkHfpdataSetAudioSMVolumeLevel(sinkHfpDataGetDefaultVolume(), i);
    }
    TonesInitSettings(sinkTonesIsPlayTonesAtFixedVolume(),
                      sinkTonesGetFixedToneVolumeLevel());

}

#ifdef ENABLE_SQIFVP
void configManagerSqifPartitionsInit( void )
{
    uint8 currentLang = SinkAudioPromptsGetLanguage();

    CONF_DEBUG(("CONF : Current SQIF partitions free 0x%x \n", sinkDataGetSquifFreePartitions()));

    /* Check that the currently selected languages partition is available */
    if((1<<currentLang) & sinkDataGetSquifFreePartitions())
    {
        CONF_DEBUG(("CONF : Current SQIF VP partition (%u) not valid\n", currentLang));
        AudioPromptSelectLanguage();
        /* if the new selected language is the same then none could be found so disable audio prompts */
        if (currentLang == currentLang)
        {
            SinkAudioPromptsSetEnabled(FALSE);
            CONF_DEBUG(("CONF : Disabling Audio Prompts, no valid partitions\n"));
        }
    }
    }
#endif

/****************************************************************************
NAME 
     configManagerEnableMultipoint
*/ 
void configManagerEnableMultipoint(bool enable)
{
    CONF_DEBUG(("CONF: Multipoint %s\n", enable ? "Enable" : "Disable"));

    if(sinkDataGetSessionMultiPointEnabled() && enable)
    {
        /* Check we can make HFP multi point */
        if(HfpLinkSetMaxConnections(2))
        {
            /* If A2DP disabled or we can make it multi point then we're done */
            if(!sinkA2dpEnabled() || A2dpConfigureMaxRemoteDevices(2))
            {
                CONF_DEBUG(("CONF: Success\n"));
                sinkDataSetMultipointEnabled(TRUE);
                return;
            }
        }
    }
    else
    {
        /* Check we can make HFP single point */
        if(HfpLinkSetMaxConnections(1))
        {
            /* If A2DP disabled or we can make it single point then we're done */
            if(!sinkA2dpEnabled() || A2dpConfigureMaxRemoteDevices(1))
            {
                CONF_DEBUG(("CONF: Success\n"));
                sinkDataSetMultipointEnabled(FALSE);
                return;
            }
        }
    }
    CONF_DEBUG(("CONF: Failed\n"));
    /* Setting failed, make sure HFP setting is restored */
    HfpLinkSetMaxConnections(sinkDataIsMultipointEnabled() ? 2 : 1);

}

/****************************************************************************
NAME 
     configManagerReadSessionData
*/ 
void configManagerReadSessionData( void )
{
    sinkVolumeGetSessionVolButtonInverted();
  
    /* if the feature bit to reset led enable state after a reboot is set then enable the leds
       otherwise get the led enable state from ps */
    if(!LedManagerResetLedState())
    {
        LedManagerSetLedEnabled(LedManagerGetSessionLedData());
    }
    else
    {
        LedManagerSetLedEnabled(TRUE);
    }  
    SinkAudioPromptsGetSessionData();
    powerManagerGetSessionData();

#ifdef ENABLE_SPEECH_RECOGNITION
    speechRecognitionSetSSREnabled(speechRecognitionGetSessionSSREnabled());
#endif
    sinkAudioRoutingGetSessionData();
    configManagerEnableMultipoint(TRUE);

    sinkA2dpSetA2dpDataFromSessionData();
    AudioSetSpeakerPeqBypass(sinkA2dpGetSpeakerPeqBypass());
    sinkFmGetFmSessionData();

	SinkWiredGetSessionData();
    sinkUsbGetSessionData();
}

/****************************************************************************
NAME 
     configManagerWriteSessionData

*/ 
void configManagerWriteSessionData( void )
{

    sinkVolumeSetSessionVolButtonInverted();
    LedManagerSetSessionLedData();
    SinkAudioPromptsSetSessionData();
    sinkDataSetSessionMultiPointEnabled();
    powerManagerSetSessionData();
    #ifdef ENABLE_SPEECH_RECOGNITION
    speechRecognitionSetSessionSSREnabled();
    #endif

    sinkAudioRoutingSetSessionData();

    sinkA2dpSetSessionDataFromA2dpData();
    sinkFmSetFmSessionData();

    SinkWiredSetSessionData();
    sinkUsbSetSessionData();

    PioWriteSessionData();
}


/****************************************************************************
NAME 
      configManagerRestoreDefaults

DESCRIPTION
    Restores default PSKEY settings.
    This function restores the following:
        1. CONFIG_SESSION_DATA
        2. Enable Audio Prompts with default language
        3. Clears the paired device list
        4. Enables the LEDs
        5. Disable multipoint
        6. Disable lbipm
        7. Reset EQ
        8. Clear User PIO
 
RETURNS
      void
*/ 

void configManagerRestoreDefaults( void ) 
{
    CONF_DEBUG(("CO: Restore Defaults\n")) ;
    
    /*Set local values*/
    sinkVolumeSetVolButtonInverted(FALSE);
    SinkAudioPromptsSetLanguage(0) ;
    SinkAudioPromptsSetEnabled(TRUE);
    configManagerEnableMultipoint(FALSE);
    powerManagerEnableLbipm(FALSE);
    setMusicProcessingMode(A2DP_MUSIC_PROCESSING_FULL);
	/*Call function to reset the PDL*/
	deviceManagerRemoveAllDevices();
    
#ifdef ENABLE_PEER
    /* Ensure permanently paired Peer device is placed back into PDL */
    AuthInitPermanentPairing();
#endif

    if    (!LedManagerIsEnabled())
    {
        /*Enable the LEDs*/
        MessageSend (&theSink.task , EventUsrLedsOn , 0) ;
    }
    
    PioSetUserPioState(FALSE);
}


/****************************************************************************
NAME 
     configManagerReadDspData
*/ 
static void configManagerReadDspData( void )
{
    /* Initialise DSP persistent store block */  
    /* the pblock library will allocate enough memory for 1 entry initially,
       reallocing as and when required */
    PblockInit(CONFIG_DSP_SESSION_KEY, 0);    
}


/****************************************************************************
NAME 
     configManagerWriteDspData
*/ 
void configManagerWriteDspData( void )
{
    /* Write DSP persistent store block */
    PblockStore();
}


/****************************************************************************
NAME 
      configManagerFillPs

DESCRIPTION
      Fill PS to the point defrag is required (for testing only)

RETURNS
      void
*/
void configManagerFillPs(void)
{
    uint8 defrag_key_size = sinkDataGetDefragKeySize();
    uint8 defrag_key_min= sinkDataGetDefragKeyMin();;
    CONF_DEBUG(("CONF: Fill PS Size[%d]Min[%d]",defrag_key_size, defrag_key_min)) ;

    if(sinkDataGetDefragKeySize())
    {
        uint16 count     = PsFreeCount(defrag_key_size);
        uint16* buff     = (uint16*)mallocPanic(defrag_key_size);
        CONF_DEBUG(("Count[%d]", count));
        if(count >  defrag_key_min)
        {
            for(count = count -  defrag_key_min ; count > 0; count --)
            {
                *buff = count;
                PsStore(CONFIG_DEFRAG_TEST, buff, defrag_key_size);
            }

        }
        CONF_DEBUG(("NowFree[%d]", PsFreeCount(defrag_key_size)));
    }
    CONF_DEBUG(("\n"));
}


/****************************************************************************
NAME 
      configManagerDefragCheck

DESCRIPTION
      Check if PS Defrag. is required.

RETURNS
      bool  TRUE if defrag. required
*/
static bool configManagerDefragCheck(void)
{
    CONF_DEBUG(("CONF: Defrag Check")) ;
    if(sinkDataGetDefragKeySize())
    {
        uint16 count = PsFreeCount(sinkDataGetDefragKeySize());
        CONF_DEBUG((", free [%d]", count)) ;
        if(count <=  sinkDataGetDefragKeyMin())
        {
            CONF_DEBUG((" ,defrag required\n")) ;
            return TRUE;
        }
    }
    
    CONF_DEBUG(("\n")) ;
    return FALSE;
}

/****************************************************************************
NAME 
      configManagerDefrag

DESCRIPTION
      Flood PS to force a PS Defragment operation on next reboot.

PARAMS
    reboot  TRUE if the application should automatically reboot
    
RETURNS
      void
*/
static void configManagerDefrag(const bool reboot)
{
    CONF_DEBUG(("Flooding PS and reboot\n")) ;
    PsFlood();
    
    if(reboot)
    {
        /* try to set the same boot mode; this triggers the target to reboot.*/
        BootSetMode(BootGetMode());
    }
}

/****************************************************************************
NAME 
      configManagerInitMemory

DESCRIPTION
      Init static size memory blocks that are required early in the boot sequence

RETURNS
      void
    
*/

void configManagerInitMemory( void )  
{ 
}

/****************************************************************************
NAME 
      configManagerInit

DESCRIPTION
      The Configuration Manager is responsible for reading the user configuration
      from the persistent store are setting up the system.  Each system component
      is initialised in order.  Where appropriate, each configuration parameter
      is limit checked and a default assigned if found to be out of range.

RETURNS
      void
    
*/
void configManagerInit( bool full_init )  
{     

    /* Allocate memory for voice prompts config if required*/
    SinkAudioPromptsInit(full_init);
    
    /*Read the local BD Address of the sink device */
    sinkDataInitLocalBdAddrFromPs();

    sinkTonesInit();

    if (full_init)    
    { 
        /* Must happen between features and session data... */
        InitA2dp();

        /* Initialise and configure the power management system */
        powerManagerInit();

        /* init the volume levels storage before it gets used by wired or usb inputs */
        volumeInit();

        /*Initialize global FM_DATA which is used during init read session data */
        sinkFmDataInit();

        /* Read and configure the volume orientation, LED Disable state, and tts_language */
        configManagerReadSessionData () ; 
        
        if(sinkAudioIsManualSrcSelectionEnabled())
        {
            /* set the active routed source to the last used source stored in session data */
            audioSwitchToAudioSource(sinkAudioGetRequestedAudioSource());
        }

        configManagerReadDspData();

        #if defined(ENABLE_SQIFVP)
        {
            /* Configure SQIF partitions */
            configManagerSqifPartitionsInit();
        }
        #endif

        /* Read multi-channel audio output configuration */
        SinkMultiChannelInit();

        SinkWiredReadI2SAndSpdifConfiguration();
        
        #if defined(ENABLE_SUBWOOFER)
            sinkSwatInit();
        #endif
        
        /* Is the input manager required? If so, read the Input Manager configuration data */
        #if defined(ENABLE_IR_REMOTE) || (defined(GATT_ENABLED) && defined(GATT_HID_CLIENT))
        {
            /* Input manager is required for the IR functionality */
            InputManagerInit();
        }
        #endif
        
        /* Is the Infra Red configuration required? */
        #if defined(ENABLE_IR_REMOTE)
        irRemoteControlInit();
        #endif
    }
    
}

/****************************************************************************
NAME 
  	configManagerInitFeatures

DESCRIPTION
  	Read and configure the system features from PS

RETURNS
  	void
    
*/
void configManagerInitFeatures( void )  
{    
    /* Read and configure the system features */
    configManagerFeatureBlock( );
}

/****************************************************************************
NAME
    configManagerProcessEventSysDefrag

DESCRIPTION
    Processes defrag related system events

RETURNS
    void
*/
void configManagerProcessEventSysDefrag(const MessageId defragEvent)
{
    if(sinkAudioIsAudioRouted())
    {
        MessageSendLater(&theSink.task, EventSysCheckDefrag, 0, D_SEC(sinkDataDefragCheckTimout()));
        }
    else if(IsAudioBusy())
    {
        MessageSendConditionallyOnTask(&theSink.task , EventSysCheckDefrag , 0, AudioBusyPtr());
    }
    else if(!powerManagerIsChargerConnected())
    {
        MessageSendLater(&theSink.task, EventSysCheckDefrag, 0, D_SEC(sinkDataDefragCheckTimout()));
    }

    else if(defragEvent == EventSysCheckDefrag)
    {
        /* check PS Store */
        if(configManagerDefragCheck())
        {
            /* defrag required, schedule a reboot */
            MessageSendLater(&theSink.task, EventSysDefrag, 0, D_SEC(sinkDataDefragCheckTimout()));
        }
        else
        {
            MessageSendLater(&theSink.task, EventSysCheckDefrag, 0, D_SEC(sinkDataDefragCheckTimout()));
        }
    }
    else if(defragEvent == EventSysDefrag)
    {
        configManagerDefrag(TRUE);
    }
}

/****************************************************************************
NAME
    configManagerDefragIfRequired

DESCRIPTION
    Check if PS Defrag is required, if it is then flood PS to force a
    PS Defragment operation on next reboot.

RETURNS
    void
*/
void configManagerDefragIfRequired(void)
{
    if(configManagerDefragCheck())
    {
        configManagerDefrag(TRUE);
    }
}

/******************************************************************************
NAME
    configManagerGetReadOnlyConfig

DESCRIPTION
    Open a config block as read-only.

    The pointer to the memory for the config block is returned in *data.

    If the block cannot be opened, this function will panic.

PARAMS
    config_id [in] Id of config block to open
    data [out] Will be set with the pointer to the config data buffer if
               successful, NULL otherwise.

RETURNS
    uint16 size of the opened config block buffer. The size can be 0,
    e.g. if the config is an empty array.Here size returned is not 
    equivalent to sizeof(), but represents the number of uint16's
    containing config blocks information.
*/
uint16 configManagerGetReadOnlyConfig(uint16 config_id, const void **data)
{
    config_store_status_t status;
    uint16 size = 0;

    status = ConfigStoreGetReadOnlyConfig(config_id, &size, data);
    if (config_store_success != status)
    {
        CONF_DEBUG(("CFG: configManagerGetReadOnlyConfig(): Error opening RO config block %u status %u\n", config_id, status));
        Panic();
    }

    return size;
}

/******************************************************************************
NAME
    configManagerGetWriteableConfig

DESCRIPTION
    Open a config block as writeable.

    The pointer to the memory for the config block is returned in *data.

    If the block cannot be opened, this function will panic.

PARAMS
    config_id [in] Id of config block to open
    data [out] Will be set with the pointer to the config data buffer if
               successful, NULL otherwise.
    size [in] Size of the buffer to allocate for the config data.
              Set this to 0 to use the size of config block in the
              config store.

RETURNS
    uint16 Size of the opened config block buffer. The size can be 0,
           e.g. if the config is an empty array.
*/
uint16 configManagerGetWriteableConfig(uint16 config_id, void **data, uint16 size)
{
    config_store_status_t status;
    uint16 config_size = size;

    status = ConfigStoreGetWriteableConfig(config_id, &config_size, data);
    if (config_store_success != status)
    {
        CONF_DEBUG(("CFG: configManagerGetWriteableConfig(): Error opening writeable config block %u status %u\n", config_id, status));
        Panic();
    }

    return config_size;
}

/******************************************************************************
NAME
    configManagerReleaseConfig

DESCRIPTION
    Release the given config block so that config_store can release any
    resources it is using to keep track of it.

    After this has been called any pointers to the config block data buffer
    will be invalid.

PARAMS
    config_id Id of the config block to release.

RETURNS
    void
*/
void configManagerReleaseConfig(uint16 config_id)
{
    ConfigStoreReleaseConfig(config_id);
}

/******************************************************************************
NAME
    configManagerUpdateWriteableConfig

DESCRIPTION
    Update the config block data in the config store and release it.

    After this has been called any pointers to the config block data buffer
    will be invalid.

PARAMS
    config_id Id of the config block to update and release.

RETURNS
    void
*/
void configManagerUpdateWriteableConfig(uint16 config_id)
{
    ConfigStoreWriteConfig(config_id);
    ConfigStoreReleaseConfig(config_id);
}
