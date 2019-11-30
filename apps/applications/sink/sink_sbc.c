/*
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
\brief   Set the SBC encoder configuration parameters. 
          This is currently used only for PTS conformance
*/

#ifdef SBC_ENCODER_CONFORMANCE

#include <stdio.h>

#include <config_store.h>

#include "sink_configmanager.h"
#include "sink_sbc.h"
#include "sink_sbc_config_def.h"
#include "sink_a2dp.h"
#include "sink_debug.h"

#ifdef DEBUG_SBC_CONF
#define SBC_CONF_DEBUG(x) DEBUG(x)
#else
#define SBC_CONF_DEBUG(x)
#endif

/**************************************************************************/
/* SBC configuration bit fields */

/*Sampling frequency */
#define SBC_SAMPLING_FREQ_SHIFT         (0x06)
#define SBC_16KHZ_MASK                  (0x00)
#define SBC_32KHZ_MASK                  (0x01)
#define SBC_441KHZ_MASK                 (0x02)
#define SBC_48KHZ_MASK                  (0x03)


#define SBC_SAMPLING_FREQ_16000         128
#define SBC_SAMPLING_FREQ_32000         64
#define SBC_SAMPLING_FREQ_44100         32
#define SBC_SAMPLING_FREQ_48000         16

/*Channel Mode*/
#define SBC_CHANNEL_MODE_SHIFT          (0x02)
#define SBC_CHANNEL_MODE_MONO           8
#define SBC_CHANNEL_MODE_DUAL_CHAN      4
#define SBC_CHANNEL_MODE_STEREO         2
#define SBC_CHANNEL_MODE_JOINT_STEREO   1

/*Block Length */
#define SBC_BLOCK_LENGTH_SHIFT          (0x04)
#define SBC_BLK_LEN4_MASK               (0x00)
#define SBC_BLK_LEN8_MASK               (0x01)
#define SBC_BLK_LEN12_MASK              (0x02)
#define SBC_BLK_LEN16_MASK              (0x03)

#define SBC_BLOCK_LENGTH_4              128
#define SBC_BLOCK_LENGTH_8              64
#define SBC_BLOCK_LENGTH_12             32
#define SBC_BLOCK_LENGTH_16             16

/*Subbands */
#define SBC_SUB_BANDS_MASK              (0x04)
#define SBC_SUB_BANDS_FORMAT8           (0x01)
#define SBC_SUB_BANDS_FORMAT4           (0x00)
#define SBC_SUBBANDS_4                  8
#define SBC_SUBBANDS_8                  4


/*Allocation Method*/
#define SBC_ALLOC_METHOD_MASK           (0x02)
#define SBC_ALLOC_METHOD_SNR            (0x02)
#define SBC_ALLOC_MEHTOD_LOUDNESS       (0x01)


static uint8 sinkGetSamplingFreqFormat(void)
{
    uint8 format = 0;

    switch (SinkSbcGetSamplingFreq())
    {
        case SBC_SAMPLING_FREQ_16000:
            format = SBC_16KHZ_MASK << SBC_SAMPLING_FREQ_SHIFT;
            break;

        case SBC_SAMPLING_FREQ_32000:
            format = SBC_32KHZ_MASK << SBC_SAMPLING_FREQ_SHIFT;
            break;
            
        case SBC_SAMPLING_FREQ_44100:
            format = SBC_441KHZ_MASK << SBC_SAMPLING_FREQ_SHIFT;
            break;
            
        case SBC_SAMPLING_FREQ_48000:
            format = SBC_48KHZ_MASK << SBC_SAMPLING_FREQ_SHIFT;
            break;

        default:
            break;
    }

    return format;
}

static uint8 sinkGetChannelModeFormat(void)
{
    uint8 format = 0;

    switch (SinkSbcGetChannelMode())
    {
        case SBC_CHANNEL_MODE_MONO:
            format = ((uint8) a2dp_mono) << SBC_CHANNEL_MODE_SHIFT;
            break;

        case SBC_CHANNEL_MODE_DUAL_CHAN:
            format = ((uint8) a2dp_dual_channel) << SBC_CHANNEL_MODE_SHIFT;
            break;
            
        case SBC_CHANNEL_MODE_STEREO:
            format = ((uint8) a2dp_stereo) << SBC_CHANNEL_MODE_SHIFT;
            break;
            
        case SBC_CHANNEL_MODE_JOINT_STEREO:
            format =  ((uint8) a2dp_joint_stereo) << SBC_CHANNEL_MODE_SHIFT;
            break;

        default:
            break;
    }

    return format;
}

static uint8 sinkGetBlkLengthFormat(void)
{
    uint8 format = 0;

    switch (SinkSbcGetBlockLength())
    {
        case SBC_BLOCK_LENGTH_4:
            format = SBC_BLK_LEN4_MASK << SBC_BLOCK_LENGTH_SHIFT;
            break;

        case SBC_BLOCK_LENGTH_8:
            format =  SBC_BLK_LEN8_MASK << SBC_BLOCK_LENGTH_SHIFT;
            break;
            
        case SBC_BLOCK_LENGTH_12:
            format =  SBC_BLK_LEN12_MASK << SBC_BLOCK_LENGTH_SHIFT;
            break;
            
        case SBC_BLOCK_LENGTH_16:
            format =   SBC_BLK_LEN16_MASK << SBC_BLOCK_LENGTH_SHIFT;
            break;

        default:
            break;
    }

    return format;
}

static uint8 sinkGetSubbandsFormat(void)
{
    uint8 format = SBC_SUB_BANDS_FORMAT4;
    
    if (SinkSbcGetSubbands() & SBC_SUB_BANDS_MASK)
    {
        format = SBC_SUB_BANDS_FORMAT8;
    }
    return format;
}

static uint8 sinkGetAllocationMethodFormat(void)
{
    uint8 format = SBC_ALLOC_MEHTOD_LOUDNESS;
    
    if (SinkSbcGetAllocationMethod() & SBC_ALLOC_METHOD_MASK)
    {
        format = SBC_ALLOC_METHOD_SNR;
    }
    return format;
}

uint8 SinkSbcGetAudioConnectFormat(void)
{
    uint8 sbc_format=0;

    sbc_format = sinkGetSamplingFreqFormat() | sinkGetChannelModeFormat() |sinkGetBlkLengthFormat() |sinkGetSubbandsFormat() | sinkGetAllocationMethodFormat();
    
    SBC_CONF_DEBUG(("SinkSbcGetAudioConnectFormat sbc_format 0x%x \n", sbc_format));

    return sbc_format;
}

uint8 SinkSbcGetSamplingFreq(void)
{
    sink_sbc_read_only_config_def_t *config_data = NULL;
    uint8 sbc_freq = 0;

    if (configManagerGetReadOnlyConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID, (const void **)&config_data))
    {
        sbc_freq = config_data->sampling_freq;
        configManagerReleaseConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID);
    }

    SBC_CONF_DEBUG(("SBC Conformance sbc_freq 0x%x \n", sbc_freq));
    return sbc_freq;
}

uint8 SinkSbcGetChannelMode(void)
{
    sink_sbc_read_only_config_def_t *config_data = NULL;
    uint8 channel_mode = 0;

    if (configManagerGetReadOnlyConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID , (const void **)&config_data))
    {
        channel_mode = config_data->channel_mode;
        configManagerReleaseConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID);
    }
    
    SBC_CONF_DEBUG(("SBC Conformance channel_mode 0x%x \n", channel_mode));
    return channel_mode;
}

uint8 SinkSbcGetBlockLength(void)
{
    sink_sbc_read_only_config_def_t *config_data = NULL;
    uint8 block_length = 0;

    if (configManagerGetReadOnlyConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID, (const void **)&config_data))
    {
        block_length = config_data->block_length;
        configManagerReleaseConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID);
    }

    SBC_CONF_DEBUG(("SBC Conformance block_length 0x%x \n", block_length));
    return block_length;
}

uint8 SinkSbcGetSubbands(void)
{
    sink_sbc_read_only_config_def_t *config_data = NULL;
    uint8 subbands = 0;

    if (configManagerGetReadOnlyConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID, (const void **)&config_data))
    {
        subbands = config_data->subbands;
        configManagerReleaseConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID);
    }
    
    SBC_CONF_DEBUG(("SBC Conformance subbands 0x%x \n", subbands));
    return subbands;
}

uint8 SinkSbcGetAllocationMethod(void)
{
    sink_sbc_read_only_config_def_t *config_data = NULL;
    uint8 alloc_method = 0;

    if (configManagerGetReadOnlyConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID, (const void **)&config_data))
    {
        alloc_method = config_data->alloc_method;
        configManagerReleaseConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID);
    }

    SBC_CONF_DEBUG(("SBC Conformance alloc_method 0x%x \n", alloc_method));
    return alloc_method;
}

uint8 SinkSbcGetMinBitpool(void)
{
    sink_sbc_read_only_config_def_t *config_data;
    uint8 sbc_min_bitpool = 0;

    if (configManagerGetReadOnlyConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID, (const void **)&config_data))
    {
        sbc_min_bitpool = config_data->sbc_min_bitpool;
        configManagerReleaseConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID);
    }
    
    SBC_CONF_DEBUG(("SBC Conformance sbc_min_bitpool 0x%x \n", sbc_min_bitpool));
    return sbc_min_bitpool;
}

uint8 SinkSbcGetMaxBitpool(void)
{
    sink_sbc_read_only_config_def_t *config_data;
    uint8 sbc_max_bitpool = 0;

    if (configManagerGetReadOnlyConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID, (const void **)&config_data))
    {
        sbc_max_bitpool = config_data->sbc_max_bitpool;
    configManagerReleaseConfig(SINK_SBC_READ_ONLY_CONFIG_BLK_ID);
    }    

    SBC_CONF_DEBUG(("SBC Conformance sbc_max_bitpool 0x%x \n", sbc_max_bitpool));
    return sbc_max_bitpool;
}
#endif

