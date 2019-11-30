/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_plugin_music_variants.h
 
DESCRIPTION
    Definitions of music plugin variants.
*/

#ifndef AUDIO_PLUGIN_MUSIC_VARIANTS_H_
#define AUDIO_PLUGIN_MUSIC_VARIANTS_H_

#include <message.h>

typedef enum
{
     SBC_DECODER                = 1
    ,MP3_DECODER                = 2
    ,AAC_DECODER                = 3
    ,FASTSTREAM_SINK            = 4
    ,USB_DECODER                = 5
    ,APTX_DECODER               = 6
    ,APTX_ACL_SPRINT_DECODER    = 7
    ,ANALOG_DECODER             = 8
    ,SPDIF_DECODER              = 9
    ,TWS_SBC_DECODER            = 10
    ,TWS_MP3_DECODER            = 11
    ,TWS_AAC_DECODER            = 12
    ,TWS_APTX_DECODER           = 13
    ,FM_DECODER                 = 14
    ,I2S_DECODER                = 15
    ,APTXHD_DECODER             = 16
    ,BA_CELT_DECODER            = 17
    ,BA_SBC_DECODER             = 18
    ,BA_AAC_DECODER             = 19
    ,BA_ANALOGUE_DECODER        = 20
    ,BA_USB_DECODER             = 21
    ,APTX_AD_DECODER            = 22
    ,TWS_APTX_AD_DECODER        = 23
    ,NUM_DECODER_PLUGINS
}A2DP_DECODER_PLUGIN_TYPE_T;

typedef struct
{
    TaskData                    data;
    A2DP_DECODER_PLUGIN_TYPE_T  a2dp_plugin_variant:8 ; /* Selects the A2DP plugin variant */
    unsigned                    reserved:8 ;           /* Set the reserved bits to zero */
}A2dpPluginTaskdata;

extern const A2dpPluginTaskdata csr_sbc_decoder_plugin;
extern const A2dpPluginTaskdata csr_mp3_decoder_plugin;
extern const A2dpPluginTaskdata csr_aac_decoder_plugin;
extern const A2dpPluginTaskdata csr_faststream_sink_plugin;
extern const A2dpPluginTaskdata csr_aptx_decoder_plugin;
extern const A2dpPluginTaskdata csr_aptx_acl_sprint_decoder_plugin;
extern const A2dpPluginTaskdata csr_aptxhd_decoder_plugin;
extern const A2dpPluginTaskdata csr_aptx_ad_decoder_plugin;

extern const A2dpPluginTaskdata csr_tws_master_sbc_decoder_plugin;
extern const A2dpPluginTaskdata csr_tws_slave_sbc_decoder_plugin;
extern const A2dpPluginTaskdata csr_tws_master_mp3_decoder_plugin;
extern const A2dpPluginTaskdata csr_tws_slave_mp3_decoder_plugin;
extern const A2dpPluginTaskdata csr_tws_master_aac_decoder_plugin;
extern const A2dpPluginTaskdata csr_tws_slave_aac_decoder_plugin;
extern const A2dpPluginTaskdata csr_tws_master_aptx_decoder_plugin;
extern const A2dpPluginTaskdata csr_tws_slave_aptx_decoder_plugin;
extern const A2dpPluginTaskdata csr_tws_master_aptx_ad_decoder_plugin;
extern const A2dpPluginTaskdata csr_tws_slave_aptx_ad_decoder_plugin;

extern const A2dpPluginTaskdata csr_ba_receiver_decoder_plugin;
extern const A2dpPluginTaskdata csr_ba_sbc_decoder_plugin;
extern const A2dpPluginTaskdata csr_ba_aac_decoder_plugin;
extern const A2dpPluginTaskdata csr_ba_usb_decoder_plugin;
extern const A2dpPluginTaskdata csr_ba_analogue_decoder_plugin;

extern const A2dpPluginTaskdata csr_fm_decoder_plugin;
extern const A2dpPluginTaskdata csr_analogue_decoder_plugin;
extern const A2dpPluginTaskdata csr_i2s_decoder_plugin;
extern const A2dpPluginTaskdata csr_spdif_decoder_plugin;
extern const A2dpPluginTaskdata csr_usb_decoder_plugin;

bool AudioPluginMusicVariantsCodecDeterminesTwsEncoding(void);

#endif /* AUDIO_PLUGIN_MUSIC_VARIANTS_H_ */
