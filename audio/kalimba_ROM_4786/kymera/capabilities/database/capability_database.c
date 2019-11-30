/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  capability_database.c
 * \ingroup  database
 *
 * Capability database file. <br>
 * This file contains the capability data definitions and other static data for each capability. <br>
 */

#include "opmgr/opmgr_for_ops.h"
#ifdef INSTALL_CAP_DOWNLOAD_MGR
#include "opmgr/opmgr_for_cap_download.h"
#endif
#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
#include "kip_mgr/kip_mgr.h"
#endif
#include "hal/hal_dm_sections.h"

#ifdef INSTALL_OPERATOR_BASIC_PASS
#include "basic_passthrough/basic_passthrough.h"
#endif
#ifdef INSTALL_SCO
#include "sco_nb/sco_nb.h"
#include "wbs/wbs.h"
#endif
#ifdef INSTALL_OPERATOR_CVC_RECEIVE
#include "cvc_receive/cvc_receive.h"
#endif
#ifdef INSTALL_OPERATOR_RESAMPLER
#include "resampler/resampler_cap.h"
#endif
#ifdef INSTALL_OPERATOR_AEC_REFERENCE
#include "aec_reference/aec_reference.h"
#endif
#ifdef INSTALL_OPERATOR_SPLITTER
#include "splitter/splitter.h"
#endif
#if defined(INSTALL_OPERATOR_MIXER)       || \
    defined(INSTALL_OPERATOR_CHANNEL_MIXER)
#include "mixer/mixer.h"
#endif
#ifdef INSTALL_OPERATOR_PEQ
#include "peq/peq_cap.h"
#endif
#ifdef INSTALL_OPERATOR_VOLUME
#include "volume_control/volume_control.h"
#endif
#ifdef INSTALL_OPERATOR_DBE
#include "dbe/dbe_cap.h"
#endif
#ifdef INSTALL_OPERATOR_VSE
#include "vse/vse_cap.h"
#endif
#ifdef INSTALL_OPERATOR_XOVER
#include "xover/xover_cap.h"
#endif
#ifdef INSTALL_OPERATOR_COMPANDER
#include "compander/compander_cap.h"
#endif
#ifdef INSTALL_OPERATOR_DELAY
#include "delay/delay_cap.h"
#endif
#if defined INSTALL_OPERATOR_RINGTONE_GENERATOR
#include "ringtone_generator/ringtone_generator.h"
#endif
#if defined(INSTALL_OPERATOR_SBC_DECODE)       || \
    defined(INSTALL_OPERATOR_SBC_SHUNT_DECODER)
#include "sbc_decode/sbc_decode.h"
#endif
#ifdef INSTALL_OPERATOR_SBC_ENCODE
#include "sbc_encode/sbc_encode.h"
#endif
#if defined(INSTALL_OPERATOR_CELT_DECODE)
#include "celt_decode/celt_decode.h"
#endif
#ifdef INSTALL_OPERATOR_CELT_ENCODE
#include "celt_encode/celt_encode.h"
#endif

#if defined(INSTALL_OPERATOR_APTX_CLASSIC_DECODE)      || \
    defined(INSTALL_OPERATOR_APTX_LOW_LATENCY_DECODE)      || \
    defined(INSTALL_OPERATOR_APTX_SHUNT_DECODER)    || \
    defined(INSTALL_OPERATOR_APTXHD_DECODE)      || \
    defined(INSTALL_OPERATOR_APTXHD_SHUNT_DECODER)||  \
    defined(INSTALL_OPERATOR_APTX_CLASSIC_MONO_DECODER)      || \
    defined(INSTALL_OPERATOR_APTXHD_MONO_DECODER) ||\
    defined(INSTALL_OPERATOR_APTX_CLASSIC_MONO_DECODER_NO_AUTOSYNC) ||\
    defined(INSTALL_OPERATOR_APTXHD_MONO_DECODER_NO_AUTOSYNC)
    #include "aptx_decode/aptx_decode.h"
#endif

#if defined(INSTALL_OPERATOR_APTX_CLASSIC_ENCODE) || \
    defined(INSTALL_OPERATOR_ATPX_LOW_LATENCY_ENCODE) || \
    defined(INSTALL_OPERATOR_APTXHD_ENCODE)
#include "aptx_encode/aptx_encode.h"
#endif


#if defined(INSTALL_OPERATOR_AAC_DECODE)      || \
    defined(INSTALL_OPERATOR_AAC_SHUNT_DECODER)
#include "aac_decode/aac_decode.h"
#endif
#if defined(INSTALL_OPERATOR_CVC_HEADSET_1MIC)     || \
    defined(INSTALL_OPERATOR_CVC_HEADSET_2MIC_MONO)     || \
    defined(INSTALL_OPERATOR_CVC_HEADSET_BINAURAL) || \
    defined(INSTALL_OPERATOR_CVC_AUTO_1MIC)        || \
    defined(INSTALL_OPERATOR_CVC_AUTO_2MIC)        || \
    defined(INSTALL_OPERATOR_CVC_SPEAKER_1MIC)     || \
    defined(INSTALL_OPERATOR_CVC_SPEAKER_2MIC)     || \
    defined(INSTALL_OPERATOR_CVC_SPEAKER_3MIC)     || \
    defined(INSTALL_OPERATOR_CVC_SPEAKER_4MIC)
#include "cvc_send/cvc_send.h"
#endif

#ifdef INSTALL_CAP_DOWNLOAD_MGR
/* capability download database */
#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
DM_SHARED_ZI DOWNLOAD_CAP_DATA_DB* cap_download_data_list_shared;
#else
DOWNLOAD_CAP_DATA_DB* cap_download_data_list_aux;
#endif
DOWNLOAD_CAP_DATA_DB** cap_download_data_list;
#endif

#ifdef INSTALL_SPDIF
#include "spdif_decode/spdif_decode.h"
#endif

#ifdef INSTALL_OPERATOR_INTERLEAVE
#include "interleave/interleave.h"
#endif

#ifdef INSTALL_OPERATOR_USB_AUDIO
#include "usb_audio/usb_audio.h"
#endif

#ifdef INSTALL_OPERATOR_IIR_RESAMPLER
#include "iir_resampler/iir_resampler.h"
#endif

#ifdef INSTALL_OPERATOR_VAD
    #include "vad/vad.h"
#endif

#ifdef INSTALL_OPERATOR_RTP_DECODE
#include "rtp_decode/rtp_decode.h"
#endif

#ifdef INSTALL_OPERATOR_SOURCE_SYNC
#include "source_sync.h"
#endif
#ifdef INSTALL_OPERATOR_TEST_STALL_DROP
#include "test_stall_drop/test_stall_drop.h"
#endif

#ifdef INSTALL_OPERATOR_STUB_CAPABILITY
#include "stub_capability/stub_capability.h"
#endif

#ifdef INSTALL_OPERATOR_TEST_CONSUMER
#include "test_consumer/test_consumer.h"
#endif

#ifdef INSTALL_OPERATOR_SVA
#include "sva/sva.h"
#endif

#ifdef INSTALL_OPERATOR_CVSD
#include "cvsd/cvsd.h"
#endif

#ifdef INSTALL_OPERATOR_CVSD_LOOPBACK
#include "cvsd_loopback/cvsd_loopback.h"
#endif
#include "patch/patch.h"

/* Capability database */
const CAPABILITY_DATA* const capability_data_table[] = {
#ifdef INSTALL_OPERATOR_BASIC_PASS
    &basic_passthrough_cap_data,
#endif

#ifdef INSTALL_OPERATOR_STUB_CAPABILITY
   /* This is the stub used as a baseline for development and is not included
    * in standard builds. When used as a baseline for development it should be
    * duplicated and renamed. */
   &stub_capability_cap_data,
#endif

#ifdef INSTALL_OPERATOR_CVSD
    &cvsd_cap_data,
#endif

#ifdef INSTALL_OPERATOR_CVSD_LOOPBACK
    &cvsd_loopback_cap_data,
#endif

#ifdef INSTALL_OPERATOR_SCO_SEND
    &sco_send_cap_data,
#endif

#ifdef INSTALL_OPERATOR_SCO_RCV
    &sco_rcv_cap_data,
#endif

#ifdef INSTALL_OPERATOR_WBS_ENC
    &wbs_enc_cap_data,
#endif

#ifdef INSTALL_OPERATOR_WBS_DEC
    &wbs_dec_cap_data,
#endif

#ifdef INSTALL_OPERATOR_SBC_DECODE
    &sbc_decode_cap_data,
#endif

#ifdef INSTALL_OPERATOR_SBC_SHUNT_DECODER
    &sbc_a2dp_decoder_cap_data,
#endif

#ifdef INSTALL_OPERATOR_SBC_ENCODE
    &sbc_encode_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CELT_DECODE
    &celt_decode_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CELT_ENCODE
    &celt_encode_cap_data,
#endif
#ifdef INSTALL_OPERATOR_AAC_DECODE
    &aac_decode_cap_data,
#endif

#ifdef INSTALL_OPERATOR_AAC_SHUNT_DECODER
    &aac_a2dp_decoder_cap_data,
#endif
#if defined (INSTALL_OPERATOR_APTX_CLASSIC_DECODE)
    &aptx_classic_decode_cap_data,
#endif
#if defined (INSTALL_OPERATOR_APTX_LOW_LATENCY_DECODE)
    &aptx_low_latency_decode_cap_data,
#endif

#ifdef INSTALL_OPERATOR_APTX_SHUNT_DECODER
    &aptx_a2dp_decoder_cap_data,
#endif

#if defined (INSTALL_OPERATOR_APTXHD_DECODE)
    &aptxhd_decode_cap_data,
#endif

#ifdef INSTALL_OPERATOR_APTXHD_SHUNT_DECODER
    &aptxhd_a2dp_decoder_cap_data,
#endif

#ifdef INSTALL_OPERATOR_APTX_CLASSIC_ENCODE
    &aptx_classic_encode_cap_data,
#endif

#ifdef INSTALL_OPERATOR_APTXHD_ENCODE
    &aptx_hd_encode_cap_data,
#endif

#ifdef INSTALL_OPERATOR_APTX_LOW_LATENCY_ENCODE
    &aptx_low_latency_encode_cap_data,
#endif

#ifdef INSTALL_OPERATOR_APTX_CLASSIC_MONO_DECODER
    &aptx_classic_mono_decode_cap_data,
#endif

#ifdef INSTALL_OPERATOR_APTXHD_MONO_DECODER
    &aptxhd_mono_decode_cap_data,
#endif

#ifdef INSTALL_OPERATOR_APTX_CLASSIC_MONO_DECODER_NO_AUTOSYNC
    &aptx_classic_mono_decode_noasync_cap_data,
#endif    

#ifdef INSTALL_OPERATOR_APTXHD_MONO_DECODER_NO_AUTOSYNC
    &aptxhd_mono_decode_noasync_cap_data,
#endif    

#ifdef INSTALL_OPERATOR_RESAMPLER
    &resampler_cap_data,
#endif

#ifdef INSTALL_OPERATOR_SPLITTER
    &splitter_cap_data,
#endif
#ifdef INSTALL_OPERATOR_PEQ
   &peq_cap_data,
#endif
#ifdef INSTALL_OPERATOR_DBE
   &dbe_cap_data,
   &dbe_fullband_cap_data,
   &dbe_fullband_bassout_cap_data,
#endif
#ifdef INSTALL_OPERATOR_VSE
   &vse_cap_data,
#endif
#ifdef INSTALL_OPERATOR_COMPANDER
   &compander_cap_data,
#endif
#ifdef INSTALL_OPERATOR_XOVER
   &xover_cap_data,
#endif

#ifdef INSTALL_OPERATOR_CVC_RECEIVE
    &cvc_receive_nb_cap_data,
    &cvc_receive_wb_cap_data,
    &cvc_receive_fe_cap_data,
#ifdef INSTALL_OPERATOR_CVC_24K
    &cvc_receive_uwb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    &cvc_receive_swb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    &cvc_receive_fb_cap_data,
#endif
#endif

#ifdef INSTALL_OPERATOR_AEC_REFERENCE
    &aec_reference_cap_data,
#endif

#ifdef INSTALL_OPERATOR_CVC_HEADSET_1MIC
    &cvc_send_1mic_nb_hs_cap_data,
    &cvc_send_1mic_wb_hs_cap_data,
#ifdef INSTALL_OPERATOR_CVC_24K
    &cvc_send_1mic_uwb_hs_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    &cvc_send_1mic_swb_hs_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    &cvc_send_1mic_fb_hs_cap_data,
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_HEADSET_2MIC_MONO
    &cvc_send_2mic_hs_mono_nb_cap_data,
    &cvc_send_2mic_hs_mono_wb_cap_data,
#ifdef INSTALL_OPERATOR_CVC_24K
    &cvc_send_2mic_hs_mono_uwb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    &cvc_send_2mic_hs_mono_swb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    &cvc_send_2mic_hs_mono_fb_cap_data,
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_HEADSET_BINAURAL
    &cvc_send_2mic_hs_binaural_nb_cap_data,
    &cvc_send_2mic_hs_binaural_wb_cap_data,
#ifdef INSTALL_OPERATOR_CVC_24K
    &cvc_send_2mic_hs_binaural_uwb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    &cvc_send_2mic_hs_binaural_swb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    &cvc_send_2mic_hs_binaural_fb_cap_data,
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_AUTO_1MIC
    &cvc_send_1mic_nb_auto_cap_data,
    &cvc_send_1mic_wb_auto_cap_data,
#ifdef INSTALL_OPERATOR_CVC_24K
    &cvc_send_1mic_uwb_auto_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    &cvc_send_1mic_swb_auto_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    &cvc_send_1mic_fb_auto_cap_data,
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_AUTO_2MIC
    &cvc_send_2mic_nb_auto_cap_data,
    &cvc_send_2mic_wb_auto_cap_data,
#ifdef INSTALL_OPERATOR_CVC_24K
    &cvc_send_2mic_uwb_auto_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    &cvc_send_2mic_swb_auto_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    &cvc_send_2mic_fb_auto_cap_data,
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_1MIC
    &cvc_send_1mic_speaker_nb_cap_data,
    &cvc_send_1mic_speaker_wb_cap_data,
#ifdef INSTALL_OPERATOR_CVC_24K
    &cvc_send_1mic_speaker_uwb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    &cvc_send_1mic_speaker_swb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    &cvc_send_1mic_speaker_fb_cap_data,
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_2MIC
    &cvc_send_2mic_speaker_nb_cap_data,
    &cvc_send_2mic_speaker_wb_cap_data,
#ifdef INSTALL_OPERATOR_CVC_24K
    &cvc_send_2mic_speaker_uwb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    &cvc_send_2mic_speaker_swb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    &cvc_send_2mic_speaker_fb_cap_data,
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_3MIC
    &cvc_send_3mic_speaker_nb_cap_data,
    &cvc_send_3mic_speaker_wb_cap_data,
#ifdef INSTALL_OPERATOR_CVC_24K
    &cvc_send_3mic_speaker_uwb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    &cvc_send_3mic_speaker_swb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    &cvc_send_3mic_speaker_fb_cap_data,
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_4MIC
    &cvc_send_4mic_speaker_nb_cap_data,
    &cvc_send_4mic_speaker_wb_cap_data,
#ifdef INSTALL_OPERATOR_CVC_24K
    &cvc_send_4mic_speaker_uwb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    &cvc_send_4mic_speaker_swb_cap_data,
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    &cvc_send_4mic_speaker_fb_cap_data,
#endif
#endif

#ifdef INSTALL_OPERATOR_VOLUME
   &vol_ctlr_cap_data,
#endif

#ifdef INSTALL_OPERATOR_DELAY
    &delay_cap_data,
#endif

#ifdef INSTALL_OPERATOR_MIXER
    &mixer_cap_data,
#endif

#ifdef INSTALL_OPERATOR_CHANNEL_MIXER
    &channel_mixer_cap_data,
#endif

#ifdef INSTALL_SPDIF
    &spdif_decode_cap_data,
#endif

#ifdef INSTALL_OPERATOR_INTERLEAVE
     &interleave_cap_data,
     &deinterleave_cap_data,
#endif

#ifdef INSTALL_OPERATOR_USB_AUDIO
     &usb_audio_rx_cap_data,
     &usb_audio_tx_cap_data,
#endif

#ifdef INSTALL_OPERATOR_IIR_RESAMPLER
    &iir_resampler_cap_data,
#endif

#ifdef INSTALL_OPERATOR_RINGTONE_GENERATOR
    &ringtone_generator_cap_data,
#endif

#ifdef INSTALL_OPERATOR_SVA
    &sva_cap_data,
#endif

#ifdef INSTALL_OPERATOR_SOURCE_SYNC
   &source_sync_cap_data,
#endif
#ifdef INSTALL_OPERATOR_RTP_DECODE
    &rtp_decode_cap_data,
#endif

#ifdef INSTALL_OPERATOR_TTP_PASS
    &ttp_passthrough_cap_data,
#endif

#ifdef INSTALL_OPERATOR_TEST_STALL_DROP
    &test_stall_drop_cap_data,
#endif

#ifdef INSTALL_OPERATOR_VAD
    &vad_cap_data,
#endif

#ifdef INSTALL_OPERATOR_TEST_CONSUMER
    &test_consumer_cap_data,
#endif
   /* NULL terminator for the table */
   NULL
};

/****************************************************************************
Private Function Definitions
*/
#ifdef INSTALL_CAP_DOWNLOAD_MGR

void capability_database_init_download_list(void);

/****************************************************************************
 *
 * capability_database_init_download_list
 *
 */
void capability_database_init_download_list(void)
{
    patch_fn(capability_database_init_download_list);
    
#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
    if(!KIP_PRIMARY_CONTEXT())
    {
        if(ipc_get_lookup_addr(IPC_LUT_ID_DATABASE_DOWNLOAD_LIST,
                              (uintptr_t*)(&cap_download_data_list)) != IPC_SUCCESS)
        {
            panic_diatribe(PANIC_IPC_LUT_CORRUPTED, IPC_LUT_ID_DATABASE_DOWNLOAD_LIST);
        }
    }
    else
    {
        cap_download_data_list = &cap_download_data_list_shared;
        /* Initialise list to null */
        *cap_download_data_list = NULL;
        if(ipc_set_lookup_addr(IPC_LUT_ID_DATABASE_DOWNLOAD_LIST,
                           (uintptr_t)cap_download_data_list) != IPC_SUCCESS )
        {
            panic_diatribe(PANIC_IPC_LUT_CORRUPTED, IPC_LUT_ID_DATABASE_DOWNLOAD_LIST);
        }
    }
#else
    cap_download_data_list = &cap_download_data_list_aux;
    /* Initialise list to null */
    *cap_download_data_list = NULL;
#endif /* INSTALL_DUAL_CORE_SUPPORT && AUDIO_SECOND_CORE */
}
#endif /* INSTALL_CAP_DOWNLOAD_MGR */
