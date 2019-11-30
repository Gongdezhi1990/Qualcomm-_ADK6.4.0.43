/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.
Part of Headset-ADK 1.1

FILE NAME
    a2dp_codec_csr_aptx_acl_sprint.c

DESCRIPTION
    This file contains

NOTES

*/

/****************************************************************************
    Header files
*/
#include "a2dp.h"
#include "a2dp_private.h"
#include "a2dp_caps_parse.h"
#include "a2dp_codec_csr_aptx_acl_sprint.h"

#ifndef A2DP_SBC_ONLY

/**************************************************************************/
void selectOptimalCsrAptxAclSprintCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
	/* Choose what is supported at both sides */
    remote_codec_caps[10] = (remote_codec_caps[10]) & (local_codec_caps[10]);
    remote_codec_caps[11] = (remote_codec_caps[11]) & (local_codec_caps[11]);
    
    /* Select sample frequency */
    if (remote_codec_caps[10] & 0x20)
    {   /* choose 44k1 */
        remote_codec_caps[10] &= 0x2f;
    }
    else
    {   /* choose 48k - aptX only supports 44.1Khz and 48Khz */
        remote_codec_caps[10] &= 0x1f;
    }
    
    /* Choose stereo */
    remote_codec_caps[10] &= 0xf2;

    /* Choose bi-directional link and new caps*/
    remote_codec_caps[11] &= 0x03;
}


/**************************************************************************/
void selectOptimalCsrAptxAclSprintCapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
   	/* Choose what is supported at both sides */
    remote_codec_caps[10] = (remote_codec_caps[10]) & (local_codec_caps[10]);
    remote_codec_caps[11] = (remote_codec_caps[11]) & (local_codec_caps[11]);
    
    /* Select sample frequency */
    if (remote_codec_caps[10] & 0x10)
    {   /* choose 48k */
        remote_codec_caps[10] &= 0x1f;
    }
    else
    {   /* choose 44k1 - aptX only supports 44.1Khz and 48Khz*/
        remote_codec_caps[10] &= 0x2f;
    }

    /*Choose stereo */
    remote_codec_caps[10] &= 0xf2;
	
	/* Choose bi-directional link and new caps */
    remote_codec_caps[11] &= 0x03;
        
    /* only try copying more data if the far end is using new caps
     * otherwise we'll run off the end of our array */
    if (remote_codec_caps[11] & 0x02)
    {
        /* Choose default values for the new aptx ll caps */
        remote_codec_caps[13] = A2DP_APTX_LOWLATENCY_TCL_LSB;
        remote_codec_caps[14] = A2DP_APTX_LOWLATENCY_TCL_MSB;
        remote_codec_caps[15] = A2DP_APTX_LOWLATENCY_ICL_LSB;
        remote_codec_caps[16] = A2DP_APTX_LOWLATENCY_ICL_MSB;
        remote_codec_caps[17] = A2DP_APTX_LOWLATENCY_MAX_RATE;
        remote_codec_caps[18] = A2DP_APTX_LOWLATENCY_AVG_TIME;
        remote_codec_caps[19] = A2DP_APTX_LOWLATENCY_GWBL_LSB;
        remote_codec_caps[20] = A2DP_APTX_LOWLATENCY_GWBL_MSB;
    }
}

/*************************************************************************/
void getCsrAptxAclSprintConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings)
{
    if (!service_caps)
    {
        codec_settings->rate = 44100;
        codec_settings->channel_mode = a2dp_stereo;
        codec_settings->codecData.packet_size = 289;
        return;
    }
    
    /* Work out the sample rate based on the codec configuration. */
    if (service_caps[10] & 0x10)
    {
        codec_settings->rate = 48000;
        codec_settings->codecData.packet_size = 313;
    }
    else
    { /* aptX only supports 44.1Khz and 48Khz */
        codec_settings->rate = 44100;
        codec_settings->codecData.packet_size = 289;
    }
    
    codec_settings->channel_mode = a2dp_stereo;
	
	if (service_caps[11] & 0x01)
	{
        codec_settings->codecData.voice_rate = 16000;
	}
    else
	{
        codec_settings->codecData.voice_rate = 0;
	}

    
	if (service_caps[11] & 0x02)
	{
		codec_settings->codecData.aptx_sprint_params.target_codec_level = service_caps[13] | (service_caps[14] << 8) ;
		codec_settings->codecData.aptx_sprint_params.initial_codec_level = service_caps[15] | (service_caps[16] << 8) ;
		codec_settings->codecData.aptx_sprint_params.sra_max_rate = service_caps[17];
		codec_settings->codecData.aptx_sprint_params.sra_avg_time = service_caps[18];
		codec_settings->codecData.aptx_sprint_params.good_working_level = service_caps[19] | (service_caps[20] << 8) ;
	}
	else
	{	/* Legacy mode - default values */
		codec_settings->codecData.aptx_sprint_params.target_codec_level = 180;
		codec_settings->codecData.aptx_sprint_params.initial_codec_level = 360;
		codec_settings->codecData.aptx_sprint_params.sra_max_rate = 50;
		codec_settings->codecData.aptx_sprint_params.sra_avg_time = 1;
		codec_settings->codecData.aptx_sprint_params.good_working_level = 180;
	}
}

#endif /* A2DP_SBC_ONLY */
