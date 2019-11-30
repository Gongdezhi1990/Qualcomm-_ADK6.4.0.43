/****************************************************************************
Copyright (c) 2012 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_fm.c

DESCRIPTION
    This file handles connection and stream setup for FM Rx 
    (sink to plugin interfacing)
*/

#include <string.h>

#ifdef ENABLE_FM

#include <fm_rx_plugin.h>
#include <fm_rx_api.h>
#include <ps.h>

#include "sink_main_task.h"
#include "sink_debug.h"
#include "sink_statemanager.h"
#include "sink_configmanager.h"
#include "sink_powermanager.h"
#include "sink_config.h"
#include "sink_audio.h"

/* Include config store and definition headers */
#include "config_definition.h"
#include "sink_fm_config_def.h"
#include <config_store.h>

#ifdef ENABLE_SUBWOOFER
#include "sink_swat.h"
#endif

#include "sink_tones.h"
#include "sink_display.h"
#include "display_plugin_if.h"

#include "sink_audio_routing.h"
#include "sink_fm.h"

#include "audio_plugin_music_variants.h"

#ifdef DEBUG_FM
    #define FM_DEBUG(x) DEBUG(x)
    #define FM_ASSERT(x) { if (!(x)) { Panic(); } }
#else
    #define FM_DEBUG(x)
    #define FM_ASSERT(x) 
#endif

/*Sink FM Global Data */
typedef struct __sink_fm_globaldata_t
{
    bool fmRxOn;
    volume_info fm_volume;
    unsigned fmRxTunedFreq;
    fm_rx_data_t *fm_plugin_data;
} sink_fm_globaldata_t;

static sink_fm_globaldata_t *gFmData = NULL;
/* Global data for FM */
#define FM_DATA   gFmData

#define FM_PLUGIN_RX (TaskData *) &fm_rx_plugin


/****************************************************************************
NAME
    sinkFmConfigInit

DESCRIPTION
    Allocate memory and retrieve FM config from config lib
*/
static void sinkFmConfigInit(void)
{
    int lSize = (sizeof(fm_rx_data_t) + FMRX_MAX_BUFFER_SIZE);
    sink_fm_readonly_config_def_t *read_config_data = NULL;
       
    FM_DATA->fm_plugin_data = PanicUnlessMalloc( lSize );

    if (configManagerGetReadOnlyConfig(SINK_FM_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
       memcpy((fm_rx_config_t*)&FM_DATA->fm_plugin_data->config, &read_config_data->config, sizeof(fm_rx_config_t));
       configManagerReleaseConfig(SINK_FM_READONLY_CONFIG_BLK_ID);
    }    
}

/****************************************************************************
NAME
    sinkFmDataInit

DESCRIPTION
    Allocate memory for the global FM DATA pointer
*/
void sinkFmDataInit(void)
{
    if(!FM_DATA)
    {
       /* allocate FM global data memory if not already done */
       FM_DATA = PanicUnlessMalloc(sizeof(sink_fm_globaldata_t));
       memset(FM_DATA, 0, (sizeof(sink_fm_globaldata_t)));
    }
}

/****************************************************************************
NAME
    sinkFmSetFmVolume

DESCRIPTION
    Sets the FM receiver volume.
*/
void sinkFmSetFmVolume(volume_info fm_volume)
{
    FM_DATA->fm_volume = fm_volume;    
}

/****************************************************************************
NAME
    sinkFmGetFmVolume

DESCRIPTION
    Get the FM receiver volume.
*/
void sinkFmGetFmVolume(volume_info *fm_volume)
{
    *fm_volume = FM_DATA->fm_volume; /* Use the values set during read of session data during init */
}

/****************************************************************************
NAME
    sinkFmGetFmMainVolume

DESCRIPTION
    Get the FM receiver Main volume.
*/
int16 sinkFmGetFmMainVolume(void)
{
    return FM_DATA->fm_volume.main_volume;
}

/****************************************************************************
NAME
    sinkFmSetFmRxTunedFreq

DESCRIPTION
    Set the FM rx tuned freq value.
*/
void sinkFmSetFmRxTunedFreq(uint16 fmRxTunedFreq)
{
    FM_DATA->fmRxTunedFreq = fmRxTunedFreq;
}

/****************************************************************************
NAME
    sinkFmGetFmRxTunedFreq

DESCRIPTION
    Get the FM rx tuned freq value.
*/
uint16 sinkFmGetFmRxTunedFreq(void)
{
    return FM_DATA->fmRxTunedFreq;
}

/****************************************************************************
NAME    
    sinkFmInit
    
DESCRIPTION
    initialises the fm hardware, mode is passed to determine which part of the FM
    system is to be configured
*/   
static void sinkFmInit(fm_mode mode)
{
    FM_DEBUG(("sinkFmInit \n"));

     switch (mode)
    {
        case FM_ENABLE_RX:

            if( FM_DATA && (FM_DATA->fm_plugin_data == NULL) )
            {
                sinkFmConfigInit();
            }

            if(FM_DATA->fm_plugin_data)
               fmRxInit(FM_PLUGIN_RX, &theSink.task, FM_DATA->fm_plugin_data);
            
            break;
            
        case FM_ENABLE_TX:
        case FM_ENABLE_RX_TX:
        default:
            break;        
    }
}

void sinkFmRxPostConnectConfiguration(void)
{
    fmRxTuneFrequency(sinkFmGetFmRxTunedFreq());
}


bool sinkFmRxPopulateConnectParameters(audio_connect_parameters *connect_parameters)
{
    A2dpPluginConnectParams *audio_connect_params = getAudioPluginConnectParams();
    audio_connect_params->mode_params = getAudioPluginModeParams();

    connect_parameters->audio_plugin = (TaskData *)&csr_fm_decoder_plugin;
    connect_parameters->audio_sink = FM_SINK;
    connect_parameters->sink_type = AUDIO_SINK_FM;
    connect_parameters->volume = TonesGetToneVolumeInDb(audio_output_group_main);
    connect_parameters->rate = FM_SAMPLE_RATE;
    connect_parameters->features = sinkAudioGetPluginFeatures();
    connect_parameters->mode = AUDIO_MODE_CONNECTED;
    connect_parameters->route = AUDIO_ROUTE_INTERNAL;
    connect_parameters->power = powerManagerGetLBIPM();
    connect_parameters->params = audio_connect_params;
    connect_parameters->app_task = &theSink.task;
    return TRUE;
}

/****************************************************************************
NAME    
    sinkFmRxAudioPostDisconnectConfiguration
    
DESCRIPTION
    Disconnects the FM audio via the audio library/FM audio plugin
*/   
void sinkFmRxAudioPostDisconnectConfiguration(void)
{
    FM_DEBUG(("sinkFmRxAudioPostDisconnectConfiguration\n"));
    
    /* ensure FM is on and FM audio currently being routed to speaker */
    if (sinkFmIsFmRxOn())
    {
        FM_DEBUG(("FM audio disconnected \n"));

        /* Update limbo state */
        if (stateManagerGetState() == deviceLimbo )
            stateManagerUpdateLimboState();
    }
}

/****************************************************************************
NAME    
    sinkFmTuneUp
    
DESCRIPTION
    initiates an FM auto tune in an increasing frequency direction
*/   
static void sinkFmRxTuneUp(void)
{
    FM_DEBUG(("sinkFmRxTuneUp \n"));
    fmRxTuneUp();
}


/****************************************************************************
NAME    
    sinkFmTuneDown
    
DESCRIPTION
    initiates an FM auto tune in a decreasing frequency direction
*/   
static void sinkFmRxTuneDown(void)
{
    FM_DEBUG(("sinkFmRxTuneDown \n"));
    fmRxTuneDown();
}

/****************************************************************************
NAME    
    sinkFmDisplayFreq
    
DESCRIPTION
    utility function for displaying FM station on LCD 
    A favourite station will be indicated by appending a (*)
*/   
static void sinkFmDisplayFreq(uint16 freq, fm_display_type type )
{
    char display_freq[FM_DISPLAY_STR_LEN];
    uint8 len;
   
    if (type == FM_ADD_FAV_STATION)
    {      
        /*Add a star to a favourite station for the user to identify*/
        len = sprintf(display_freq, "%d.%d FM*", (freq/100), (freq%100)/10);
    }
    else
    {
        len = sprintf(display_freq, "%d.%d FM", (freq/100), (freq%100)/10);
    }

    FM_DEBUG(("FM display freq: %s  (len = %d)\n", display_freq, len));

    displayShowText((char*)display_freq, len, DISPLAY_TEXT_SCROLL_STATIC, 500, 2000, FALSE, 0, SINK_TEXT_TYPE_RADIO_FREQ);
}

/****************************************************************************
NAME    
    sinkFmGetIndex
    
DESCRIPTION
    utility function to get a index for requested operation
    In case of STORE, returns a free index.
    In case of ERASE, returns index corresponding to the requested freq

RETURNS
    index in PS key 
*/   
static uint8 sinkFmGetIndex(fm_stored_freq_t *stored_freq, uint16 freq)
{
    uint8 index;

    for (index=0;index<FM_MAX_PRESET_STATIONS;index++) 
    {
        if (stored_freq->freq[index] == freq)
        {
            break;
        }
    }

    FM_DEBUG(("sinkFmGetIndex (%d)\n", index));

    return index;
}

/****************************************************************************
NAME    
    sinkFmUpdateAtIndex
    
DESCRIPTION
    utility function to update freq in Ps key at requested index
*/   
static void sinkFmUpdateAtIndex(uint8 index, uint16 freq, fm_stored_freq_t *stored_freq)
{
    FM_ASSERT(index<FM_MAX_PRESET_STATIONS);
    stored_freq->freq[index] = freq;
}

/****************************************************************************
NAME    
    sinkFmRxStoreFreq
    
DESCRIPTION
    Stores the currently tuned frequency to persistant store, if storage is full
    signal to user
*/   
static void sinkFmRxStoreFreq(uint16 freq)
{
    sink_fm_writeable_config_def_t *write_config = NULL;
    fm_stored_freq_t *stored_freq = NULL;
    uint8 index = FM_MAX_PRESET_STATIONS;

    if (configManagerGetWriteableConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID, (void **)&write_config, 0))
    {
        stored_freq = &write_config->fmStoredFreq;
    }    

    FM_DEBUG(("sinkFmRxStoreFreq freq %d\n", freq));
    
    if ((freq != FM_INVALID_FREQ) && (stored_freq))
    {
        /*If requested freq already present in PSKEY, ignore the request*/
        if (sinkFmGetIndex(stored_freq, freq) < FM_MAX_PRESET_STATIONS)
        {       
            FM_DEBUG(("Freq already stored - Do nothing\n"));
            configManagerReleaseConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID);
        }
        else
        {
            /*Get free index*/
            index = sinkFmGetIndex(stored_freq, FM_INVALID_FREQ);
            
            if (index < FM_MAX_PRESET_STATIONS)
            {
                FM_DEBUG(("Stored station %d at index %d \n", freq, index));
                sinkFmUpdateAtIndex(index, freq, stored_freq);
                    
                /* store requested frequency */
                configManagerUpdateWriteableConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID);

                /*Display stored freq with a favourite sign*/
                sinkFmDisplayFreq(freq, FM_ADD_FAV_STATION);
            }
            else /*If no free index available, storage is full, indicate to user*/
            {
                displayShowText(DISPLAYSTR_FM_STORAGE_FULL, strlen(DISPLAYSTR_FM_STORAGE_FULL), DISPLAY_TEXT_SCROLL_SCROLL, 500, 2000, FALSE, 5, SINK_TEXT_TYPE_RADIO_FREQ);
                FM_DEBUG(("FM storage full. Please delete a stored station. \n"));
                configManagerReleaseConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID);
            }
        }
    }
}

/****************************************************************************
NAME    
    sinkFmRxEraseFreq
    
DESCRIPTION
    erases the currently tuned frequency if it is stored in persistant store
*/   
static void sinkFmRxEraseFreq(uint16 freq)
{    
    sink_fm_writeable_config_def_t *write_config = NULL;
    fm_stored_freq_t *stored_freq = NULL;
    uint8 index = FM_MAX_PRESET_STATIONS;

    FM_DEBUG(("sinkFmRxEraseFreq \n"));

    if (configManagerGetWriteableConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID, (void **)&write_config, 0))
    {
       stored_freq = &write_config->fmStoredFreq;
    }
    else
    {
        return; /* Failed to get writeable config */
    }
    /*Get index where requested freq is stored*/
    index = sinkFmGetIndex(stored_freq, freq);
    
    /*If no free index available, storage is full, indicate to user*/
    if (index < FM_MAX_PRESET_STATIONS)
    {    
        FM_DEBUG(("Erased station %d at index %d \n", freq, index));

        sinkFmUpdateAtIndex(index, FM_INVALID_FREQ, stored_freq);

        FM_DEBUG(("Station tuned to Freq %d erased \n", freq));

        /* erase the stored frequency */
        configManagerUpdateWriteableConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID);


        /*Display stored freq without a favourite sign, continue tuning until changed by user*/
        sinkFmDisplayFreq(freq, FM_DEL_FAV_STATION);
    }
    else
    {
        configManagerReleaseConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID);
    }
}

/****************************************************************************
NAME
    sinkFmGetNextStoredFreq

DESCRIPTION
    utility function to get return the next stored freq in after the
    current frequency
*/
static uint16 sinkFmGetNextStoredFreq(uint16 curr_freq, fm_stored_freq_t *stored_freq)
{
    uint16 firstValidFreq = FM_INVALID_FREQ;
    bool bFound = FALSE;
    uint8 index = 0;

    while (index < FM_MAX_PRESET_STATIONS)
    {
        if (stored_freq->freq[index])/*check for valid freq in stored list*/
        {
            if (firstValidFreq == FM_INVALID_FREQ)
            {
                /*Store the first valid freq for cases where the curr_freq is not present in stored list*/
                firstValidFreq = stored_freq->freq[index];
            }

            if (stored_freq->freq[index] == curr_freq)
            {
                bFound = TRUE;
            }
            else if (bFound == TRUE)
            {
                return stored_freq->freq[index];
            }
        }
        index++;
    }

    return firstValidFreq;
}

/****************************************************************************
NAME    
    sinkFmRxTuneToStore
    
DESCRIPTION
    tunes the FM receiver to the stored frequency in increasing order
    If currently tuned freq is the first stored freq, then the second entry in the stored list will be played.
    If the second entry is zero, the third entry will be tuned to.
    If no station is stored, request is ignored.
*/   
static void sinkFmRxTuneToStore(uint16 current_freq)
{
    sink_fm_writeable_config_def_t *write_config = NULL;
    uint16 tune_freq = FM_INVALID_FREQ;
    fm_stored_freq_t *stored_freq = NULL;

    FM_DEBUG(("sinkFmRxTuneToStore current freq %d \n", current_freq));

    if (configManagerGetWriteableConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID, (void **)&write_config, 0))
    {
       stored_freq = &write_config->fmStoredFreq;
    }
    else
    {
        return; /* Failed to get writeable config */
    }

    /*Check if currently tuned freq is stored, then tune to the next entry*/
    tune_freq = sinkFmGetNextStoredFreq(current_freq, stored_freq);
    
    FM_DEBUG(("Tune to freq %d \n", tune_freq));
    
    if(stored_freq)
       configManagerReleaseConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID);

    /* ensure valid frequency before attempting to tune to it */
    if(tune_freq!=FM_INVALID_FREQ)
        fmRxTuneFrequency(tune_freq);
    else
        MessageSend ( &theSink.task, EventSysError, 0) ;
}


/****************************************************************************
NAME    
    sinkFmRxUpdateVolume
    
DESCRIPTION
    Sets the volume output of the FM reciever chip itself, codec volume control
    is via the FM audio plugin
*/   
static void sinkFmRxUpdateVolume(uint8 vol)
{
    FM_DEBUG(("sinkFmRxUpdateVolume \n"));
    fmRxUpdateVolume(vol * FM_VOLUME_SCALE_FACTOR);
}

/****************************************************************************
NAME
    sinkFmAudioSinkMatch

DESCRIPTION
    Compare sink to FM audio sink.

RETURNS
    TRUE if Sink matches, FALSE otherwise
*/
bool sinkFmAudioSinkMatch(Sink sink)
{
    if(sinkFmIsFmRxOn())
        return (sink == sinkFmGetFmSink() && sink);
    return FALSE;
}

/****************************************************************************
NAME
    sinkFmIsFmRxOn

DESCRIPTION
    Check to see if the FM receiver is on.
*/
bool sinkFmIsFmRxOn(void)
{
    return ((FM_DATA->fmRxOn)?(TRUE):(FALSE));
}

/****************************************************************************
NAME
    sinkFmSetFmRxOn

DESCRIPTION
    Sets the FmRxOn flag.
*/
void sinkFmSetFmRxOn(bool isOn)
{
    FM_DATA->fmRxOn = isOn;
}

/****************************************************************************
NAME
    sinkFmRxPowerOff

DESCRIPTION
    Power off the FM receiver.
*/
static void sinkFmRxPowerOff(void)
{    
    FM_DEBUG(("sinkFmRxPowerOff \n"));
    fmRxPowerOff();
}

/****************************************************************************
NAME
    processEventUsrFmRxInOnState

DESCRIPTION
    processes On state FM related user events.

RETURN
    TRUE if handled, FALSE otherwise
*/
static bool processEventUsrFmRxInOnState(const MessageId EventUsrFmRx)
{
    bool success = TRUE;
    uint16 fmRxTunedFreq = sinkFmGetFmRxTunedFreq();

    switch(EventUsrFmRx)
    {
        case EventUsrFmRxOff:
            sinkFmRxPowerOff();
            sinkFmSetFmRxOn(FALSE);
            audioUpdateAudioRoutingAfterDisconnect();
            displayRemoveText(SINK_TEXT_TYPE_RADIO_FREQ);
            displayRemoveText(SINK_TEXT_TYPE_RADIO_INFO);
            break;
        case EventUsrFmRxTuneUp:
            sinkFmRxTuneUp();
            break;
        case EventUsrFmRxTuneDown:
            sinkFmRxTuneDown();
            break;
        case EventUsrFmRxStore:
            sinkFmRxStoreFreq(fmRxTunedFreq);
            break;
        case EventUsrFmRxTuneToStore:
            sinkFmRxTuneToStore(fmRxTunedFreq);
            break;
        case EventUsrFmRxErase:
            sinkFmRxEraseFreq(fmRxTunedFreq);
            break;
        default:
            success = FALSE;
            break;
    }
    return success;
}

/****************************************************************************
NAME
    processEventUsrFmRxInOffState

DESCRIPTION
    processes Off state FM related user events.

RETURN
    TRUE if handled, FALSE otherwise
*/
static bool processEventUsrFmRxInOffState(const MessageId EventUsrFmRx)
{
    bool success = FALSE;
    if(EventUsrFmRx == EventUsrFmRxOn)
    {
        if(stateManagerGetState() != deviceLimbo)
        {
            sinkFmInit(FM_ENABLE_RX);
            success = TRUE;
        }
    }
    return success;
}

/****************************************************************************
NAME
    sinkFmProcessEventUsrFmRx

DESCRIPTION
    processes FM related user events.

RETURN
    TRUE if handled, FALSE otherwise
*/
bool sinkFmProcessEventUsrFmRx(const MessageId EventUsrFmRx)
{
    bool success = TRUE;
    if(sinkFmIsFmRxOn())
    {
        success = processEventUsrFmRxInOnState(EventUsrFmRx);
    }
    else
    {
        success = processEventUsrFmRxInOffState(EventUsrFmRx);
    }
    return success;
}

/****************************************************************************
NAME
    sinkFmHandleFmPluginMessage

DESCRIPTION
    processes messages received from the FM plugin.
*/
void sinkFmHandleFmPluginMessage(const MessageId id, const Message message)
{
    switch (id)
    {
        /* received when the FM hardware has been initialised and tuned to the
           last used frequency */
        case FM_PLUGIN_INIT_IND:
        {
            FM_PLUGIN_INIT_IND_T *m = (FM_PLUGIN_INIT_IND_T*) message;
            FM_DEBUG(("HS: FM INIT: %d\n", m->result));

            if (m->result)
            {
                /* set the fm receiver hardware to default volume level (0x3F) */
                sinkFmRxUpdateVolume(sinkFmGetFmMainVolume());
                /* set flag to indicate FM audio is now available */
                sinkFmSetFmRxOn(TRUE);
                /* connect the FM audio if no other audio sources are avilable */
                audioUpdateAudioRouting();
            }
        }
        break;

        /* received when tuning is complete, the frequency tunes to is returned
           within the message, this is stored in persistant store */
        case FM_PLUGIN_TUNE_COMPLETE_IND:
        {
            FM_PLUGIN_TUNE_COMPLETE_IND_T *m = (FM_PLUGIN_TUNE_COMPLETE_IND_T*) message;
            FM_DEBUG(("HS: FM_PLUGIN_TUNE_COMPLETE_IND: %d\n", m->result));

            if (m->result)
            {
                /* valid the returned frequency and store for later writing to ps session data */
                if (m->tuned_freq!=0x0000)
                {
                    sinkFmSetFmRxTunedFreq(m->tuned_freq);
                    /*Display new frequency, clear older display*/
                    {
                        /*If the freq is stored in the Ps key, add special char for user to identify as favourite station */
                        uint8 index=0;
                        fm_display_type type=FM_SHOW_STATION;
                        sink_fm_writeable_config_def_t *write_config = NULL;
                        uint16 size = configManagerGetWriteableConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID, (void **)&write_config, 0);                       

                        for (index=0;index<FM_MAX_PRESET_STATIONS;index++)
                        {
                            if ((size) && (write_config->fmStoredFreq.freq[index] == m->tuned_freq))
                            {
                                type=FM_ADD_FAV_STATION;
                                break;
                            }
                        }
                        if(size)
                           configManagerReleaseConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID);

                        /*Display frequency*/
                        sinkFmDisplayFreq(m->tuned_freq, type);
                    }
                }
                FM_DEBUG(("FM RX currently tuned to freq (0x%x) (%d) \n", sinkFmGetFmRxTunedFreq(),
                                                                            sinkFmGetFmRxTunedFreq()));
            }
        }
        break;

#ifdef ENABLE_FM_RDS

        /*Display RDS info*/
        case FM_PLUGIN_RDS_IND:
        {
            FM_PLUGIN_RDS_IND_T *m = (FM_PLUGIN_RDS_IND_T*) message;
            FM_DEBUG(("HS: FM_PLUGIN_RDS_IND \n"));

            if ((m->data_len > 0) && (m->data != NULL))
            {
                FM_DEBUG(("HS:  data from msg (%s) (%d) \n", m->data, m->data_len));

                switch (m->rds_type)
                {
                case FMRX_RDS_PROGRAM_SERVICE:
                    FM_DEBUG(("HS: PS data %s \n", m->data));
                    displayShowText((char*)m->data, m->data_len, DISPLAY_TEXT_SCROLL_STATIC, 500, 2000, FALSE, 0, SINK_TEXT_TYPE_RADIO_INFO);
                    break;

                case FMRX_RDS_RADIO_TEXT:
                    FM_DEBUG(("HS: RT data %s \n", m->data));
                    displayShowText((char*)m->data, m->data_len, DISPLAY_TEXT_SCROLL_SCROLL, 500, 2000, FALSE, 50, SINK_TEXT_TYPE_RADIO_INFO);
                    break;

                case FMRX_RDS_PROGRAM_TYPE:
                    FM_DEBUG(("HS: Program Type %s \n", m->data));
                    displayShowText((char*)m->data, m->data_len, DISPLAY_TEXT_SCROLL_SCROLL, 500, 2000, FALSE, 50, SINK_TEXT_TYPE_RADIO_INFO);
                    break;

                default:
                    break;
                }
            }

        }
        break;

#endif /*ENABLE_FM_RDS*/

        default:
        FM_DEBUG(("HS :  FM unhandled msg [%x]\n",id)) ;
        break ;
    }
}

/****************************************************************************
NAME
    sinkFmGetFmSessionData

DESCRIPTION
    Gets FM related volume and tuned freq session data.
*/
void sinkFmGetFmSessionData(void)
{
    sink_fm_writeable_config_def_t *writeable_data = NULL;
    FM_DEBUG(("SinkFM: sinkFmGetFmVolumeTunedFreqData\n")); 

    if (configManagerGetReadOnlyConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID, (const void **)&writeable_data))
    {
       FM_DATA->fm_volume.main_volume = writeable_data->fm_volume.main_volume;
       FM_DATA->fm_volume.aux_volume = writeable_data->fm_volume.aux_volume;
       FM_DATA->fmRxTunedFreq = writeable_data->fmRxTunedFreq;
       configManagerReleaseConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID);
    }
}

/****************************************************************************
NAME
    sinkFmSetFmSessionData

DESCRIPTION
    Sets FM related volume and tuned freq session data using current data values.
*/
void sinkFmSetFmSessionData(void)
{
    sink_fm_writeable_config_def_t *writeable_data = NULL;
    FM_DEBUG(("SinkFM:sinkFmSetFmVolumeTunedFreqData()\n"));

    if (configManagerGetWriteableConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID, (void **)&writeable_data, 0))
    {
       writeable_data->fmRxTunedFreq = FM_DATA->fmRxTunedFreq;
       writeable_data->fm_volume.main_volume = FM_DATA->fm_volume.main_volume;
       writeable_data->fm_volume.aux_volume = FM_DATA->fm_volume.aux_volume;
       configManagerUpdateWriteableConfig(SINK_FM_WRITEABLE_CONFIG_BLK_ID);
    } 
}

#endif /*ENABLE_FM*/
