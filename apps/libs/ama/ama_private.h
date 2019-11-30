/*****************************************************************************

Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_private.h

DESCRIPTION
    Private AMA data and functions shared throughout the library.

*********************************************************************************/
#ifndef _AMA_PRIVATE_H_
#define _AMA_PRIVATE_H_

#include "ama.h"

#define NUMBER_OF_SUPPORTED_TRANSPORTS (3)

#define AMA_Feature_Auxiliary_Connected                   0x100
#define AMA_Feature_Bluetooth_A2DP_Enabled                0x130
#define AMA_Feature_Bluetooth_HFP_Enabled                 0x131
#define AMA_Feature_Bluetooth_A2DP_Connected              0x132
#define AMA_Feature_Bluetooth_HFP_Connected               0x133
#define AMA_Feature_Bluetooth_Classic_Discoverable        0x134

#define AMA_Feature_Device_Calibration_Required           0x200
#define AMA_Feature_Device_Theme                          0x201
#define AMA_Feature_Device_DND_Enabled                    0x202
#define AMA_Feature_Device_Cellular_Connectivity_Status   0x203


#define AMA_Feature_Message_Notification                  0x300
#define AMA_Feature_Call_Notification                     0x301
#define AMA_Feature_Remote_Notification                   0x302

#define AMA_Feature_INVALID                               0xFFFF

/* AMA TO-DO cg11 the following copies stuff defined in constants-ph.h.h */
typedef enum {
    ama_error_code_success = 0,
    ama_error_code_unknown = 1,
    ama_error_code_internal = 2,
    ama_error_code_unsupported = 3,
    ama_error_code_user_cancelled = 4,
    ama_error_code_not_found = 5,
    ama_error_code_invalid = 6,
    ama_error_code_busy = 7
} ama_error_code_t;

typedef enum {
    AMA_STATE_VALUE_NOT_SET = 0,
    AMA_STATE_VALUE_BOOLEAN = 2,
    AMA_STATE_VALUE_INTEGER = 3
} ama_state_value_case_t;

typedef enum _AMA_MEDIA_CONTROL {
    AMA_MEDIA_CONTROL_PLAY ,
    AMA_MEDIA_CONTROL_PAUSE,
    AMA_MEDIA_CONTROL_NEXT,
    AMA_MEDIA_CONTROLPREVIOUS
}AMA_MEDIA_CONTROL;

/* macro for allocating message structures */
#define MAKE_AMA_MESSAGE(TYPE) MESSAGE_MAKE(message,TYPE##_T);

void amaDisconnect(void);

void amaSendSinkMessage(ama_message_type_t id, void* data);

void AmaSendKeepAlive(void);

void amaMediaControl(AMA_MEDIA_CONTROL control);


/***************************************************************************
DESCRIPTION
    Returns the ama device configuration information.

PARAMS
    None

RETURNS
    pointer to the ama device configuration.
*/
ama_device_config_t * amaGetDeviceConfiguration(void);

#endif /* _AMA_PRIVATE_H_ */

