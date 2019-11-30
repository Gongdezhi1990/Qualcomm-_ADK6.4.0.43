/*******************************************************************************
Copyright (c) 2015 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    vmal_transform.c

DESCRIPTION
    Create shim version of transform traps
*/

#include <vmal.h>
#include <panic.h>


#define AAC_PAYLOAD_HEADER_SIZE           0
#define APTXHD_PAYLOAD_HEADER_SIZE     0

static void vmalTransformConfigureSbc(Transform t)
{
    PanicFalse(TransformConfigure(t, VM_TRANSFORM_RTP_CODEC_TYPE, 
                                     VM_TRANSFORM_RTP_CODEC_SBC));
}

static void vmalTransformConfigureMp3(Transform t)
{
    PanicFalse(TransformConfigure(t, VM_TRANSFORM_RTP_CODEC_TYPE, 
                                     VM_TRANSFORM_RTP_CODEC_MP3));
}

static void vmalTransformConfigureAac(Transform t)
{
    PanicFalse(TransformConfigure(t, VM_TRANSFORM_RTP_CODEC_TYPE, 
                                     VM_TRANSFORM_RTP_CODEC_AAC));
    PanicFalse(TransformConfigure(t, VM_TRANSFORM_RTP_PAYLOAD_HEADER_SIZE, 
                                     AAC_PAYLOAD_HEADER_SIZE));
}

static void vmalTransformConfigureAptxHD(Transform t)
{
    PanicFalse(TransformConfigure(t, VM_TRANSFORM_RTP_PAYLOAD_HEADER_SIZE, 
                                     APTXHD_PAYLOAD_HEADER_SIZE));
}

static void vmalTransformConfigureAptx(Transform t)
{
    PanicFalse(TransformConfigure(t, VM_TRANSFORM_RTP_CODEC_TYPE, 
                                     VM_TRANSFORM_RTP_CODEC_APTX));
    PanicFalse(TransformConfigure(t, VM_TRANSFORM_RTP_PAYLOAD_HEADER_SIZE, 
                                     APTX_PAYLOAD_HEADER_SIZE));
}

Transform VmalTransformRtpSbcDecode(Source source, Sink sink)
{
    Transform t = TransformRtpDecode(source, sink);
    vmalTransformConfigureSbc(t);
    return t;
}

Transform VmalTransformRtpSbcEncode(Source source, Sink sink)
{
    Transform t = TransformRtpEncode(source, sink);
    vmalTransformConfigureSbc(t);
    return t;
}
Transform VmalTransformRtpAptxEncode(Source source, Sink sink)
{
    Transform t = TransformRtpEncode(source, sink);
    vmalTransformConfigureAptx(t);
    return t;
}
Transform VmalTransformRtpMp3Decode(Source source, Sink sink)
{
    Transform t = TransformRtpDecode(source, sink);
    vmalTransformConfigureMp3(t);
    return t;
}

Transform VmalTransformRtpMp3Encode(Source source, Sink sink)
{
    Transform t = TransformRtpEncode(source, sink);
    vmalTransformConfigureMp3(t);
    return t;
}

Transform VmalTransformRtpAacDecode(Source source, Sink sink)
{
    Transform t = TransformRtpDecode(source, sink);
    vmalTransformConfigureAac(t);
    return t;
}

Transform VmalTransformRtpAptxHdDecode(Source source, Sink sink)
{
    Transform t = TransformRtpDecode(source, sink);
    vmalTransformConfigureAptxHD(t);
    return t;
}

Transform VmalTransformRtpAptxDecode(Source source, Sink sink)
{
    Transform t = TransformRtpDecode(source, sink);
    vmalTransformConfigureAptx(t);
    return t;
}

