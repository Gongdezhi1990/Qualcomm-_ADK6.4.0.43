/*****************************************************************************

            (c) Qualcomm Technologies International, Ltd. 2017
            Confidential information of Qualcomm

            Refer to LICENSE.txt included with this source for details
            on the license terms.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef STREAM_PRIM_H
#define STREAM_PRIM_H


#define STREAM_EP_CHAN_POSN                                             (0)
#define STREAM_EP_EXT_ID_POSN                                           (0)
#define STREAM_EP_CHAN_MASK                                             (0x003f)
#define STREAM_EP_OPID_MASK                                             (0x1fc0)
#define STREAM_EP_EXT_ID_MASK                                           (0x1fff)
#define STREAM_EP_EP_BIT                                                (0x2000)
#define STREAM_EP_OP_BIT                                                (0x4000)
#define STREAM_EP_SINK_BIT                                              (0x8000)
#define STREAM_EP_SHADOW_ID_MASK                                        (0x9fff)
#define STREAM_EP_OPID_POSN                                             (6)
#define STREAM_EP_EXT_SOURCE                                            (STREAM_EP_EP_BIT)
#define STREAM_EP_OP_SOURCE                                             (STREAM_EP_EP_BIT | STREAM_EP_OP_BIT)
#define STREAM_EP_OP_SINK                                               (STREAM_EP_EP_BIT | STREAM_EP_OP_BIT | STREAM_EP_SINK_BIT)
#define STREAM_EP_EXT_SINK                                              (STREAM_EP_EP_BIT | STREAM_EP_SINK_BIT)
#define STREAM_EP_SHADOW_MASK                                           (STREAM_EP_OP_BIT | STREAM_EP_EP_BIT)
#define STREAM_EP_SHADOW_TYPE_MASK                                      (STREAM_EP_OP_BIT | STREAM_EP_EP_BIT)
#define STREAM_EP_TYPE_MASK                                             (STREAM_EP_SINK_BIT | STREAM_EP_OP_BIT | STREAM_EP_EP_BIT)


/*******************************************************************************

  NAME
    STREAM_DEVICE

  DESCRIPTION
    Stream device type IDs. These are common across BC and Hydra platforms.
    These are also defined in
    //depot/bc/main/interface/app/stream/stream_if.h ("stream_device") and
    need to remain aligned. If you need to allocate a new number, check there
    first as well as here, as someone may have allocated a new number there
    without updating this file. (We also have another copy of a subset of
    this information in common/hydra/interface/accmd_types.xml) FASTPIPE =
    0x0008 on BlueCore I2C = 0x000D on BlueCore

 VALUES
    PCM             -
    I2S             -
    CODEC           -
    SPDIF           -
    DIGITAL_MIC     -
    SHUNT           -
    SCO             -
    FILE            -
    APPDATA         -
    L2CAP           -
    USB_ENDPOINT    -
    CAP_DOWNLOAD    -
    PWM             -
    TIMESTAMPED     -
    FILE_MANAGER    -
    OPERATOR        -
    SIMULATION_FILE -
    TESTER          - Endpoint that provides mechanisms to test the audio data
                      service.
    RAW_BUFFER      - Endpoint that provides a raw buffer without flow
                      control/kicks. If this endpoint is connected to a PCM
                      interface, the data in in the buffer will be looped around
                      and played out.

*******************************************************************************/
typedef enum
{
    STREAM_DEVICE_PCM = 0x0001,
    STREAM_DEVICE_I2S = 0x0002,
    STREAM_DEVICE_CODEC = 0x0003,
    STREAM_DEVICE_SPDIF = 0x0005,
    STREAM_DEVICE_DIGITAL_MIC = 0x0006,
    STREAM_DEVICE_SHUNT = 0x0007,
    STREAM_DEVICE_SCO = 0x0009,
    STREAM_DEVICE_FILE = 0x000A,
    STREAM_DEVICE_APPDATA = 0x000B,
    STREAM_DEVICE_L2CAP = 0x000C,
    STREAM_DEVICE_USB_ENDPOINT = 0x000E,
    STREAM_DEVICE_CAP_DOWNLOAD = 0x000F,
    STREAM_DEVICE_PWM = 0x0010,
    STREAM_DEVICE_TIMESTAMPED = 0x0011,
    STREAM_DEVICE_FILE_MANAGER = 0x0012,
    STREAM_DEVICE_OPERATOR = 0x001E,
    STREAM_DEVICE_SIMULATION_FILE = 0x007D,
    STREAM_DEVICE_TESTER = 0x007E,
    STREAM_DEVICE_RAW_BUFFER = 0x007F
} STREAM_DEVICE;


#define STREAM_PRIM_ANY_SIZE 1


#endif /* STREAM_PRIM_H */

