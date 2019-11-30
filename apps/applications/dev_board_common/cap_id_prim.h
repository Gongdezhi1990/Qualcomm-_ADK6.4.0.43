/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*    */
/*****************************************************************************

            (c) Qualcomm Technologies International, Ltd. 2016
            Confidential information of Qualcomm

            Refer to LICENSE.txt included with this source for details
            on the license terms.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef CAP_ID_PRIM_H
#define CAP_ID_PRIM_H


/*******************************************************************************

  NAME
    Cap_Id

  DESCRIPTION
    Defines for the Capability identifiers.

 VALUES
    NONE                        - test basic_op capability
    BASIC_PASS                  - test basic passthrough capability
    STEREO_PASS                 - test stereo passthrough capability
    SCO_SEND                    - SCO send operator
    SCO_RCV                     - SCO receiver with packet loss concealment
    WBS_ENC                     - Wideband Speech encoder
    WBS_DEC                     - Wideband Speech decoder
    SBC_A2DP_ENCODER            - SBC Encoder for A2DP
    MP3_A2DP_ENCODER            - Configurable Mp3 encoder
    RESAMPLER                   - Standard-frequency-set Resampler
    MIXER                       - Stereo/Mono Audio Mixer
    APTX_A2DP_ENCODER           - High-Quality APT-X Audio Encoder
    FM_ENHANCEMENT              - FM Receive audio quality enhancement
    SBC_A2DP_DECODER            - SBC decoder over A2DP
    MP3_A2DP_DECODER            - MP3 decoder over A2DP
    AAC_A2DP_DECODER            - AAC decoder over A2DP
    APTX_A2DP_DECODER           - APTX decoder over A2DP
    RINGTONE_GEN                -
    EQ_COMP                     - Equaliser and Compressor, Limiter, Expander
    SPLITTER                    - Mono audio splitter
    SBC_ENCODER                 - SBC encoder - Raw
    MP3_ENCODER                 - MP3 encoder - Raw
    SBC_DECODER                 - SBC decoder - Raw data in
    MP3_DECODER                 - MP3 raw decoder
    AAC_DECODER                 - AAC raw decoder
    APTX_DECODER                - aptx raw decoder
    CVC_RECEIVE_FE              - CVC 1 Mic receive
    CVCHF1MIC_SEND_NB           - CVC 1 Mic Automotive
    CVC_RECEIVE_NB              - CVC 1 Mic receive
    CVCHF1MIC_SEND_WB           - CVC 1 Mic Automotive
    CVC_RECEIVE_WB              - CVC 1 Mic receive
    CVCHF2MIC_SEND_NB           - CVC 2 Mic Automotive
    CVCHF2MIC_SEND_WB           - CVC 2 Mic Automotive
    CVCHS1MIC_SEND_NB           - CVC 1 Mic Headset
    CVCHS1MIC_SEND_WB           - CVC 1 Mic Headset
    CVCHS2MIC_MONO_SEND_NB      - CVC 2 Mic Headset endfire
    CVCHS2MIC_MONO_SEND_WB      - CVC 2 Mic Headset endfire
    CVCHS2MIC_BINAURAL_SEND_NB  - CVC 2 Mic Headset binaural
    CVCHS2MIC_BINAURAL_SEND_WB  - CVC 2 Mic Headset binaural
    CVCSPKR1MIC_SEND_NB         - CVC 1 Mic Speakerphone
    CVCSPKR1MIC_SEND_WB         - CVC 1 Mic Speakerphone
    CVCSPKR2MIC_SEND_NB         - CVC 2 Mic Speakerphone
    CVCSPKR2MIC_SEND_WB         - CVC 2 Mic Speakerphone
    DBE                         - DBE
    XOVER                       - XOVER
    DELAY                       - DELAY
    SPDIF_DECODE                - S/PDIF RX driver
    RINGTONE_GENERATOR          - Ringtone Generator
    INTERLEAVE                  - Interleave operator
    DEINTERLEAVE                - Deinterleave operator
    AEC_REFERENCE_1MIC          - 1-MIC AEC Front End
    AEC_REFERENCE_2MIC          - 2-MIC AEC Front End
    AEC_REFERENCE_3MIC          - 3-MIC AEC Front End
    AEC_REFERENCE_4MIC          - 4-MIC AEC Front End
    CVCSPKR3MIC_SEND_NB         - CVC 3 Mic Speakerphone
    CVCSPKR3MIC_SEND_WB         - CVC 3 Mic Speakerphone
    CVCSPKR4MIC_SEND_NB         - CVC 4 Mic Speakerphone
    CVCSPKR4MIC_SEND_WB         - CVC 4 Mic Speakerphone
    VOL_CTRL_VOL                - Volume Control
    PEQ                         - PEQ
    VSE                         - VSE
    CVCHS3MIC_MONO_SEND_NB      - CVC 3 Mic Mono Headset (endfire)
    CVCHS3MIC_MONO_SEND_WB      - CVC 3 Mic Mono Headset (endfire)
    CVCHS3MIC_BINAURAL_SEND_NB  - CVC 3 Mic Binaural Headset
    CVCHS3MIC_BINAURAL_SEND_WB  - CVC 3 Mic Binaural Headset
    CVCSPKR3MIC_CIRC_SEND_NB    - CVC 3 Mic Speakerphone circular
    CVCSPKR3MIC_CIRC_SEND_WB    - CVC 3 Mic Speakerphone circular
    CVCSPKR4MIC_CIRC_SEND_NB    - CVC 4 Mic Speakerphone circular
    CVCSPKR4MIC_CIRC_SEND_WB    - CVC 4 Mic Speakerphone circular
    CVC_RECEIVE_UWB             - CVC 1 Mic receive
    CVC_RECEIVE_SWB             - CVC 1 Mic receive
    CVC_RECEIVE_FB              - CVC 1 Mic receive
    CVCHF1MIC_SEND_UWB          - CVC 1 Mic Automotive
    CVCHF1MIC_SEND_SWB          - CVC 1 Mic Automotive
    CVCHF1MIC_SEND_FB           - CVC 1 Mic Automotive
    CVCHF2MIC_SEND_UWB          - CVC 2 Mic Automotive
    CVCHF2MIC_SEND_SWB          - CVC 2 Mic Automotive
    CVCHF2MIC_SEND_FB           - CVC 2 Mic Automotive
    CVCHS1MIC_SEND_UWB          - CVC 1 Mic Headset
    CVCHS1MIC_SEND_SWB          - CVC 1 Mic Headset
    CVCHS1MIC_SEND_FB           - CVC 1 Mic Headset
    CVCHS2MIC_MONO_SEND_UWB     - CVC 2 Mic Headset endfire
    CVCHS2MIC_MONO_SEND_SWB     - CVC 2 Mic Headset endfire
    CVCHS2MIC_MONO_SEND_FB      - CVC 2 Mic Headset endfire
    CVCHS2MIC_BINAURAL_SEND_UWB - CVC 2 Mic Headset binaural
    CVCHS2MIC_BINAURAL_SEND_SWB - CVC 2 Mic Headset binaural
    CVCHS2MIC_BINAURAL_SEND_FB  - CVC 2 Mic Headset binaural
    CVCHS3MIC_MONO_SEND_UWB     - CVC 3 Mic Mono Headset (endfire)
    CVCHS3MIC_MONO_SEND_SWB     - CVC 3 Mic Mono Headset (endfire)
    CVCHS3MIC_MONO_SEND_FB      - CVC 3 Mic Mono Headset (endfire)
    CVCHS3MIC_BINAURAL_SEND_UWB - CVC 3 Mic Binaural Headset
    CVCHS3MIC_BINAURAL_SEND_SWB - CVC 3 Mic Binaural Headset
    CVCHS3MIC_BINAURAL_SEND_FB  - CVC 3 Mic Binaural Headset
    CVCSPKR1MIC_SEND_UWB        - CVC 1 Mic Speakerphone
    CVCSPKR1MIC_SEND_SWB        - CVC 1 Mic Speakerphone
    CVCSPKR1MIC_SEND_FB         - CVC 1 Mic Speakerphone
    CVCSPKR2MIC_SEND_UWB        - CVC 2 Mic Speakerphone
    CVCSPKR2MIC_SEND_SWB        - CVC 2 Mic Speakerphone
    CVCSPKR2MIC_SEND_FB         - CVC 2 Mic Speakerphone
    CVCSPKR3MIC_SEND_UWB        - CVC 3 Mic Speakerphone
    CVCSPKR3MIC_SEND_SWB        - CVC 3 Mic Speakerphone
    CVCSPKR3MIC_SEND_FB         - CVC 3 Mic Speakerphone
    CVCSPKR4MIC_SEND_UWB        - CVC 4 Mic Speakerphone
    CVCSPKR4MIC_SEND_SWB        - CVC 4 Mic Speakerphone
    CVCSPKR4MIC_SEND_FB         - CVC 4 Mic Speakerphone
    CVCSPKR3MIC_CIRC_SEND_UWB   - CVC 3 Mic Speakerphone circular
    CVCSPKR3MIC_CIRC_SEND_SWB   - CVC 3 Mic Speakerphone circular
    CVCSPKR3MIC_CIRC_SEND_FB    - CVC 3 Mic Speakerphone circular
    CVCSPKR4MIC_CIRC_SEND_UWB   - CVC 4 Mic Speakerphone circular
    CVCSPKR4MIC_CIRC_SEND_SWB   - CVC 4 Mic Speakerphone circular
    CVCSPKR4MIC_CIRC_SEND_FB    - CVC 4 Mic Speakerphone circular
    DBE_FULLBAND                - DBE_FULLBAND
    DBE_FULLBAND_BASSOUT        - DBE_FULLBAND_BASSOUT
    COMPANDER                   - COMPANDER
    IIR_RESAMPLER               - IIR Resampler
    CHANNEL_MIXER               - Channel Mixer for down/up mixing
    RTP_DECODE                  - RTP decode capability
    MONO_PASS_BURST             - Mono passthrough burst capability
    MONITOR_OP                  -
    XOR_PASS                    -
    STEREO_XOR                  -
    GARG_PASS                   -
    STEREO_GARG                 -
    DOWNLOAD_SELF_TEST          -
    DOWNLOAD_PASSTHROUGH        -

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
    CAP_ID_SBC_A2DP_DECODER = 0x000D,
    CAP_ID_MP3_A2DP_DECODER = 0x000E,
    CAP_ID_AAC_A2DP_DECODER = 0x000F,
    CAP_ID_APTX_A2DP_DECODER = 0x0010,
    CAP_ID_RINGTONE_GEN = 0x0011,
    CAP_ID_EQ_COMP = 0x0012,
    CAP_ID_SPLITTER = 0x0013,
    CAP_ID_SBC_ENCODER = 0x0014,
    CAP_ID_MP3_ENCODER = 0x0015,
    CAP_ID_SBC_DECODER = 0x0016,
    CAP_ID_MP3_DECODER = 0x0017,
    CAP_ID_AAC_DECODER = 0x0018,
    CAP_ID_APTX_DECODER = 0x0019,
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
    CAP_ID_INTERLEAVE = 0x003E,
    CAP_ID_DEINTERLEAVE = 0x003F,
    CAP_ID_AEC_REFERENCE_1MIC = 0x0040,
    CAP_ID_AEC_REFERENCE_2MIC = 0x0041,
    CAP_ID_AEC_REFERENCE_3MIC = 0x0042,
    CAP_ID_AEC_REFERENCE_4MIC = 0x0043,
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
    CAP_ID_CHANNEL_MIXER = 0x0097,
    CAP_ID_RTP_DECODE = 0x0098,
    CAP_ID_MONO_PASS_BURST = 0x3FF8,
    CAP_ID_MONITOR_OP = 0x3FF9,
    CAP_ID_XOR_PASS = 0x3FFC,
    CAP_ID_STEREO_XOR = 0x3FFD,
    CAP_ID_GARG_PASS = 0x3FFE,
    CAP_ID_STEREO_GARG = 0x3FFF,
    CAP_ID_DOWNLOAD_SELF_TEST = 0x4001,
    CAP_ID_DOWNLOAD_PASSTHROUGH = 0x4002
} CAP_ID;


#define CAP_ID_PRIM_ANY_SIZE 1


#endif /* CAP_ID_PRIM_H */

