/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    feature_if.h

CONTAINS
    Interface elements between firmware (related to Security Decoder and
    Licensable features) and VM applications.

DESCRIPTION
    This file is seen by VM applications and the firmware.
*/

#ifndef __FEATURE_IF_H__
#define __FEATURE_IF_H__

typedef enum 
{
    /* cVc-1 */
    CVC_RECV,
    CVC_SEND_HS_1MIC,
    CVC_SEND_HS_2MIC_MO,
    CVC_SEND_HS_2MIC_B,
    CVC_SEND_HS_3MIC_MO,
    CVC_SEND_HS_3MIC_BI,
    CVC_SEND_SPKR_1MIC,
    CVC_SEND_SPKR_2MIC,
    CVC_SEND_SPKR_3MIC_BS,
    CVC_SEND_SPKR_4MIC_BS,
    CVC_SEND_SPKR_3MIC_CC,
    CVC_SEND_SPKR_4MIC_CC,

    /* cVc-Earbud */
    CVC_SEND_HS_1MIC_EARBUD,
    CVC_SEND_HS_2MIC_MO_EARBUD,

    /* cVc-2 */
    CVC_SEND_AUTO_1MIC = 24,
    CVC_SEND_AUTO_2MIC,

    /* aptX */
    APTX_CLASSIC = 32,
    APTX_HD,
    APTX_LOWLATENCY,
    APTX_CLASSIC_MONO,

    /* VAD/SVA */
    VAD = 48,
    SVA,

    /* ANC */
    IIR_FILTERS_SIDETONE_PATH = 64,
    ANC_FEED_FORWARD,
    ANC_FEED_BACK,
    ANC_HYBRID,

    /* ANC-Earbud */
    ANC_FEED_FORWARD_EARBUD,
    ANC_FEED_BACK_EARBUD,
    ANC_HYBRID_EARBUD,

    MAX_FEATURES_NUMBER
} feature_id;

#endif /* __FEATURE_IF_H__  */
