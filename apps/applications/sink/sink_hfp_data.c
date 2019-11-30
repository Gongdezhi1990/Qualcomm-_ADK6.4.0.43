/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_hfp.c

DESCRIPTION
    It contains HFP configuration data and Global data which need to be used by other modules
*/

#include "sink_debug.h"
#include "sink_bootmode.h"
#include "sink_hfp_data.h"
#include "sink_configmanager.h"
#include "sink_audio.h"
#include "sink_usb.h"
#include "sink_bredr.h"
#include "sink_config.h"
#include "config_definition.h"
#include "sink_hfp_data_config_def.h"
#include "sink_audio_rate_adjustment.h"
#include <config_store.h>

#include <hfp.h>
#include <csrtypes.h>
#include <boot.h>
#include <byte_utils.h>


#ifdef DEBUG_HFP
#define HFP_DATA_DEBUG(x) DEBUG(x)
#else
#define HFP_DATA_DEBUG(x)
#endif


#define SIZE_CONFIG_PHONE_NUMBER 20
#define SIZE_PHONE_NUMBER       21

typedef struct __sink_hfp_global_config_data_t
{
    hfp_common_plugin_params_t  hfp_plugin_params;
    sink_hfp_at_commands_config_def_t *hfp_at_commands_data; /* Stored reference to AT Command data for use after we release the config blk */
    unsigned         size_at_commands_data;
    sink_hfp_event_at_command_table_config_def_t *hfp_event_at_command_table; /* Stored reference to Event-Command mapping for use after we release the config blk */
    unsigned         size_event_at_command_table;
    hfp_audio_params *audio_params;  /* Note: this is required to provide reference of audio params in global because we release the config blk id once we retrieve it, returning the address of audio params becomes invalid*/
    profile_data_t   profile_data[MAX_MULTIPOINT_CONNECTIONS];
    unsigned         NetworkIsPresent:1;
    unsigned         last_outgoing_ag:2 ;   /* which AG made the last outgoing call */
    unsigned         RepeatCallerIDFlag:1;
    unsigned         HeldCallIndex:4;           /* which call to route in the case of multiple held calls */
    unsigned         MissedCallIndicated:8;
#ifdef WBS_TEST
    unsigned         FailAudioNegotiation:1;
    unsigned         RenegotiateSco:1;
#endif
#if defined(TEST_HF_INDICATORS)
    unsigned         hf_enhancedSafety:1;
#endif
    unsigned         VoiceRecognitionIsActive:2;
    sync_pkt_type    packet_types:10; /*Note:This is readonly config and gets updated at runtime without writing to config store. So, make it global after reading from readonly config */
    unsigned         unused:1;
    unsigned         linkLossReminderTime:8;/*Note:This is readonly config and gets updated at runtime without writing to config store. So,make it global after reading from readonly config */
    unsigned         audio_plugin:4; /*Note:This is readonly config and gets updated at runtime without writing to config store. So,make it global after reading from readonly config */
    unsigned         hfp_profiles:3; /*Note:This is readonly config and gets updated at runtime without writing to config store. So,make it global after reading from readonly config */
    unsigned         spare:1;
}sink_hfp_global_config_data_t;

static sink_hfp_global_config_data_t gHfpData;
#define G_HFP_DATA gHfpData

/* Pack higher and lower uint16s to form Uint32 bitmask */
#define PACK_32(bandwidth_low, bandwidth_high) (((uint32)bandwidth_high << 16) | ((uint32) (bandwidth_low) & 0x0000FFFFUL))

enum
{
    hfp_codec_support_nb = 0x01,
    hfp_codec_support_wb = 0x02,
    hfp_codec_support_uwb = 0x04,
    hfp_codec_support_swb = 0x08
} hfp_codec_support_t;


/*************************************************************************
NAME
    sinkHfpDataGetVoiceRecognitionIsActive

DESCRIPTION
    Function to get VoiceRecognitionIsActive.
RETURNS
    uint8
*/
uint8 sinkHfpDataGetVoiceRecognitionIsActive(void)
{
     return G_HFP_DATA.VoiceRecognitionIsActive;
}
/*************************************************************************
NAME
    sinkHfpDataSetVoiceRecognitionIsActive

DESCRIPTION
    Function to set VoiceRecognitionIsActive.

*/
void sinkHfpDataSetVoiceRecognitionIsActive(uint16 status)
{
     G_HFP_DATA.VoiceRecognitionIsActive = status;
}

/*************************************************************************
NAME
    sinkHfpDataGetVoiceRecognitionSupported

DESCRIPTION
    Get if the specific HFP connection supports voice recognition.

RETURNS
    bool TRUE if voice recognition is supported, FALSE otherwise.
*/
bool sinkHfpDataGetVoiceRecognitionSupported(uint8 index)
{
     return (bool)G_HFP_DATA.profile_data[index].status.voice_recognition_supported;
}

/*************************************************************************
NAME
    sinkHfpDataSetVoiceVoiceRecognitionSupported

DESCRIPTION
    Set if the specific HFP connection supports voice recognition.
*/
void sinkHfpDataSetVoiceRecognitionSupported(uint8 index, bool supported)
{
     G_HFP_DATA.profile_data[index].status.voice_recognition_supported = supported;
}

/*************************************************************************
NAME
    sinkHfpDataUpdateStoredNumber

DESCRIPTION
    Function to update the stored number.

*/
void sinkHfpDataUpdateStoredNumber(uint8 * phone_number, uint16 size_phone_number)
{
    if (size_phone_number <= SIZE_CONFIG_PHONE_NUMBER)
    {
        sink_hfp_data_writeable_config_def_t *write_data = NULL;

        HFP_DATA_DEBUG(("Store Number\n"));
        if (configManagerGetWriteableConfig(SINK_HFP_DATA_WRITEABLE_CONFIG_BLK_ID, (void **)&write_data, 0))
        {
            memset(write_data->phone_number, 0, sizeof write_data->phone_number);
            ByteUtilsMemCpyPackString((uint16 *) write_data->phone_number, phone_number, size_phone_number);
            configManagerUpdateWriteableConfig(SINK_HFP_DATA_WRITEABLE_CONFIG_BLK_ID);
        }
    }
}

/*************************************************************************
NAME
    sinkHfpDataGetStoredNumber

DESCRIPTION
    Function to get stored phone number.

RETURNS
    uint16: Length of data

*/
uint16 sinkHfpDataGetStoredNumber(uint16 *phone_number)
{
    uint16 len_words = 0;
    uint16 len_bytes = 0;
    sink_hfp_data_writeable_config_def_t *write_data = NULL;

    len_words = configManagerGetReadOnlyConfig(SINK_HFP_DATA_WRITEABLE_CONFIG_BLK_ID, (const void **)&write_data);
    if (len_words)
    {
        /* Find length of phone number string and unpack. */
        len_bytes = ByteUtilsGetPackedStringLen((const uint16*)write_data->phone_number, len_words);
        ByteUtilsMemCpyUnpackString((uint8 *)phone_number, (const uint16*)write_data->phone_number, len_bytes);
        configManagerReleaseConfig(SINK_HFP_DATA_WRITEABLE_CONFIG_BLK_ID);
    }
    return len_bytes;
}

static hfp_wbs_codec_mask getSupportedWbsCodecs(unsigned supported_codecs)
{
    hfp_wbs_codec_mask wbs_codecs = hfp_wbs_codec_mask_none;
    if(supported_codecs & hfp_codec_support_nb)
    {
        wbs_codecs |= hfp_wbs_codec_mask_cvsd;
    }
    if(supported_codecs & hfp_codec_support_wb)
    {
        wbs_codecs |= hfp_wbs_codec_mask_msbc;
    }
    return wbs_codecs;
}

static codec_mode_bit_type getSupportedQ2QCodecs(unsigned supported_codecs)
{
    codec_mode_bit_type q2q_codecs = 0;
    if(supported_codecs & hfp_codec_support_uwb)
    {
        q2q_codecs |= CODEC_MODE_ID_MASK_UWB;
    }
    if(supported_codecs & hfp_codec_support_swb)
    {
        q2q_codecs |= CODEC_MODE_ID_MASK_SWB;
    }
    return q2q_codecs;
}

/*************************************************************************
NAME
    sinkHfpUpdateInitParams

DESCRIPTION
    Function to initialize hfp_init_params.

RETURNS
    void

*/
static void sinkHfpUpdateInitParams(hfp_init_params *hfp_init_param, sink_hfp_data_readonly_config_def_t *hfp_config_data)
{
    hfp_init_param->supported_profile = hfp_config_data->hfp_init_params.supported_profile_def ;
    hfp_init_param->supported_features = hfp_config_data->hfp_init_params.supported_features_def ;
    hfp_init_param->supported_wbs_codecs = getSupportedWbsCodecs(hfp_config_data->hfp_init_params.supported_hfp_codecs_def);

    hfp_init_param->optional_indicators.service = hfp_config_data->hfp_init_params.optional_indicators_def.service;
    hfp_init_param->optional_indicators.signal_strength = hfp_config_data->hfp_init_params.optional_indicators_def.signal_strength;
    hfp_init_param->optional_indicators.roaming_status = hfp_config_data->hfp_init_params.optional_indicators_def.roaming_status;
    hfp_init_param->optional_indicators.battery_charge = hfp_config_data->hfp_init_params.optional_indicators_def.battery_charge;

    hfp_init_param->disable_nrec = hfp_config_data->hfp_init_params.disable_nrec_def;
    hfp_init_param->extended_errors = hfp_config_data->hfp_init_params.extended_errors_def;
    hfp_init_param->multipoint = hfp_config_data->hfp_init_params.multipoint_def;
    hfp_init_param->link_loss_time = hfp_config_data->hfp_init_params.link_loss_time_def;
    hfp_init_param->link_loss_interval = hfp_config_data->hfp_init_params.link_loss_interval_def;
    hfp_init_param->csr_features.caller_name= hfp_config_data->hfp_init_params.qtil_supported_features_def.caller_name;
    hfp_init_param->csr_features.codec_bandwidths = hfp_config_data->hfp_init_params.qtil_supported_features_def.codec_bandwidths;
    hfp_init_param->csr_features.raw_text = hfp_config_data->hfp_init_params.qtil_supported_features_def.raw_text;
    hfp_init_param->csr_features.sms = hfp_config_data->hfp_init_params.qtil_supported_features_def.sms;
    hfp_init_param->csr_features.batt_level = hfp_config_data->hfp_init_params.qtil_supported_features_def.batt_level;
    hfp_init_param->csr_features.pwr_source = hfp_config_data->hfp_init_params.qtil_supported_features_def.pwr_source;
    hfp_init_param->csr_features.codecs = hfp_config_data->hfp_init_params.qtil_supported_features_def.supported_codecs;
    hfp_init_param->hf_indicators = hfp_config_data->hfp_init_params.hf_indicators_mask_def;
    hfp_init_param->hf_codec_modes = getSupportedQ2QCodecs(hfp_config_data->hfp_init_params.supported_hfp_codecs_def);
}

/*************************************************************************
NAME
    sinkHfpInitGlobalData

DESCRIPTION
    Function to initialize global data from readonly config .

RETURNS
    void

*/
static void sinkHfpInitGlobalData(sink_hfp_data_readonly_config_def_t *hfp_config_data)
{
    /*Note:
    Update Global Data : this is required because these variables are updated in runtime after reading from the config.
    So, these will be read first from read config during hfp initialization and keep under global data.
    Global references will be used for further runtime update and reading*/
    G_HFP_DATA.hfp_profiles = hfp_config_data->hfp_init_params.supported_profile_def;
    G_HFP_DATA.linkLossReminderTime = hfp_config_data->hfp_init_params.link_loss_interval_def;
    G_HFP_DATA.packet_types =hfp_config_data->hfp_supported_features.packet_types;
    G_HFP_DATA.audio_plugin = hfp_config_data->hfp_features_config.audio_plugin;

    /* Note :
    Structure alignment for hfp_audio_params*(from library) and	hfp_audio_params_t(generated .h from xml) is different.
    So, it is better way to have a hfp_audio_params memebr in HFP global data,then copy the data element wise to hfp_audio_params from hfp_audio_params_t,
    and for 32 bit use macro PACK_32 to pack into 32 bits*/

    G_HFP_DATA.audio_params = PanicUnlessMalloc(sizeof(hfp_audio_params));
    memset(G_HFP_DATA.audio_params, 0,sizeof(hfp_audio_params));

    G_HFP_DATA.audio_params->bandwidth = PACK_32(hfp_config_data->hfp_supported_features.audio_params.bandwidth_low,hfp_config_data->hfp_supported_features.audio_params.bandwidth_high);
    G_HFP_DATA.audio_params->max_latency = hfp_config_data->hfp_supported_features.audio_params.max_latency;
    G_HFP_DATA.audio_params->voice_settings = hfp_config_data->hfp_supported_features.audio_params.voice_settings;
    G_HFP_DATA.audio_params->retx_effort = hfp_config_data->hfp_supported_features.audio_params.retransmission_effort;

    sinkAudioRateAdjustmentInit();

}

/*************************************************************************
NAME
    sinkHfpDataInit

DESCRIPTION
    Function to Initialise the Hfp configuration data.

RETURNS
    bool

*/
bool sinkHfpDataInit(hfp_init_params *hfp_init_param)
{
    sink_hfp_data_readonly_config_def_t *hfp_config_data = NULL;

    G_HFP_DATA.hfp_at_commands_data = NULL;
    G_HFP_DATA.size_at_commands_data = 0;

    G_HFP_DATA.hfp_event_at_command_table = NULL;
    G_HFP_DATA.size_event_at_command_table = 0;

    if(BootGetMode() != BOOTMODE_DFU)
    {
        uint16 size = configManagerGetReadOnlyConfig(SINK_HFP_EVENT_AT_COMMAND_TABLE_CONFIG_BLK_ID,
                                       (const void **)&G_HFP_DATA.hfp_event_at_command_table);

        G_HFP_DATA.size_event_at_command_table = size * sizeof (uint16) / sizeof G_HFP_DATA.hfp_event_at_command_table[0];

        /* Note : if AT commands data contains NULL values, it means it is not valid, so return NULL at data  */
        if((G_HFP_DATA.hfp_event_at_command_table != NULL)
            && (G_HFP_DATA.hfp_event_at_command_table->event_at_commands[0].event == 0)
            && (G_HFP_DATA.hfp_event_at_command_table->event_at_commands[0].at_cmd == 0))
        {
            configManagerReleaseConfig(SINK_HFP_EVENT_AT_COMMAND_TABLE_CONFIG_BLK_ID);
            G_HFP_DATA.hfp_event_at_command_table = NULL;
        }
        else
        {
            G_HFP_DATA.size_at_commands_data = configManagerGetReadOnlyConfig(SINK_HFP_AT_COMMANDS_CONFIG_BLK_ID,
                                           (const void **)&G_HFP_DATA.hfp_at_commands_data);
        }
    }

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_config_data))
    {
        /* Note:
           The structure elements alignment for both hfp_init_params(from library) and hfp_config_data->hfp_init_param(script generated from xml)is different.
           This has been filled structure member by member because hfp_init_params is a library structure
           which is passed as arguement in hfpInit() interface to library. Whereas hfp_config_data->hfp_init_param is script generated header definition.
           */
        sinkHfpUpdateInitParams(hfp_init_param,hfp_config_data);


        sinkHfpInitGlobalData(hfp_config_data);

        /* Make sure all references to mic parameters point to the right place */
        sinkAudioGetCommonMicParams(&G_HFP_DATA.hfp_plugin_params.voice_mic_params);
        
        sinkUsbGetHfpPluginParams(&G_HFP_DATA.hfp_plugin_params.usb_params);

        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);

        return TRUE;
    }
        return FALSE;
}

/*************************************************************************
NAME
    sinkHfpDataGetHfpPluginParams

DESCRIPTION
    Functions to retrieve HFP plugin params.

RETURNS
    Pointer to hfp_common_plugin_params_t.

*/
hfp_common_plugin_params_t * sinkHfpDataGetHfpPluginParams(void)
{
    return &G_HFP_DATA.hfp_plugin_params;
}

/*************************************************************************
NAME
    sinkHfpDataHasAtCmdsData

DESCRIPTION
    Function to check if HFP AT Commands data present.

RETURNS
    bool

*/
bool sinkHfpDataHasAtCmdsData(void)
{
     return G_HFP_DATA.size_at_commands_data != 0;
}

/*************************************************************************
NAME
    sinkHfpDataGetActionOnCallTransfer

DESCRIPTION
    Functions to return HFP Features config- ActionOnCallTransfer.

RETURNS
    uint8

*/
uint8 sinkHfpDataGetActionOnCallTransfer(void)
{
    uint8 ret_val = 0;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
        ret_val = hfp_read_config_data->hfp_features_config.ActionOnCallTransfer;
        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

/*************************************************************************
NAME
    sinkHfpDataGetAudioPlugin

DESCRIPTION
    Functions to return HFP Features config audio plugin.

RETURNS
    uint8

*/
uint8 sinkHfpDataGetAudioPlugin(void)
{
#ifdef CVC_PRODTEST
    sink_hfp_data_readonly_config_def_t *hfp_config_data = NULL;
    uint8_t audio_plugin;

    PanicFalse(configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_config_data));
    audio_plugin = hfp_config_data->hfp_features_config.audio_plugin;
    configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);

    return audio_plugin;
#else
    return G_HFP_DATA.audio_plugin;
#endif
}

/*************************************************************************
NAME
    sinkHfpDataSetAudioPlugin

DESCRIPTION
    Interface function to set HFP Features config audio plugin.
*************************************************************************/
void sinkHfpDataSetAudioPlugin(uint16 audio_plugin)
{
      G_HFP_DATA.audio_plugin = audio_plugin;
}

/*************************************************************************
NAME
    sinkHfpDataIsForceEV3S1ForSco2

DESCRIPTION
    Functions to check ForceEV3S1ForSco2.

RETURNS
    bool

*/
bool sinkHfpDataIsForceEV3S1ForSco2  (void)
{
    uint16 ret_val = 0;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
      ret_val = hfp_read_config_data->hfp_features_config.ForceEV3S1ForSco2;
      configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

/*************************************************************************
NAME
    sinkHfpDataUpdateSupportedProfile

DESCRIPTION
    Function to update the Hfp supported profile

RETURNS
    void

*/
void sinkHfpDataUpdateSupportedProfile(hfp_profile profile)
{
    G_HFP_DATA.hfp_profiles = profile;
}

/*************************************************************************
NAME
    sinkHfpDataGetSupportedProfile

DESCRIPTION
    To get the Hfp supported profile
RETURNS
    uint8
*/
uint8 sinkHfpDataGetSupportedProfile(void)
{
     return G_HFP_DATA.hfp_profiles;
}

/*************************************************************************
NAME
    sinkHfpDataUpdateSyncPktTypes

DESCRIPTION
    Function to update HFP sync pkt types.

RETURNS
    void

*/
void sinkHfpDataUpdateSyncPktTypes(void)
{
    G_HFP_DATA.packet_types |= (sync_hv1 |sync_hv2 |sync_ev3);
}

/*************************************************************************
NAME
    sinkHfpDataGetNWSIndicatorRepeatTime

DESCRIPTION
    Functions to return HFP Features timeouts - NetworkServiceIndicatorRepeatTime.

RETURNS
    uint16

*/
uint16 sinkHfpDataGetNWSIndicatorRepeatTime(void)
{
    uint16 ret_val = 0;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
        ret_val = hfp_read_config_data->hfp_feature_timeouts.NwsServiceIndicatorRepeatTime_s;
        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

bool sinkHfpDataSetNWSIndicatorRepeatTime(uint8 repeat_timeout)
{
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetWriteableConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (void **)&hfp_read_config_data, 0))
    {
        hfp_read_config_data->hfp_feature_timeouts.NwsServiceIndicatorRepeatTime_s = repeat_timeout;
        configManagerUpdateWriteableConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME
    sinkHfpDataGetMuteRemindTime

DESCRIPTION
    Functions to return HFP Features timeouts - MuteRemindTime.

RETURNS
    uint16

*/
uint16 sinkHfpDataGetMuteRemindTime(void)
{
    uint16 ret_val = 0;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
        ret_val = hfp_read_config_data->hfp_feature_timeouts.MuteRemindTime;
        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

bool sinkHfpDataSetMuteRemindTime(uint16 mute_timeout)
{
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetWriteableConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (void **)&hfp_read_config_data, 0))
    {
        hfp_read_config_data->hfp_feature_timeouts.MuteRemindTime = mute_timeout;
        configManagerUpdateWriteableConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME
    sinkHfpDataGetMissedCallIndicateTime

DESCRIPTION
    Functions to return HFP Features timeouts - MissedCallIndicateTime.

RETURNS
    uint16

*/
uint16 sinkHfpDataGetMissedCallIndicateTime(void)
{
    uint16 ret_val = 0;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
        ret_val = hfp_read_config_data->hfp_feature_timeouts.MissedCallIndicateTime_s;
        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

bool sinkHfpDataSetMissedCallIndicateTime(uint16 timeout)
{
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetWriteableConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (void **)&hfp_read_config_data, 0))
    {
        hfp_read_config_data->hfp_feature_timeouts.MissedCallIndicateTime_s = timeout;
        configManagerUpdateWriteableConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME
    sinkHfpDataGetMissedCallIndicateAttempsTime

DESCRIPTION
    Functions to return HFP Features timeouts - MissedCallIndicateAttemps.

RETURNS
    uint16

*/
uint16 sinkHfpDataGetMissedCallIndicateAttempsTime(void)
{
    uint16 ret_val = 0;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
        ret_val = hfp_read_config_data->hfp_feature_timeouts.MissedCallIndicateAttempts;
        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

bool sinkHfpDataSetMissedCallIndicateAttempsTime(uint16 timeout)
{
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetWriteableConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (void **)&hfp_read_config_data, 0))
    {
        hfp_read_config_data->hfp_feature_timeouts.MissedCallIndicateAttempts = timeout;
        configManagerUpdateWriteableConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME
    GetHfpFeaturesSyncType

DESCRIPTION
    Function to get Hfp Supported Features type .

RETURNS
    sync_pkt_type

*/
sync_pkt_type sinkHfpDataGetFeaturesSyncType(void)
{
    return G_HFP_DATA.packet_types;
}

/*************************************************************************
NAME
    GetHfpFeaturesAdditionalAudioParams

DESCRIPTION
    Function to get Hfp Features Additional params .

RETURNS
    bool

*/
bool sinkHfpDataGetFeaturesAdditionalAudioParams(void)
{
    bool add_audio_params = FALSE;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
        add_audio_params = hfp_read_config_data->hfp_supported_features.additional_audio_params;
        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return (add_audio_params)?(TRUE):(FALSE);
}

/*************************************************************************
NAME
    GetHfpFeaturesTypeAudioParams

DESCRIPTION
    Function to get Hfp Features audio params .

RETURNS
    Address to hfp_audio_params

*/
hfp_audio_params* sinkHfpDataGetFeaturesTypeAudioParams(void)
{
    HFP_DATA_DEBUG(("HFP : AUD : [%lx][%x][%x][%x]\n", G_HFP_DATA.audio_params->bandwidth,G_HFP_DATA.audio_params->max_latency,
                                            G_HFP_DATA.audio_params->voice_settings,G_HFP_DATA.audio_params->retx_effort));
    return G_HFP_DATA.audio_params;
}

/*************************************************************************
NAME
    sinkHfpDataIsLNRCancelsVoiceDialIfActive

DESCRIPTION
    Function to check LNRCancelsVoiceDialIfActive features.

RETURNS
    bool

*/
bool sinkHfpDataIsLNRCancelsVoiceDialIfActive(void)
{
    bool ret_val = FALSE;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
        ret_val = hfp_read_config_data->hfp_features_config.LNRCancelsVoiceDialIfActive;
        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

/*************************************************************************
NAME
    sinkHfpDataIsEnableSyncMuteMicrophones

DESCRIPTION
    Function to check EnableSyncMuteMicrophones features.

RETURNS
    bool

*/
bool sinkHfpDataIsEnableSyncMuteMicrophones(void)
{
    bool ret_val = FALSE;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
        ret_val = hfp_read_config_data->hfp_features_config.EnableSyncMuteMicrophones;
        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

/*************************************************************************
NAME
    sinkHfpDataIsAutoAnswerOnConnect

DESCRIPTION
    Function to check AutoAnswerOnConnect features.

RETURNS
    bool

*/
bool sinkHfpDataIsAutoAnswerOnConnect(void)
{
    bool ret_val = FALSE;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
        ret_val = hfp_read_config_data->hfp_features_config.AutoAnswerOnConnect;
        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

/*************************************************************************
NAME
    sinkHfpDataGetCallActivePIO

DESCRIPTION
    Function to get CallActivePIO.

RETURNS
    uint8

*/
uint8 sinkHfpDataGetCallActivePIO(void)
{
    uint8 ret_val = 0;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
        ret_val = hfp_read_config_data->hfp_pio_data.CallActivePIO;
        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

/*************************************************************************
NAME
    sinkHfpDataGetIncomingRingPIO

DESCRIPTION
    Function to get IncomingRingPIO.

RETURNS
    uint8

*/
uint8 sinkHfpDataGetIncomingRingPIO(void)
{
    uint8 ret_val = 0;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
        ret_val = hfp_read_config_data->hfp_pio_data.IncomingRingPIO;
        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

/*************************************************************************
NAME
    sinkHfpDataGetOutgoingRingPIO

DESCRIPTION
    Function to get OutgoingRingPIO.

RETURNS
    uint8

*/
uint8 sinkHfpDataGetOutgoingRingPIO(void)
{
    uint8 ret_val = 0;
    sink_hfp_data_readonly_config_def_t *hfp_read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&hfp_read_config_data))
    {
        ret_val = hfp_read_config_data->hfp_pio_data.OutgoingRingPIO;
        configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

/************************************************************************************
NAME
    sinkHfpDataGetEventCount

DESCRIPTION
    Function to get number of configured AT command events.

RETURNS
    Number of events

*/
uint16 sinkHfpDataGetEventCount(void)
{
    return G_HFP_DATA.size_event_at_command_table;
}

/************************************************************************************
NAME
    sinkHfpDataGetEventAtCommandsEvent

DESCRIPTION
    Function to get At commands event.

RETURNS
    uint8

*/
uint16 sinkHfpDataGetEventAtCommandsEvent(uint16 i)
{
    return G_HFP_DATA.hfp_event_at_command_table->event_at_commands[i].event;
}

/************************************************************************************
NAME
    sinkHfpDataGetEventAtCommands

DESCRIPTION
    Function to get At commands .

RETURNS
    uint16

*/
uint16 sinkHfpDataGetEventAtCommands(uint16 i)
{
    return G_HFP_DATA.hfp_event_at_command_table->event_at_commands[i].at_cmd;
}

/************************************************************************************
NAME
    sinkHfpDataGetAtCommandsSize

DESCRIPTION
    Function to get size of AT Commands data.

RETURNS
    uint16
*/
uint16 sinkHfpDataGetAtCommandsSize(void)
{
    return G_HFP_DATA.size_at_commands_data;
}


/*************************************************************************
NAME
    sinkHfpDataGetAtCommands

DESCRIPTION
    Function to get At commands .

RETURNS
    Pointer to packed command string

*/
uint16 *sinkHfpDataGetAtCommands(void)
{
     return (uint16 *) G_HFP_DATA.hfp_at_commands_data->at_commands;
}

void sinkHfpDataSetAudioValid(uint8 index, bool status)
{
    G_HFP_DATA.profile_data[index].audio.audio_valid = (status) ? 1 : 0;
}

bool sinkHfpDataGetAudioValid(uint8 index)
{
    if (G_HFP_DATA.profile_data[index].audio.audio_valid)
    {
        return TRUE;
    }
    return FALSE;
}

/******************************************************************************/
uint8 sinkHfpDataGetDefaultVolume(void)
{
    uint8 default_hfp_volume = 0x0;
    sink_hfp_data_readonly_config_def_t *config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (const void **)&config_data))
    {
       default_hfp_volume = config_data->hfp_default_volume;
       configManagerReleaseConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
    }
    HFP_DATA_DEBUG(("HFP: Get Default volume [%d]\n", default_hfp_volume));
    return default_hfp_volume;
}

/******************************************************************************/
bool sinkHfpDataSetDefaultVolume(uint8 volume)
{
    sink_hfp_data_readonly_config_def_t *config_data = NULL;

    HFP_DATA_DEBUG(("HFP: Set Default volume [%d]\n", volume));

    if (configManagerGetWriteableConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID, (void **)&config_data, 0))
    {
       config_data->hfp_default_volume = volume;
       configManagerUpdateWriteableConfig(SINK_HFP_DATA_READONLY_CONFIG_BLK_ID);
       return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME
    sinkHfpdataSetAudioSMVolumeLevel

DESCRIPTION
    Function to set audio data Volume level.

RETURNS
    void

*/
void sinkHfpdataSetAudioSMVolumeLevel( uint16 volume_level, uint8 index)
{
    G_HFP_DATA.profile_data[index].audio.gSMVolumeLevel= volume_level;
}

/*************************************************************************
NAME
    sinkHfpDataGetAudioSMVolumeLevel

DESCRIPTION
    Function to get audio data Volume level.

RETURNS
    uint8

**************************************************************************/
uint8 sinkHfpDataGetAudioSMVolumeLevel(uint8 index)
{
   return G_HFP_DATA.profile_data[index].audio.gSMVolumeLevel;
}

/*************************************************************************
NAME
    sinkHfpDataSetAudioSynclinkType

DESCRIPTION
    Function to set audio data Sync Link Type

RETURNS
    void

*/
void sinkHfpDataSetAudioSynclinkType( sync_link_type link_type, uint8 index)
{
   G_HFP_DATA.profile_data[index].audio.link_type = link_type;
}

/*************************************************************************
NAME
    sinkHfpDataGetAudioSynclinkType

DESCRIPTION
    Function to get audio data Sync Link Type

RETURNS
    sync_link_type

**************************************************************************/
sync_link_type sinkHfpDataGetAudioSynclinkType(uint8 index)
{
   return G_HFP_DATA.profile_data[index].audio.link_type;
}

/*************************************************************************
NAME
    sinkHfpDataSetAudioCodec

DESCRIPTION
    Function to set audio data codec selected

RETURNS
    void

*/
void sinkHfpDataSetAudioCodec(hfp_wbs_codec_mask codec_selected, uint8 index)
{
   G_HFP_DATA.profile_data[index].audio.codec_selected = codec_selected;
}

/*************************************************************************
NAME
    sinkHfpDataGetAudioCodec

DESCRIPTION
    Function to get audio data codec selected

RETURNS
    hfp_wbs_codec_mask

*/
hfp_wbs_codec_mask sinkHfpDataGetAudioCodec(uint8 index)
{
   return G_HFP_DATA.profile_data[index].audio.codec_selected;
}

/*************************************************************************
NAME
    sinkHfpDataGetAudioSink

DESCRIPTION
    Function to get the HFP audio sink

RETURNS
    HFP audio sink or NULL if the sink could not be retrieved

*/
Sink sinkHfpDataGetAudioSink(uint8 index)
{
    Sink sink = NULL;

    if (index < MAX_MULTIPOINT_CONNECTIONS)
    {
        if (sinkHfpDataGetAudioValid(index))
        {
            hfp_link_priority link_priority = hfp_primary_link + index;
            HfpLinkGetAudioSink(link_priority, &sink);
        }
    }

    return sink;
}

/*************************************************************************
NAME
    sinkHfpDataSetScoPriority

DESCRIPTION
    Function to set audio data sco priority

RETURNS
    void

*/
void sinkHfpDataSetScoPriority(audio_priority sco_priority, uint16 index)
{
   G_HFP_DATA.profile_data[index].audio.sco_priority = sco_priority;
}

/*************************************************************************
NAME
    sinkHfpDataGetScoPriority

DESCRIPTION
    Function to get audio data sco priority

RETURNS
    audio_priority

*/
audio_priority sinkHfpDataGetScoPriority(uint16 index)
{
   return G_HFP_DATA.profile_data[index].audio.sco_priority;
}

/*************************************************************************
NAME
    sinkHfpDataSetAudioTxBandwidth

DESCRIPTION
    Function to set audio data Tx bandwidth

RETURNS
    void

*/
void sinkHfpDataSetAudioTxBandwidth(uint32 tx_bandwidth, uint8 index)
{
    G_HFP_DATA.profile_data[index].audio.tx_bandwidth = tx_bandwidth;
}

/*************************************************************************
NAME
    sinkHfpDataGetAudioTxBandwidth

DESCRIPTION
    Function to get audio data Tx bandwidth

RETURNS
    uint32

*/
uint32 sinkHfpDataGetAudioTxBandwidth(uint8 index)
{
    return G_HFP_DATA.profile_data[index].audio.tx_bandwidth;
}

/*************************************************************************
NAME
    sinkHfpDataSetAudioGMuted

DESCRIPTION
    Function to set audio data Mute state.

RETURNS
    void

*/
void sinkHfpDataSetAudioGMuted(bool g_muted, uint8 index)
{
    G_HFP_DATA.profile_data[index].audio.gMuted = g_muted;
}

/*************************************************************************
NAME
    sinkHfpDataGetAudioGMuted

DESCRIPTION
    Function to get audio data Mute state.

RETURNS
    bool
*/
bool sinkHfpDataGetAudioGMuted(uint8 index)
{
    return G_HFP_DATA.profile_data[index].audio.gMuted ? TRUE:FALSE;
}

/*************************************************************************
NAME
    sinkHfpDataSetProfileConnectedStatus

DESCRIPTION
    Function to set Hfp profiles connected status.

RETURNS
    void
*/
void sinkHfpDataSetProfileConnectedStatus(hfp_connection_status value, uint16 index)
{
    G_HFP_DATA.profile_data[index].status.connected = value;
}

/*************************************************************************
NAME
    sinkHfpDatacopyLinkData

DESCRIPTION
    Function to copy Hfp profile data from secondary into primary link.
RETURNS
    void

*/
void sinkHfpDatacopyLinkData(uint8 primary_index, uint8 secondary_index )
{
    G_HFP_DATA.profile_data[primary_index] = G_HFP_DATA.profile_data[secondary_index];
}

/*************************************************************************
NAME
    sinkHfpDataSetProfileListIdStatus

DESCRIPTION
    Function to set Hfp profiles List Id status.

RETURNS
    void

*/
void sinkHfpDataSetProfileListIdStatus(uint8 value, uint16 index)
{
    G_HFP_DATA.profile_data[index].status.list_id = value;
}

/*************************************************************************
NAME
    sinkHfpDataSetProfileLocalCallActionStatus

DESCRIPTION
    Function to set Hfp profiles local call action status.

RETURNS
    void

*/
void sinkHfpDataSetProfileLocalCallActionStatus(bool value, uint16 index)
{
    G_HFP_DATA.profile_data[index].status.local_call_action = value;
}

/*************************************************************************
NAME
    sinkHfpDataGetProfileConnectedStatus

DESCRIPTION
    Function to get Hfp profiles connected status.

RETURNS
    hfp_connection_status

*/
hfp_connection_status sinkHfpDataGetProfileStatusConnected(uint16 index)
{
    return G_HFP_DATA.profile_data[index].status.connected;
}

/*************************************************************************
NAME
    sinkHfpDataGetProfileStatusListId

DESCRIPTION
    Function to Get Hfp profiles List Id status.

RETURNS
    uint8

*/
uint8 sinkHfpDataGetProfileStatusListId(uint16 index)
{
    return G_HFP_DATA.profile_data[index].status.list_id;
}

/*************************************************************************
NAME
    sinkHfpDataGetProfileLocalCallActionStatus

DESCRIPTION
    Function to Get Hfp profiles local call action status.

RETURNS
    bool

*/
bool sinkHfpDataGetProfileStatusLocalCallAction(uint16 index)
{
    return G_HFP_DATA.profile_data[index].status.local_call_action ? TRUE:FALSE;
}

/*************************************************************************
NAME
    sinkHfpDataSetNetworkPresent

DESCRIPTION
    Interface function to set Network status
*************************************************************************/
void sinkHfpDataSetNetworkPresent(bool status)
{
    G_HFP_DATA.NetworkIsPresent = status;
}

/*************************************************************************
NAME
    sinkHfpDataSetNetworkPresent

DESCRIPTION
    Interface function to get Network status
*************************************************************************/
bool sinkHfpDataIsNetworkPresent(void)
{
    return G_HFP_DATA.NetworkIsPresent ? TRUE:FALSE;
}

/*************************************************************************
NAME
    sinkHfpDataSetLastOutgoingAg

DESCRIPTION
    Interface function to set last_outgoing_ag
*************************************************************************/
void sinkHfpDataSetLastOutgoingAg(hfp_link_priority link_priority)
{
    G_HFP_DATA.last_outgoing_ag = link_priority;
}

/*************************************************************************
NAME
    sinkHfpDataGetLastOutgoingAg

DESCRIPTION
    Interface function to get last_outgoing_ag
*************************************************************************/
uint8 sinkHfpDataGetLastOutgoingAg(void)
{
    return G_HFP_DATA.last_outgoing_ag;
}

/*************************************************************************
NAME
    sinkHfpDataSetRepeatCallerIDFlag

DESCRIPTION
    Interface function to set RepeatCallerIDFlag
*************************************************************************/
void sinkHfpDataSetRepeatCallerIDFlag(bool status)
{
    G_HFP_DATA.RepeatCallerIDFlag = status;
}

/*************************************************************************
NAME
    sinkHfpDataGetRepeatCallerIDFlag

DESCRIPTION
    Interface function to get RepeatCallerIDFlag
*************************************************************************/
bool sinkHfpDataGetRepeatCallerIDFlag(void)
{
    return G_HFP_DATA.RepeatCallerIDFlag ? TRUE:FALSE;
}

/*************************************************************************
NAME
    sinkHfpDataSetHeldCallIndex

DESCRIPTION
    Interface function to set HeldCallIndex
*************************************************************************/
void sinkHfpDataSetHeldCallIndex(uint8 index)
{
    G_HFP_DATA.HeldCallIndex = index;
}

/*************************************************************************
NAME
    sinkHfpDataGetHeldCallIndex

DESCRIPTION
    Interface function to get HeldCallIndex
*************************************************************************/
uint8 sinkHfpDataGetHeldCallIndex(void)
{
    return G_HFP_DATA.HeldCallIndex ;
}

/*************************************************************************
NAME
    sinkHfpDataSetMissedCallIndicated

DESCRIPTION
    Interface function to set MissedCallIndicated
*************************************************************************/
void sinkHfpDataSetMissedCallIndicated(uint16 value)
{
    G_HFP_DATA.MissedCallIndicated = value;
}

/*************************************************************************
NAME
    sinkHfpDataGetMissedCallIndicated

DESCRIPTION
    Interface function to get MissedCallIndicated
*************************************************************************/
uint8 sinkHfpDataGetMissedCallIndicated(void)
{
     return G_HFP_DATA.MissedCallIndicated ;
}

/*************************************************************************
NAME
    sinkHfpDataSetLinkLossReminderTime

DESCRIPTION
    Interface function to set linkLossReminderTime
*************************************************************************/
void sinkHfpDataSetLinkLossReminderTime(uint32 link_loss_interval_time)
{
    G_HFP_DATA.linkLossReminderTime = link_loss_interval_time;
}

/*************************************************************************
NAME
    sinkHfpDataGetLinkLossReminderTime

DESCRIPTION
    Interface function to get linkLossReminderTime
*************************************************************************/
uint8 sinkHfpDataGetLinkLossReminderTime(void)
{
    return G_HFP_DATA.linkLossReminderTime;
}

#ifdef WBS_TEST
/*************************************************************************
NAME
    sinkHfpDataSetFailAudioNegotiation

DESCRIPTION
    Interface function to set FailAudioNegotiation
*************************************************************************/
void sinkHfpDataSetFailAudioNegotiation(bool status)
{
    G_HFP_DATA.FailAudioNegotiation = status;
}

/*************************************************************************
NAME
    sinkHfpDataSetRenegotiateSco

DESCRIPTION
    Interface function to set RenegotiateSco
*************************************************************************/
void sinkHfpDataSetRenegotiateSco(bool status)
{
    G_HFP_DATA.RenegotiateSco = status;
}

/*************************************************************************
NAME
    sinkHfpDataIsFailAudioNegotiation

DESCRIPTION
    Interface function to get FailAudioNegotiation
*************************************************************************/
bool sinkHfpDataIsFailAudioNegotiation(void)
{
    return G_HFP_DATA.FailAudioNegotiation;
}

/*************************************************************************
NAME
    sinkHfpDataIsRenegotiateSco

DESCRIPTION
    Interface function to get RenegotiateSco
*************************************************************************/
bool sinkHfpDataIsRenegotiateSco(void)
{
    return G_HFP_DATA.RenegotiateSco ? TRUE:FALSE;
}
#endif

#if defined(TEST_HF_INDICATORS)
/*************************************************************************
NAME
    sinkHfpDataSetHfEnhancedSafety

DESCRIPTION
    Interface function to set hf_enhancedSafety
*************************************************************************/
void sinkHfpDataSetHfEnhancedSafety(bool status)
{
    G_HFP_DATA.hf_enhancedSafety = status;
}

/*************************************************************************
NAME
    sinkHfpDataGetHfEnhancedSafety

DESCRIPTION
    Interface function to get hf_enhancedSafety
*************************************************************************/
bool sinkHfpDataGetHfEnhancedSafety(void)
{
    return G_HFP_DATA.hf_enhancedSafety ? TRUE:FALSE;
}
#endif

