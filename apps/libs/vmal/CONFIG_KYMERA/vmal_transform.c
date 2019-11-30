/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    vmal_transform.c

DESCRIPTION
    Create shim version of transform traps
*/

#include <vmal.h>
#include <panic.h>
#include <stream.h>

#define SBC_PAYLOAD_HEADER_SIZE     1
#define MP3_PAYLOAD_HEADER_SIZE     4
#define AAC_PAYLOAD_HEADER_SIZE     0

static void vmalTransformConfigureSbc(Transform t)
{
    PanicFalse(TransformConfigure(t, VM_TRANSFORM_RTP_CODEC_TYPE, 
                                     VM_TRANSFORM_RTP_CODEC_SBC));
    PanicFalse(TransformConfigure(t, VM_TRANSFORM_RTP_PAYLOAD_HEADER_SIZE, 
                                     SBC_PAYLOAD_HEADER_SIZE));
}

static void vmalTransformConfigureMp3(Transform t)
{
    PanicFalse(TransformConfigure(t, VM_TRANSFORM_RTP_CODEC_TYPE, 
                                     VM_TRANSFORM_RTP_CODEC_MP3));
    PanicFalse(TransformConfigure(t, VM_TRANSFORM_RTP_PAYLOAD_HEADER_SIZE, 
                                     MP3_PAYLOAD_HEADER_SIZE));
}

static void vmalTransformConfigureAac(Transform t)
{
    PanicFalse(TransformConfigure(t, VM_TRANSFORM_RTP_CODEC_TYPE, 
                                     VM_TRANSFORM_RTP_CODEC_AAC));
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
