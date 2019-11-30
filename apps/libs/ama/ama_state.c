#include <bdaddr.h>
#include <boot.h>
#include <connection.h>
#include <file.h>
#include <kalimba.h>
#include <kalimba_standard_messages.h>
#include <led.h>
#include <message.h>
#include <panic.h>
#include <pio.h>
#include <region.h>
#include <service.h>
#include <sink.h>
#include <source.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stream.h>
#include <util.h>
#include <vm.h>
#include <psu.h>
#include <charger.h>
#include <a2dp.h>
#include <audio.h>
#include <audio_plugin_music_variants.h>
#include <transform.h>
#include <loader.h>
#include <partition.h>
#include <micbias.h>
#include <vmal.h>
#include <audio.h>
#include <gatt_ama_server.h>
#include "ama_send_command.h"

#include <ama.h>

#include "ama_debug.h"
#include "ama_state.h"

typedef struct __AmaFeatureState
{
    bool auxiliaryConnected;
    bool bluetoothA2dpEnabled;
    bool bluetoothHfpEnabled;

}AmaFeatureState;

static AmaFeatureState amaFeatureState;

void amaStateInit(void)
{
    amaFeatureState.auxiliaryConnected = FALSE;
    amaFeatureState.bluetoothA2dpEnabled = TRUE;
    amaFeatureState.bluetoothHfpEnabled = TRUE;
}

ama_error_code_t amaLibGetState(uint32 feature, uint32* Pstate, ama_state_value_case_t* pValueCase)
{
    *pValueCase = AMA_STATE_VALUE_NOT_SET;

    ama_error_code_t errorCode = ama_error_code_success;

    switch(feature)
    {
        case AMA_Feature_Auxiliary_Connected:
            *Pstate = (uint32)amaFeatureState.auxiliaryConnected;
            *pValueCase = AMA_STATE_VALUE_BOOLEAN;
            break;

        case AMA_Feature_Bluetooth_A2DP_Enabled:
            *Pstate = (uint32)amaFeatureState.bluetoothA2dpEnabled;
            *pValueCase = AMA_STATE_VALUE_BOOLEAN;
            break;

        case AMA_Feature_Bluetooth_HFP_Enabled:
            *Pstate = (uint32)amaFeatureState.bluetoothHfpEnabled;
            *pValueCase = AMA_STATE_VALUE_BOOLEAN;
            break;

        case AMA_Feature_Bluetooth_A2DP_Connected:
            // TBD *Pstate = (uint32)amaFeatureState.bluetoothHfpEnabled;
            *pValueCase = AMA_STATE_VALUE_BOOLEAN;
            break;

        case AMA_Feature_Bluetooth_HFP_Connected:
            // TBD *Pstate = (uint32)amaFeatureState.bluetoothHfpEnabled;
            *pValueCase = AMA_STATE_VALUE_BOOLEAN;
            break;
        case AMA_Feature_Bluetooth_Classic_Discoverable:
            *pValueCase = AMA_STATE_VALUE_BOOLEAN;
            break;
        case AMA_Feature_Device_Calibration_Required:
            *pValueCase = AMA_STATE_VALUE_BOOLEAN;
            break;

        case AMA_Feature_Device_Theme:
            *pValueCase = AMA_STATE_VALUE_INTEGER;
            break;


        /* cannot get state for the features below */
        case AMA_Feature_Device_DND_Enabled:
        case AMA_Feature_Device_Cellular_Connectivity_Status:
        case AMA_Feature_Message_Notification:
        case AMA_Feature_Remote_Notification:
        case AMA_Feature_Call_Notification:
            errorCode = ama_error_code_unsupported;
            break;


       default:
            errorCode = ama_error_code_invalid;
            break;

        }


    return errorCode;
}

ama_error_code_t amaLibSetState(uint32 feature, uint32 state, ama_state_value_case_t valueCase)
{
    ama_error_code_t errorCode = ama_error_code_success;

    UNUSED(state);  //TBD, we need to set it :)

    if(valueCase != AMA_STATE_VALUE_BOOLEAN &&
       valueCase != AMA_STATE_VALUE_INTEGER)
    {

        return ama_error_code_unsupported;
    }

    switch(feature)
    {
        case AMA_Feature_Bluetooth_A2DP_Enabled:
        case AMA_Feature_Bluetooth_HFP_Enabled:
        case AMA_Feature_Bluetooth_Classic_Discoverable:
        case AMA_Feature_Device_Calibration_Required:

        case AMA_Feature_Device_Theme:
             //   *pValueCase = AMA_STATE_VALUE_INTEGER;
        break;


       case AMA_Feature_Auxiliary_Connected:
       case AMA_Feature_Bluetooth_A2DP_Connected:
       case AMA_Feature_Bluetooth_HFP_Connected:

       errorCode = ama_error_code_unsupported;
       break;


       default:
       break;
    }

    return errorCode;
}



bool AmaLibSendIntegerStateEvent(uint32 feature ,uint16 integer, bool get)
{
    bool ret = TRUE;

    switch(feature)
    {
        case    AMA_Feature_Device_Theme:
        case    AMA_Feature_Device_Cellular_Connectivity_Status:
        case    AMA_Feature_Message_Notification:
        case    AMA_Feature_Call_Notification:
        case    AMA_Feature_Remote_Notification:
            if(get == TRUE)
            {
                amaSendCommandGetState(feature);
            }
            else
            {
                /* send sync */
                amaSendCommandSyncState(feature, AMA_STATE_VALUE_INTEGER, integer);
            }
        break;

        default:
            ret = FALSE;
        break;
    }

    return ret;
}






bool AmaLibSendBooleanStateEvent(uint32 feature,bool True, bool get)
{
    bool ret = TRUE;

    switch(feature)
    {
        case    AMA_Feature_Device_DND_Enabled:
            if(get == TRUE)
            {
                amaSendCommandGetState(feature);
            }
            else
            {
                amaSendCommandSyncState(feature, AMA_STATE_VALUE_BOOLEAN, True);
            }

        break;

        default:
            ret = FALSE;
        break;
    }

    return ret;
}

