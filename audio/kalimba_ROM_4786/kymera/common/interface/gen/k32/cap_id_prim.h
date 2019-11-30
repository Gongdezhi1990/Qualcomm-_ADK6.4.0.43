/*****************************************************************************

            (c) Qualcomm Technologies International, Ltd. 2017
            Confidential information of Qualcomm

            Refer to LICENSE.txt included with this source for details
            on the license terms.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef CAP_ID_PRIM_H
#define CAP_ID_PRIM_H

#include <limits.h>


/*******************************************************************************

  NAME
    Cap_Id

  DESCRIPTION
    Defines for the Capability identifiers.

 VALUES
    NONE
                   - test basic_op capability
    BASIC_PASS
                   - test basic passthrough capability
    STEREO_PASS
                   - test stereo passthrough capability
    SCO_SEND
                   - SCO send operator
    SCO_RCV
                   - SCO receiver with packet loss concealment
    WBS_ENC
                   - Wideband Speech encoder
    WBS_DEC
                   - Wideband Speech decoder
    SBC_A2DP_ENCODER
                   - SBC Encoder for A2DP
    MP3_A2DP_ENCODER
                   - Configurable Mp3 encoder
    RESAMPLER
                   - Standard-frequency-set Resampler
    MIXER
                   - Stereo/Mono Audio Mixer
    APTX_A2DP_ENCODER
                   - High-Quality APT-X Audio Encoder
    FM_ENHANCEMENT
                   - FM Receive audio quality enhancement
    SBC_SHUNT_DECODER
                   - SBC decoder over A2DP / Shunt (i.e. with L2CAP headers)
    MP3_SHUNT_DECODER
                   - MP3 decoder over A2DP / Shunt (i.e. with L2CAP headers)
    AAC_SHUNT_DECODER
                   - AAC decoder over A2DP / Shunt (i.e. with L2CAP headers)
    APTX_SHUNT_DECODER
                   - APTX decoder over A2DP / Shunt (i.e. with L2CAP headers)
    RESERVED_0011
                   - The ringtone generator on BlueCore used to be called
                     RINGTONE_GEN with numeric id 0x11. Don't reuse this value.
    EQ_COMP
                   - Equaliser and Compressor, Limiter, Expander
    SPLITTER
                   - Mono audio splitter
    SBC_ENCODER
                   - SBC encoder - Raw
    MP3_ENCODER
                   - MP3 encoder - Raw
    SBC_DECODER
                   - SBC decoder - Raw data in
    MP3_DECODER
                   - MP3 raw decoder
    AAC_DECODER
                   - AAC raw decoder
    APTX_CLASSIC_DECODER
                   - aptx classic raw decoder
    CVC_RECEIVE_FE
                   - CVC 1 Mic receive
    CVCHF1MIC_SEND_NB
                   - CVC 1 Mic Automotive
    CVC_RECEIVE_NB
                   - CVC 1 Mic receive
    CVCHF1MIC_SEND_WB
                   - CVC 1 Mic Automotive
    CVC_RECEIVE_WB
                   - CVC 1 Mic receive
    CVCHF2MIC_SEND_NB
                   - CVC 2 Mic Automotive
    CVCHF2MIC_SEND_WB
                   - CVC 2 Mic Automotive
    CVCHS1MIC_SEND_NB
                   - CVC 1 Mic Headset
    CVCHS1MIC_SEND_WB
                   - CVC 1 Mic Headset
    CVCHS2MIC_MONO_SEND_NB
                   - CVC 2 Mic Headset endfire
    CVCHS2MIC_MONO_SEND_WB
                   - CVC 2 Mic Headset endfire
    CVCHS2MIC_BINAURAL_SEND_NB
                   - CVC 2 Mic Headset binaural
    CVCHS2MIC_BINAURAL_SEND_WB
                   - CVC 2 Mic Headset binaural
    CVCSPKR1MIC_SEND_NB
                   - CVC 1 Mic Speakerphone
    CVCSPKR1MIC_SEND_WB
                   - CVC 1 Mic Speakerphone
    CVCSPKR2MIC_SEND_NB
                   - CVC 2 Mic Speakerphone
    CVCSPKR2MIC_SEND_WB
                   - CVC 2 Mic Speakerphone
    DBE
                   - DBE
    XOVER
                   - XOVER
    DELAY
                   - DELAY
    SPDIF_DECODE
                   - S/PDIF RX driver
    RINGTONE_GENERATOR
                   - Ringtone Generator
    TTP_PASS
                   - Passthrough with time-to-play generation
    APTX_LOW_LATENCY_DECODER
                   - aptx low latency decoder
    INTERLEAVE
                   - Interleave operator
    DEINTERLEAVE
                   - Deinterleave operator
    AEC_REFERENCE
                   - AEC Front End
    CVCSPKR3MIC_SEND_NB
                   - CVC 3 Mic Speakerphone
    CVCSPKR3MIC_SEND_WB
                   - CVC 3 Mic Speakerphone
    CVCSPKR4MIC_SEND_NB
                   - CVC 4 Mic Speakerphone
    CVCSPKR4MIC_SEND_WB
                   - CVC 4 Mic Speakerphone
    VOL_CTRL_VOL
                   - Volume Control
    PEQ
                   - PEQ
    VSE
                   - VSE
    CVCHS3MIC_MONO_SEND_NB
                   - CVC 3 Mic Mono Headset (endfire)
    CVCHS3MIC_MONO_SEND_WB
                   - CVC 3 Mic Mono Headset (endfire)
    CVCHS3MIC_BINAURAL_SEND_NB
                   - CVC 3 Mic Binaural Headset
    CVCHS3MIC_BINAURAL_SEND_WB
                   - CVC 3 Mic Binaural Headset
    CVCSPKR3MIC_CIRC_SEND_NB
                   - CVC 3 Mic Speakerphone circular
    CVCSPKR3MIC_CIRC_SEND_WB
                   - CVC 3 Mic Speakerphone circular
    CVCSPKR4MIC_CIRC_SEND_NB
                   - CVC 4 Mic Speakerphone circular
    CVCSPKR4MIC_CIRC_SEND_WB
                   - CVC 4 Mic Speakerphone circular
    CVC_RECEIVE_UWB
                   - CVC 1 Mic receive
    CVC_RECEIVE_SWB
                   - CVC 1 Mic receive
    CVC_RECEIVE_FB
                   - CVC 1 Mic receive
    CVCHF1MIC_SEND_UWB
                   - CVC 1 Mic Automotive
    CVCHF1MIC_SEND_SWB
                   - CVC 1 Mic Automotive
    CVCHF1MIC_SEND_FB
                   - CVC 1 Mic Automotive
    CVCHF2MIC_SEND_UWB
                   - CVC 2 Mic Automotive
    CVCHF2MIC_SEND_SWB
                   - CVC 2 Mic Automotive
    CVCHF2MIC_SEND_FB
                   - CVC 2 Mic Automotive
    CVCHS1MIC_SEND_UWB
                   - CVC 1 Mic Headset
    CVCHS1MIC_SEND_SWB
                   - CVC 1 Mic Headset
    CVCHS1MIC_SEND_FB
                   - CVC 1 Mic Headset
    CVCHS2MIC_MONO_SEND_UWB
                   - CVC 2 Mic Headset endfire
    CVCHS2MIC_MONO_SEND_SWB
                   - CVC 2 Mic Headset endfire
    CVCHS2MIC_MONO_SEND_FB
                   - CVC 2 Mic Headset endfire
    CVCHS2MIC_BINAURAL_SEND_UWB
                   - CVC 2 Mic Headset binaural
    CVCHS2MIC_BINAURAL_SEND_SWB
                   - CVC 2 Mic Headset binaural
    CVCHS2MIC_BINAURAL_SEND_FB
                   - CVC 2 Mic Headset binaural
    CVCHS3MIC_MONO_SEND_UWB
                   - CVC 3 Mic Mono Headset (endfire)
    CVCHS3MIC_MONO_SEND_SWB
                   - CVC 3 Mic Mono Headset (endfire)
    CVCHS3MIC_MONO_SEND_FB
                   - CVC 3 Mic Mono Headset (endfire)
    CVCHS3MIC_BINAURAL_SEND_UWB
                   - CVC 3 Mic Binaural Headset
    CVCHS3MIC_BINAURAL_SEND_SWB
                   - CVC 3 Mic Binaural Headset
    CVCHS3MIC_BINAURAL_SEND_FB
                   - CVC 3 Mic Binaural Headset
    CVCSPKR1MIC_SEND_UWB
                   - CVC 1 Mic Speakerphone
    CVCSPKR1MIC_SEND_SWB
                   - CVC 1 Mic Speakerphone
    CVCSPKR1MIC_SEND_FB
                   - CVC 1 Mic Speakerphone
    CVCSPKR2MIC_SEND_UWB
                   - CVC 2 Mic Speakerphone
    CVCSPKR2MIC_SEND_SWB
                   - CVC 2 Mic Speakerphone
    CVCSPKR2MIC_SEND_FB
                   - CVC 2 Mic Speakerphone
    CVCSPKR3MIC_SEND_UWB
                   - CVC 3 Mic Speakerphone
    CVCSPKR3MIC_SEND_SWB
                   - CVC 3 Mic Speakerphone
    CVCSPKR3MIC_SEND_FB
                   - CVC 3 Mic Speakerphone
    CVCSPKR4MIC_SEND_UWB
                   - CVC 4 Mic Speakerphone
    CVCSPKR4MIC_SEND_SWB
                   - CVC 4 Mic Speakerphone
    CVCSPKR4MIC_SEND_FB
                   - CVC 4 Mic Speakerphone
    CVCSPKR3MIC_CIRC_SEND_UWB
                   - CVC 3 Mic Speakerphone circular
    CVCSPKR3MIC_CIRC_SEND_SWB
                   - CVC 3 Mic Speakerphone circular
    CVCSPKR3MIC_CIRC_SEND_FB
                   - CVC 3 Mic Speakerphone circular
    CVCSPKR4MIC_CIRC_SEND_UWB
                   - CVC 4 Mic Speakerphone circular
    CVCSPKR4MIC_CIRC_SEND_SWB
                   - CVC 4 Mic Speakerphone circular
    CVCSPKR4MIC_CIRC_SEND_FB
                   - CVC 4 Mic Speakerphone circular
    DBE_FULLBAND
                   - DBE_FULLBAND
    DBE_FULLBAND_BASSOUT
                   - DBE_FULLBAND_BASSOUT
    COMPANDER
                   - COMPANDER
    IIR_RESAMPLER
                   - IIR Resampler
    VAD
                   - Voice Activity Detection VAD
    SVA
                   - Phrase recogintion using Snapdragon Voice Activation
    CHANNEL_MIXER
                   - Channel Mixer for down/up mixing
    RTP_DECODE
                   - RTP decode capability
    SOURCE_SYNC
                   - SOURCE_SYNC
    USB_AUDIO_RX
                   - Reciving USB Audio
    USB_AUDIO_TX
                   - Transmitting USB Audio
    CELT_ENCODER
                   - CELT encoder - Raw
    CELT_DECODER
                   - CELT decoder - Raw
    APTXHD_DECODER
                   - aptxhd raw decoder
    APTXHD_A2DP_ENCODER
                   - High-Quality APT-X HD Audio Encoder
    APTXHD_SHUNT_DECODER
                   - APTXHD decoder over A2DP / Shunt (i.e. with L2CAP headers)
    APTX_CLASSIC_ENCODER
                   - APTX Classic Encoder
    APTX_LOW_LATENCY_ENCODER
                   - APTX LL Encoder
    APTXHD_ENCODER
                   - APTXHD Encoder
    APTX_CLASSIC_MONO_DECODER
                   - APTX Classic Mono Decoder
    APTXHD_MONO_DECODER
                   - APTX HD Mono Decoder
    APTX_CLASSIC_MONO_DECODER_NO_AUTOSYNC
                   - APTX Classic Mono Decoder without autosync
    APTXHD_MONO_DECODER_NO_AUTOSYNC
                   - APTX HD Mono Decoder without autosync
    APTX_CLASSIC_MONO_ENCODER
                   - APTX Classic Mono encoder with autosync
    APTX_CLASSIC_MONO_ENCODER_NO_AUTOSYNC
                   - APTX Classic Mono encoder without autosync
    APTXHD_MONO_ENCODER
                   - APTX HD Mono encoder with autosync
    APTXHD_MONO_ENCODER_NO_AUTOSYNC
                   - APTX HD Mono encoder without autosync
    TEST_CONSUMER
                   - Capability for consuming arbitrary data.
    CVSD_LOOPBACK
                   - CVSD loopback capability
    TEST_STALL_DROP
                   - Configurable stall/drop passthrough for testing
    MONO_PASS_BURST
                   - Mono passthrough burst capability
    MONITOR_OP
                   -
    XOR_PASS
                   -
    STEREO_XOR
                   -
    GARG_PASS
                   -
    STEREO_GARG
                   -
    DOWNLOAD_SELF_TEST
                   -
    DOWNLOAD_PASSTHROUGH
                   -
    DOWNLOAD_CVCSPKR1MIC_SEND_WB
                   - Downloadable CVC 1 Mic Speakerphone with gargle
    DOWNLOAD_AEC_REFERENCE
                   - Downloadable AEC Front End
    DOWNLOAD_CVC_RECEIVE_NB
                   - Downloadable CVC 1 Mic receive
    DOWNLOAD_CVC_RECEIVE_WB
                   - Downloadable CVC 1 Mic receive
    DOWNLOAD_CVC_RECEIVE_FE
                   - Downloadable CVC 1 Mic receive
    DOWNLOAD_CVC_RECEIVE_UWB
                   - Downloadable CVC 1 Mic receive
    DOWNLOAD_CVC_RECEIVE_SWB
                   - Downloadable CVC 1 Mic receive
    DOWNLOAD_CVC_RECEIVE_FB
                   - Downloadable CVC 1 Mic receive
    DOWNLOAD_CVCHF1MIC_SEND_NB
                   - Downloadable CVC 1 Mic Automotive
    DOWNLOAD_CVCHF1MIC_SEND_WB
                   - Downloadable CVC 1 Mic Automotive
    DOWNLOAD_CVCHF2MIC_SEND_NB
                   - Downloadable CVC 2 Mic Automotive
    DOWNLOAD_CVCHF2MIC_SEND_WB
                   - Downloadable CVC 2 Mic Automotive
    DOWNLOAD_CVCHS1MIC_SEND_NB
                   - Downloadable CVC 1 Mic Headset
    DOWNLOAD_CVCHS1MIC_SEND_WB
                   - Downloadable CVC 1 Mic Headset
    DOWNLOAD_CVCHS2MIC_MONO_SEND_NB
                   - Downloadable CVC 2 Mic Headset endfire
    DOWNLOAD_CVCHS2MIC_MONO_SEND_WB
                   - Downloadable CVC 2 Mic Headset endfire
    DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_NB
                   - Downloadable CVC 2 Mic Headset binaural
    DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_WB
                   - Downloadable CVC 2 Mic Headset binaural
    DOWNLOAD_CVCSPKR1MIC_SEND_NB
                   - Downloadable CVC 1 Mic Speakerphone
    DOWNLOAD_CVCSPKR2MIC_SEND_NB
                   - Downloadable CVC 2 Mic Speakerphone
    DOWNLOAD_CVCSPKR2MIC_SEND_WB
                   - Downloadable CVC 2 Mic Speakerphone
    DOWNLOAD_CVCSPKR3MIC_SEND_NB
                   - Downloadable CVC 3 Mic Speakerphone
    DOWNLOAD_CVCSPKR3MIC_SEND_WB
                   - Downloadable CVC 3 Mic Speakerphone
    DOWNLOAD_CVCSPKR4MIC_SEND_NB
                   - Downloadable CVC 4 Mic Speakerphone
    DOWNLOAD_CVCSPKR4MIC_SEND_WB
                   - Downloadable CVC 4 Mic Speakerphone
    DOWNLOAD_CVCHS3MIC_MONO_SEND_NB
                   - Downloadable CVC 3 Mic Mono Headset (endfire)
    DOWNLOAD_CVCHS3MIC_MONO_SEND_WB
                   - Downloadable CVC 3 Mic Mono Headset (endfire)
    DOWNLOAD_CVCHS3MIC_BINAURAL_SEND_NB
                   - Downloadable CVC 3 Mic Binaural Headset
    DOWNLOAD_CVCHS3MIC_BINAURAL_SEND_WB
                   - Downloadable CVC 3 Mic Binaural Headset
    DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_NB
                   - Downloadable CVC 3 Mic Speakerphone circular
    DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_WB
                   - Downloadable CVC 3 Mic Speakerphone circular
    DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_NB
                   - Downloadable CVC 4 Mic Speakerphone circular
    DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_WB
                   - Downloadable CVC 4 Mic Speakerphone circular
    DOWNLOAD_CVCHF1MIC_SEND_UWB
                   - Downloadable CVC 1 Mic Automotive
    DOWNLOAD_CVCHF1MIC_SEND_SWB
                   - Downloadable CVC 1 Mic Automotive
    DOWNLOAD_CVCHF1MIC_SEND_FB
                   - Downloadable CVC 1 Mic Automotive
    DOWNLOAD_CVCHF2MIC_SEND_UWB
                   - Downloadable CVC 2 Mic Automotive
    DOWNLOAD_CVCHF2MIC_SEND_SWB
                   - Downloadable CVC 2 Mic Automotive
    DOWNLOAD_CVCHF2MIC_SEND_FB
                   - Downloadable CVC 2 Mic Automotive
    DOWNLOAD_CVCHS1MIC_SEND_UWB
                   - Downloadable CVC 1 Mic Headset
    DOWNLOAD_CVCHS1MIC_SEND_SWB
                   - Downloadable CVC 1 Mic Headset
    DOWNLOAD_CVCHS1MIC_SEND_FB
                   - Downloadable CVC 1 Mic Headset
    DOWNLOAD_CVCHS2MIC_MONO_SEND_UWB
                   - Downloadable CVC 2 Mic Headset endfire
    DOWNLOAD_CVCHS2MIC_MONO_SEND_SWB
                   - Downloadable CVC 2 Mic Headset endfire
    DOWNLOAD_CVCHS2MIC_MONO_SEND_FB
                   - Downloadable CVC 2 Mic Headset endfire
    DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_UWB
                   - Downloadable CVC 2 Mic Headset binaural
    DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_SWB
                   - Downloadable CVC 2 Mic Headset binaural
    DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_FB
                   - Downloadable CVC 2 Mic Headset binaural
    DOWNLOAD_CVCHS3MIC_MONO_SEND_UWB
                   - Downloadable CVC 3 Mic Mono Headset (endfire)
    DOWNLOAD_CVCHS3MIC_MONO_SEND_SWB
                   - Downloadable CVC 3 Mic Mono Headset (endfire)
    DOWNLOAD_CVCHS3MIC_MONO_SEND_FB
                   - Downloadable CVC 3 Mic Mono Headset (endfire)
    DOWNLOAD_CVCHS3MIC_BINAURAL_SEND_UWB
                   - Downloadable CVC 3 Mic Binaural Headset
    DOWNLOAD_CVCHS3MIC_BINAURAL_SEND_SWB
                   - Downloadable CVC 3 Mic Binaural Headset
    DOWNLOAD_CVCHS3MIC_BINAURAL_SEND_FB
                   - Downloadable CVC 3 Mic Binaural Headset
    DOWNLOAD_CVCSPKR1MIC_SEND_UWB
                   - Downloadable CVC 1 Mic Speakerphone
    DOWNLOAD_CVCSPKR1MIC_SEND_SWB
                   - Downloadable CVC 1 Mic Speakerphone
    DOWNLOAD_CVCSPKR1MIC_SEND_FB
                   - Downloadable CVC 1 Mic Speakerphone
    DOWNLOAD_CVCSPKR2MIC_SEND_UWB
                   - Downloadable CVC 2 Mic Speakerphone
    DOWNLOAD_CVCSPKR2MIC_SEND_SWB
                   - Downloadable CVC 2 Mic Speakerphone
    DOWNLOAD_CVCSPKR2MIC_SEND_FB
                   - Downloadable CVC 2 Mic Speakerphone
    DOWNLOAD_CVCSPKR3MIC_SEND_UWB
                   - Downloadable CVC 3 Mic Speakerphone
    DOWNLOAD_CVCSPKR3MIC_SEND_SWB
                   - Downloadable CVC 3 Mic Speakerphone
    DOWNLOAD_CVCSPKR3MIC_SEND_FB
                   - Downloadable CVC 3 Mic Speakerphone
    DOWNLOAD_CVCSPKR4MIC_SEND_UWB
                   - Downloadable CVC 4 Mic Speakerphone
    DOWNLOAD_CVCSPKR4MIC_SEND_SWB
                   - Downloadable CVC 4 Mic Speakerphone
    DOWNLOAD_CVCSPKR4MIC_SEND_FB
                   - Downloadable CVC 4 Mic Speakerphone
    DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_UWB
                   - Downloadable CVC 3 Mic Speakerphone circular
    DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_SWB
                   - Downloadable CVC 3 Mic Speakerphone circular
    DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_FB
                   - Downloadable CVC 3 Mic Speakerphone circular
    DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_UWB
                   - Downloadable CVC 4 Mic Speakerphone circular
    DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_SWB
                   - Downloadable CVC 4 Mic Speakerphone circular
    DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_FB
                   - Downloadable CVC 4 Mic Speakerphone circular
    DOWNLOAD_AAC_SHUNT_DECODER
                   - Downloadable AAC decoder over A2DP / Shunt (i.e. with L2CAP
                     headers)
    DOWNLOAD_AAC_DECODER
                   - Downloadable AAC raw decoder
    DOWNLOAD_APTX_SHUNT_DECODER
                   - Downloadable APTX decoder over A2DP / Shunt (i.e. with
                     L2CAP headers)
    DOWNLOAD_APTX_CLASSIC_DECODER
                   - Downloadable aptx classic raw decoder
    DOWNLOAD_APTX_LOW_LATENCY_DECODER
                   - Downloadable aptx low latency decoder
    DOWNLOAD_IIR_RESAMPLER
                   - Downloadable IIR Resampler
    DOWNLOAD_VOL_CTRL_VOL
                   - Downloadable Volume Control
    DOWNLOAD_CELT_ENCODER
                   - Downloadable CELT encoder - Raw
    DOWNLOAD_CELT_DECODER
                   - Downloadable CELT decoder - Raw
    DOWNLOAD_COMPANDER
                   - Downloadable COMPANDER
    DOWNLOAD_DBE
                   - Downloadable DBE
    DOWNLOAD_DBE_FULLBAND
                   - Downloadable DBE_FULLBAND
    DOWNLOAD_DBE_FULLBAND_BASSOUT
                   - Downloadable DBE_FULLBAND_BASSOUT
    DOWNLOAD_MIXER
                   - Downloadable Stereo/Mono Audio Mixer
    DOWNLOAD_PEQ
                   - Downloadable PEQ
    DOWNLOAD_RINGTONE_GENERATOR
                   - Downloadable Ringtone Generator
    DOWNLOAD_RTP_DECODE
                   - Downloadable RTP decode capability
    DOWNLOAD_SBC_DECODER
                   - Downloadable SBC decoder - Raw data in
    DOWNLOAD_SBC_ENCODER
                   - Downloadable SBC encoder - Raw
    DOWNLOAD_SCO_SEND
                   - Downloadable SCO send operator
    DOWNLOAD_SCO_RCV
                   - Downloadable SCO receiver with packet loss concealment
    DOWNLOAD_WBS_ENC
                   - Downloadable Wideband Speech encoder
    DOWNLOAD_WBS_DEC
                   - Downloadable Wideband Speech decoder
    DOWNLOAD_SPDIF_DECODE
                   - Downloadable S/PDIF RX driver
    DOWNLOAD_SPLITTER
                   - Downloadable Mono audio splitter
    DOWNLOAD_TTP_PASS
                   - Downloadable Passthrough with time-to-play generation
    DOWNLOAD_USB_AUDIO_RX
                   - Downloadable Reciving USB Audio
    DOWNLOAD_USB_AUDIO_TX
                   - Downloadable Transmitting USB Audio
    DOWNLOAD_VSE
                   - Downloadable VSE
    DOWNLOAD_XOVER
                   - Downloadable XOVER
    DOWNLOAD_SVA
                   - Downloadable Phrase recogintion using Snapdragon Voice
                     Activation
    DUMMY_ID_FORCE_INT
                   - Dummy value to enforce enum uses int type

*******************************************************************************/
typedef enum
{
    CAP_ID_NONE = 0x0000,
    CAP_ID_BASIC_PASS = 0x0001,
    CAP_ID_STEREO_PASS = 0x0002,
    CAP_ID_SCO_SEND = 0x0003,
    CAP_ID_SCO_RCV = 0x0004,
    CAP_ID_WBS_ENC = 0x0005,
    CAP_ID_WBS_DEC = 0x0006,
    CAP_ID_SBC_A2DP_ENCODER = 0x0007,
    CAP_ID_MP3_A2DP_ENCODER = 0x0008,
    CAP_ID_RESAMPLER = 0x0009,
    CAP_ID_MIXER = 0x000A,
    CAP_ID_APTX_A2DP_ENCODER = 0x000B,
    CAP_ID_FM_ENHANCEMENT = 0x000C,
    CAP_ID_SBC_SHUNT_DECODER = 0x000D,
    CAP_ID_MP3_SHUNT_DECODER = 0x000E,
    CAP_ID_AAC_SHUNT_DECODER = 0x000F,
    CAP_ID_APTX_SHUNT_DECODER = 0x0010,
    CAP_ID_RESERVED_0011 = 0x0011,
    CAP_ID_EQ_COMP = 0x0012,
    CAP_ID_SPLITTER = 0x0013,
    CAP_ID_SBC_ENCODER = 0x0014,
    CAP_ID_MP3_ENCODER = 0x0015,
    CAP_ID_SBC_DECODER = 0x0016,
    CAP_ID_MP3_DECODER = 0x0017,
    CAP_ID_AAC_DECODER = 0x0018,
    CAP_ID_APTX_CLASSIC_DECODER = 0x0019,
    CAP_ID_CVC_RECEIVE_FE = 0x001B,
    CAP_ID_CVCHF1MIC_SEND_NB = 0x001C,
    CAP_ID_CVC_RECEIVE_NB = 0x001D,
    CAP_ID_CVCHF1MIC_SEND_WB = 0x001E,
    CAP_ID_CVC_RECEIVE_WB = 0x001F,
    CAP_ID_CVCHF2MIC_SEND_NB = 0x0020,
    CAP_ID_CVCHF2MIC_SEND_WB = 0x0021,
    CAP_ID_CVCHS1MIC_SEND_NB = 0x0023,
    CAP_ID_CVCHS1MIC_SEND_WB = 0x0024,
    CAP_ID_CVCHS2MIC_MONO_SEND_NB = 0x0025,
    CAP_ID_CVCHS2MIC_MONO_SEND_WB = 0x0026,
    CAP_ID_CVCHS2MIC_BINAURAL_SEND_NB = 0x0027,
    CAP_ID_CVCHS2MIC_BINAURAL_SEND_WB = 0x0028,
    CAP_ID_CVCSPKR1MIC_SEND_NB = 0x0029,
    CAP_ID_CVCSPKR1MIC_SEND_WB = 0x002A,
    CAP_ID_CVCSPKR2MIC_SEND_NB = 0x002D,
    CAP_ID_CVCSPKR2MIC_SEND_WB = 0x002E,
    CAP_ID_DBE = 0x002F,
    CAP_ID_XOVER = 0x0033,
    CAP_ID_DELAY = 0x0035,
    CAP_ID_SPDIF_DECODE = 0x0036,
    CAP_ID_RINGTONE_GENERATOR = 0x0037,
    CAP_ID_TTP_PASS = 0x003C,
    CAP_ID_APTX_LOW_LATENCY_DECODER = 0x003D,
    CAP_ID_INTERLEAVE = 0x003E,
    CAP_ID_DEINTERLEAVE = 0x003F,
    CAP_ID_AEC_REFERENCE = 0x0043,
    CAP_ID_CVCSPKR3MIC_SEND_NB = 0x0044,
    CAP_ID_CVCSPKR3MIC_SEND_WB = 0x0045,
    CAP_ID_CVCSPKR4MIC_SEND_NB = 0x0046,
    CAP_ID_CVCSPKR4MIC_SEND_WB = 0x0047,
    CAP_ID_VOL_CTRL_VOL = 0x0048,
    CAP_ID_PEQ = 0x0049,
    CAP_ID_VSE = 0x004A,
    CAP_ID_CVCHS3MIC_MONO_SEND_NB = 0x004B,
    CAP_ID_CVCHS3MIC_MONO_SEND_WB = 0x004C,
    CAP_ID_CVCHS3MIC_BINAURAL_SEND_NB = 0x004D,
    CAP_ID_CVCHS3MIC_BINAURAL_SEND_WB = 0x004E,
    CAP_ID_CVCSPKR3MIC_CIRC_SEND_NB = 0x004F,
    CAP_ID_CVCSPKR3MIC_CIRC_SEND_WB = 0x0050,
    CAP_ID_CVCSPKR4MIC_CIRC_SEND_NB = 0x0051,
    CAP_ID_CVCSPKR4MIC_CIRC_SEND_WB = 0x0052,
    CAP_ID_CVC_RECEIVE_UWB = 0x0053,
    CAP_ID_CVC_RECEIVE_SWB = 0x0054,
    CAP_ID_CVC_RECEIVE_FB = 0x0055,
    CAP_ID_CVCHF1MIC_SEND_UWB = 0x0056,
    CAP_ID_CVCHF1MIC_SEND_SWB = 0x0057,
    CAP_ID_CVCHF1MIC_SEND_FB = 0x0058,
    CAP_ID_CVCHF2MIC_SEND_UWB = 0x0059,
    CAP_ID_CVCHF2MIC_SEND_SWB = 0x005A,
    CAP_ID_CVCHF2MIC_SEND_FB = 0x005B,
    CAP_ID_CVCHS1MIC_SEND_UWB = 0x005C,
    CAP_ID_CVCHS1MIC_SEND_SWB = 0x005D,
    CAP_ID_CVCHS1MIC_SEND_FB = 0x005E,
    CAP_ID_CVCHS2MIC_MONO_SEND_UWB = 0x005F,
    CAP_ID_CVCHS2MIC_MONO_SEND_SWB = 0x0060,
    CAP_ID_CVCHS2MIC_MONO_SEND_FB = 0x0061,
    CAP_ID_CVCHS2MIC_BINAURAL_SEND_UWB = 0x0062,
    CAP_ID_CVCHS2MIC_BINAURAL_SEND_SWB = 0x0063,
    CAP_ID_CVCHS2MIC_BINAURAL_SEND_FB = 0x0064,
    CAP_ID_CVCHS3MIC_MONO_SEND_UWB = 0x0065,
    CAP_ID_CVCHS3MIC_MONO_SEND_SWB = 0x0066,
    CAP_ID_CVCHS3MIC_MONO_SEND_FB = 0x0067,
    CAP_ID_CVCHS3MIC_BINAURAL_SEND_UWB = 0x0068,
    CAP_ID_CVCHS3MIC_BINAURAL_SEND_SWB = 0x0069,
    CAP_ID_CVCHS3MIC_BINAURAL_SEND_FB = 0x006A,
    CAP_ID_CVCSPKR1MIC_SEND_UWB = 0x006B,
    CAP_ID_CVCSPKR1MIC_SEND_SWB = 0x006C,
    CAP_ID_CVCSPKR1MIC_SEND_FB = 0x006D,
    CAP_ID_CVCSPKR2MIC_SEND_UWB = 0x006E,
    CAP_ID_CVCSPKR2MIC_SEND_SWB = 0x006F,
    CAP_ID_CVCSPKR2MIC_SEND_FB = 0x0070,
    CAP_ID_CVCSPKR3MIC_SEND_UWB = 0x0071,
    CAP_ID_CVCSPKR3MIC_SEND_SWB = 0x0072,
    CAP_ID_CVCSPKR3MIC_SEND_FB = 0x0073,
    CAP_ID_CVCSPKR4MIC_SEND_UWB = 0x0074,
    CAP_ID_CVCSPKR4MIC_SEND_SWB = 0x0075,
    CAP_ID_CVCSPKR4MIC_SEND_FB = 0x0076,
    CAP_ID_CVCSPKR3MIC_CIRC_SEND_UWB = 0x0077,
    CAP_ID_CVCSPKR3MIC_CIRC_SEND_SWB = 0x0078,
    CAP_ID_CVCSPKR3MIC_CIRC_SEND_FB = 0x0079,
    CAP_ID_CVCSPKR4MIC_CIRC_SEND_UWB = 0x007A,
    CAP_ID_CVCSPKR4MIC_CIRC_SEND_SWB = 0x007B,
    CAP_ID_CVCSPKR4MIC_CIRC_SEND_FB = 0x007C,
    CAP_ID_DBE_FULLBAND = 0x0090,
    CAP_ID_DBE_FULLBAND_BASSOUT = 0x0091,
    CAP_ID_COMPANDER = 0x0092,
    CAP_ID_IIR_RESAMPLER = 0x0094,
    CAP_ID_VAD = 0x0095,
    CAP_ID_SVA = 0x0096,
    CAP_ID_CHANNEL_MIXER = 0x0097,
    CAP_ID_RTP_DECODE = 0x0098,
    CAP_ID_SOURCE_SYNC = 0x0099,
    CAP_ID_USB_AUDIO_RX = 0x009A,
    CAP_ID_USB_AUDIO_TX = 0x009B,
    CAP_ID_CELT_ENCODER = 0x009C,
    CAP_ID_CELT_DECODER = 0x009D,
    CAP_ID_APTXHD_DECODER = 0x009E,
    CAP_ID_APTXHD_A2DP_ENCODER = 0x009F,
    CAP_ID_APTXHD_SHUNT_DECODER = 0x00A0,
    CAP_ID_APTX_CLASSIC_ENCODER = 0x00A5,
    CAP_ID_APTX_LOW_LATENCY_ENCODER = 0x00A6,
    CAP_ID_APTXHD_ENCODER = 0x00A7,
    CAP_ID_APTX_CLASSIC_MONO_DECODER = 0x00A9,
    CAP_ID_APTXHD_MONO_DECODER = 0x00AA,
    CAP_ID_APTX_CLASSIC_MONO_DECODER_NO_AUTOSYNC = 0x00AB,
    CAP_ID_APTXHD_MONO_DECODER_NO_AUTOSYNC = 0x00AC,
    CAP_ID_APTX_CLASSIC_MONO_ENCODER = 0x00AD,
    CAP_ID_APTX_CLASSIC_MONO_ENCODER_NO_AUTOSYNC = 0x00AE,
    CAP_ID_APTXHD_MONO_ENCODER = 0x00AF,
    CAP_ID_APTXHD_MONO_ENCODER_NO_AUTOSYNC = 0x00B0,
    CAP_ID_TEST_CONSUMER = 0x3FF5,
    CAP_ID_CVSD_LOOPBACK = 0x3FF6,
    CAP_ID_TEST_STALL_DROP = 0x3FF7,
    CAP_ID_MONO_PASS_BURST = 0x3FF8,
    CAP_ID_MONITOR_OP = 0x3FF9,
    CAP_ID_XOR_PASS = 0x3FFC,
    CAP_ID_STEREO_XOR = 0x3FFD,
    CAP_ID_GARG_PASS = 0x3FFE,
    CAP_ID_STEREO_GARG = 0x3FFF,
    CAP_ID_DOWNLOAD_SELF_TEST = 0x4001,
    CAP_ID_DOWNLOAD_PASSTHROUGH = 0x4002,
    CAP_ID_DOWNLOAD_CVCSPKR1MIC_SEND_WB = 0x4003,
    CAP_ID_DOWNLOAD_AEC_REFERENCE = 0x4007,
    CAP_ID_DOWNLOAD_CVC_RECEIVE_NB = 0x4008,
    CAP_ID_DOWNLOAD_CVC_RECEIVE_WB = 0x4009,
    CAP_ID_DOWNLOAD_CVC_RECEIVE_FE = 0x400A,
    CAP_ID_DOWNLOAD_CVC_RECEIVE_UWB = 0x400B,
    CAP_ID_DOWNLOAD_CVC_RECEIVE_SWB = 0x400C,
    CAP_ID_DOWNLOAD_CVC_RECEIVE_FB = 0x400D,
    CAP_ID_DOWNLOAD_CVCHF1MIC_SEND_NB = 0x400E,
    CAP_ID_DOWNLOAD_CVCHF1MIC_SEND_WB = 0x400F,
    CAP_ID_DOWNLOAD_CVCHF2MIC_SEND_NB = 0x4010,
    CAP_ID_DOWNLOAD_CVCHF2MIC_SEND_WB = 0x4011,
    CAP_ID_DOWNLOAD_CVCHS1MIC_SEND_NB = 0x4012,
    CAP_ID_DOWNLOAD_CVCHS1MIC_SEND_WB = 0x4013,
    CAP_ID_DOWNLOAD_CVCHS2MIC_MONO_SEND_NB = 0x4014,
    CAP_ID_DOWNLOAD_CVCHS2MIC_MONO_SEND_WB = 0x4015,
    CAP_ID_DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_NB = 0x4016,
    CAP_ID_DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_WB = 0x4017,
    CAP_ID_DOWNLOAD_CVCSPKR1MIC_SEND_NB = 0x4018,
    CAP_ID_DOWNLOAD_CVCSPKR2MIC_SEND_NB = 0x4019,
    CAP_ID_DOWNLOAD_CVCSPKR2MIC_SEND_WB = 0x401A,
    CAP_ID_DOWNLOAD_CVCSPKR3MIC_SEND_NB = 0x401B,
    CAP_ID_DOWNLOAD_CVCSPKR3MIC_SEND_WB = 0x401C,
    CAP_ID_DOWNLOAD_CVCSPKR4MIC_SEND_NB = 0x401D,
    CAP_ID_DOWNLOAD_CVCSPKR4MIC_SEND_WB = 0x401E,
    CAP_ID_DOWNLOAD_CVCHS3MIC_MONO_SEND_NB = 0x401F,
    CAP_ID_DOWNLOAD_CVCHS3MIC_MONO_SEND_WB = 0x4020,
    CAP_ID_DOWNLOAD_CVCHS3MIC_BINAURAL_SEND_NB = 0x4021,
    CAP_ID_DOWNLOAD_CVCHS3MIC_BINAURAL_SEND_WB = 0x4022,
    CAP_ID_DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_NB = 0x4023,
    CAP_ID_DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_WB = 0x4024,
    CAP_ID_DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_NB = 0x4025,
    CAP_ID_DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_WB = 0x4026,
    CAP_ID_DOWNLOAD_CVCHF1MIC_SEND_UWB = 0x4027,
    CAP_ID_DOWNLOAD_CVCHF1MIC_SEND_SWB = 0x4028,
    CAP_ID_DOWNLOAD_CVCHF1MIC_SEND_FB = 0x4029,
    CAP_ID_DOWNLOAD_CVCHF2MIC_SEND_UWB = 0x402A,
    CAP_ID_DOWNLOAD_CVCHF2MIC_SEND_SWB = 0x402B,
    CAP_ID_DOWNLOAD_CVCHF2MIC_SEND_FB = 0x402C,
    CAP_ID_DOWNLOAD_CVCHS1MIC_SEND_UWB = 0x402D,
    CAP_ID_DOWNLOAD_CVCHS1MIC_SEND_SWB = 0x402E,
    CAP_ID_DOWNLOAD_CVCHS1MIC_SEND_FB = 0x402F,
    CAP_ID_DOWNLOAD_CVCHS2MIC_MONO_SEND_UWB = 0x4030,
    CAP_ID_DOWNLOAD_CVCHS2MIC_MONO_SEND_SWB = 0x4031,
    CAP_ID_DOWNLOAD_CVCHS2MIC_MONO_SEND_FB = 0x4032,
    CAP_ID_DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_UWB = 0x4033,
    CAP_ID_DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_SWB = 0x4034,
    CAP_ID_DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_FB = 0x4035,
    CAP_ID_DOWNLOAD_CVCHS3MIC_MONO_SEND_UWB = 0x4036,
    CAP_ID_DOWNLOAD_CVCHS3MIC_MONO_SEND_SWB = 0x4037,
    CAP_ID_DOWNLOAD_CVCHS3MIC_MONO_SEND_FB = 0x4038,
    CAP_ID_DOWNLOAD_CVCHS3MIC_BINAURAL_SEND_UWB = 0x4039,
    CAP_ID_DOWNLOAD_CVCHS3MIC_BINAURAL_SEND_SWB = 0x403A,
    CAP_ID_DOWNLOAD_CVCHS3MIC_BINAURAL_SEND_FB = 0x403B,
    CAP_ID_DOWNLOAD_CVCSPKR1MIC_SEND_UWB = 0x403C,
    CAP_ID_DOWNLOAD_CVCSPKR1MIC_SEND_SWB = 0x403D,
    CAP_ID_DOWNLOAD_CVCSPKR1MIC_SEND_FB = 0x403E,
    CAP_ID_DOWNLOAD_CVCSPKR2MIC_SEND_UWB = 0x403F,
    CAP_ID_DOWNLOAD_CVCSPKR2MIC_SEND_SWB = 0x4040,
    CAP_ID_DOWNLOAD_CVCSPKR2MIC_SEND_FB = 0x4041,
    CAP_ID_DOWNLOAD_CVCSPKR3MIC_SEND_UWB = 0x4042,
    CAP_ID_DOWNLOAD_CVCSPKR3MIC_SEND_SWB = 0x4043,
    CAP_ID_DOWNLOAD_CVCSPKR3MIC_SEND_FB = 0x4044,
    CAP_ID_DOWNLOAD_CVCSPKR4MIC_SEND_UWB = 0x4045,
    CAP_ID_DOWNLOAD_CVCSPKR4MIC_SEND_SWB = 0x4046,
    CAP_ID_DOWNLOAD_CVCSPKR4MIC_SEND_FB = 0x4047,
    CAP_ID_DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_UWB = 0x4048,
    CAP_ID_DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_SWB = 0x4049,
    CAP_ID_DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_FB = 0x404A,
    CAP_ID_DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_UWB = 0x404B,
    CAP_ID_DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_SWB = 0x404C,
    CAP_ID_DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_FB = 0x404D,
    CAP_ID_DOWNLOAD_AAC_SHUNT_DECODER = 0x404E,
    CAP_ID_DOWNLOAD_AAC_DECODER = 0x404F,
    CAP_ID_DOWNLOAD_APTX_SHUNT_DECODER = 0x4050,
    CAP_ID_DOWNLOAD_APTX_CLASSIC_DECODER = 0x4051,
    CAP_ID_DOWNLOAD_APTX_LOW_LATENCY_DECODER = 0x4052,
    CAP_ID_DOWNLOAD_IIR_RESAMPLER = 0x4053,
    CAP_ID_DOWNLOAD_VOL_CTRL_VOL = 0x4054,
    CAP_ID_DOWNLOAD_CELT_ENCODER = 0x4055,
    CAP_ID_DOWNLOAD_CELT_DECODER = 0x4056,
    CAP_ID_DOWNLOAD_COMPANDER = 0x4057,
    CAP_ID_DOWNLOAD_DBE = 0x4058,
    CAP_ID_DOWNLOAD_DBE_FULLBAND = 0x4059,
    CAP_ID_DOWNLOAD_DBE_FULLBAND_BASSOUT = 0x405A,
    CAP_ID_DOWNLOAD_MIXER = 0x405B,
    CAP_ID_DOWNLOAD_PEQ = 0x405C,
    CAP_ID_DOWNLOAD_RINGTONE_GENERATOR = 0x405D,
    CAP_ID_DOWNLOAD_RTP_DECODE = 0x405E,
    CAP_ID_DOWNLOAD_SBC_DECODER = 0x405F,
    CAP_ID_DOWNLOAD_SBC_ENCODER = 0x4060,
    CAP_ID_DOWNLOAD_SCO_SEND = 0x4061,
    CAP_ID_DOWNLOAD_SCO_RCV = 0x4062,
    CAP_ID_DOWNLOAD_WBS_ENC = 0x4063,
    CAP_ID_DOWNLOAD_WBS_DEC = 0x4064,
    CAP_ID_DOWNLOAD_SPDIF_DECODE = 0x4065,
    CAP_ID_DOWNLOAD_SPLITTER = 0x4066,
    CAP_ID_DOWNLOAD_TTP_PASS = 0x4067,
    CAP_ID_DOWNLOAD_USB_AUDIO_RX = 0x4068,
    CAP_ID_DOWNLOAD_USB_AUDIO_TX = 0x4069,
    CAP_ID_DOWNLOAD_VSE = 0x406A,
    CAP_ID_DOWNLOAD_XOVER = 0x406B,
    CAP_ID_DOWNLOAD_SVA = 0x406C,
    CAP_ID_DUMMY_ID_FORCE_INT = INT_MAX
} CAP_ID;


#define CAP_ID_PRIM_ANY_SIZE 1


#endif /* CAP_ID_PRIM_H */

