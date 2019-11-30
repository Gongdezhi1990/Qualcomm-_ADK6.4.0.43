/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_music_processing.c
DESCRIPTION
        This library implements functionality required for managing the music processing capabilities.
NOTES
*/

#ifndef UNUSED
#define UNUSED(x) ((void)x)
#endif

#include <panic.h>
#include <print.h>
#include <stdlib.h>
#include <string.h>

#include "audio_music_processing.h"
#include <audio_plugin_if.h>
#include <audio_config.h>

#define GET_PARAMS 0x2003

#define GAIA_PARAM_ID_NUM_BANDS (2)
#define GAIA_PARAM_ID_MASTER_GAIN (3)
#define GAIA_PARAMS_PER_BAND (4)

#define UINT32_SIZE (32)

typedef struct
{
    Operator audio_music_operators[max_music_processing_roles];
    Operator mbc_operators[max_music_processing_mbc_roles];
} music_processing_data_t;

static music_processing_data_t music_processing_data =
{
    {0},{0}
};

#define NULL_OPERATOR ((Operator)0)


static bool isMultiBandCompanderRole(const audio_music_processing_role_t role);
static bool isMultiBandCompanderRolesRegistered (void);
static void setMultiBandCompanderProcessingMode(bool enable);
/****************************************************************************
NAME
    audioMusicProcessingGetBandFromId
DESCRIPTION
    Decode user eq band from parameter id */
static uint16 audioMusicProcessingGetBandFromId(const uint16 id)
{
    return (id >> 2) & 0x000f;
}

/****************************************************************************
NAME
    audioMusicProcessingGetParamTypeFromId
DESCRIPTION
    Decode user eq parameter type from parameter id */
static uint16 audioMusicProcessingGetParamTypeFromId(const uint16 id)
{
    return id & 0x0003;
}

/****************************************************************************
NAME
    isNumBands
DESCRIPTION
    Return true if the band/param_type combination indicates Number of Bands is selected
*/
static bool isNumBands(const uint16 band, const eq_param_type_t param_type)
{
    return ((band == 0) && (param_type == 0));
}

/****************************************************************************
NAME
    isMasterGain
DESCRIPTION
    Return true if the band/param_type combination indicates Master Gain is selected
*/
static bool isMasterGain(const uint16 band, const eq_param_type_t param_type)
{
    return ((band == 0) && (param_type == 1));
}

/****************************************************************************
NAME
    audioMusicProcessingMakeParamId
DESCRIPTION
    construct a user eq parameter id from the supplied parameters */
static uint16 audioMusicProcessingMakeParamId(const uint16 band, const eq_param_type_t param_type)
{
    if (isNumBands(band, param_type))
        return GAIA_PARAM_ID_NUM_BANDS;

    if (isMasterGain(band, param_type))
        return GAIA_PARAM_ID_MASTER_GAIN;

    return (uint16)((uint16)(band * GAIA_PARAMS_PER_BAND) + param_type);
}

/****************************************************************************
NAME
    convertFromQFormatTo32bitNumber
DESCRIPTION
    Convert the Q number with N integer bits to a 32 bit value*/
static uint32 convertFromQFormatTo32bitNumber(const uint8 N, const uint32 value)
{
    uint8 M = 0;
    if (N <= UINT32_SIZE)
    {
        M = (uint8)((uint8)UINT32_SIZE - N);
    }
    return value >> M;
}

/****************************************************************************
NAME
    audioMusicProcessingConvertValueToGaia
DESCRIPTION
    Convert the value from the dsp to the correct format required for gaia */
static uint32 audioMusicProcessingConvertValueToGaia(const standard_param_value_t value, const audio_plugin_user_eq_param_id_t param_id)
{
    if (isMasterGain(param_id.band, param_id.param_type))
    {
        /* value is Q 12.N (signed), return format is Gain * 60 */
        return convertFromQFormatTo32bitNumber(14, (uint32)((int32)value * 0.9375));
    }
    else
    {
        switch (param_id.param_type)
        {
            case eq_param_type_filter:
                return value;

            case eq_param_type_cutoff_freq:
                /* value is Q 20.N (uint32), return format is f * 3 */
                return convertFromQFormatTo32bitNumber(22, (uint32)(value * 0.75));

            case eq_param_type_gain:
                /* value is Q 12.N (int32), return format is Gain * 60 */
                return convertFromQFormatTo32bitNumber(18, (uint32)((int32)value * 0.9375));

            case eq_param_type_q:
                /* value is Q 8.N (uint32), return value Q * 4096 */
                return convertFromQFormatTo32bitNumber(20, value);

            default:
                return value;
        }
    }
}

/****************************************************************************
NAME
    convertTo32bitQFormat
DESCRIPTION
    Convert the value to a 32 bit Q number with N integer bits */
static uint32 convertTo32bitQFormat(const uint8 N, const uint32 value)
{
    uint8 M = 0;
    if (N <= UINT32_SIZE)
    {
        M = (uint8)((uint8)UINT32_SIZE - N);
    }
    return value << M;
}

/****************************************************************************
NAME
    audioMusicProcessingConvertValueFromGaia
DESCRIPTION
    Convert the value sent by gaia to the correct format required for kymera */
static uint32 audioMusicProcessingConvertValueFromGaia(const uint32 value, const audio_plugin_user_eq_param_id_t param_id)
{
    if (isMasterGain(param_id.band, param_id.param_type))
    {
        /* value is Gain * 60, required format is Q 12.N (int32) */
        return convertTo32bitQFormat(17, (uint32)((int32)value * 0.5333));
    }
    else
    {
        switch (param_id.param_type)
        {
            case eq_param_type_filter:
                return value;

            case eq_param_type_cutoff_freq:
                /* value is f * 3, required format is Q 20.N (uint32) */
                return convertTo32bitQFormat(21, (uint32)(value * 0.666));

            case eq_param_type_gain:
                /* value is Gain * 60, required format is Q 12.N (int32) */
                return convertTo32bitQFormat(17, (uint32)((int32)value * 0.5333));

            case eq_param_type_q:
                /* value is Q * 4096, required value is Q 8.N (uint32) */
                return convertTo32bitQFormat(20, value);

            default:
                return value;
        }
    }
}

/****************************************************************************
NAME
    audioMusicProcessingSendGetParamRespone
DESCRIPTION
    Send get param response message to audio plugin */
static void audioMusicProcessingSendGetParamResponse(Task audio_plugin, const bool data_valid, const standard_param_t* standard_param)
{
    PRINT(("audioMusicProcessingSendGetParamsResponse: data_valid=%x", (const unsigned) data_valid));
    if (audio_plugin)
    {
        unsigned message_size = sizeof(AUDIO_PLUGIN_GET_USER_EQ_PARAMETER_RESPONSE_MSG_T);
        AUDIO_PLUGIN_GET_USER_EQ_PARAMETER_RESPONSE_MSG_T* message = PanicUnlessMalloc(message_size);
        memset(message, 0, message_size);

        message->data_valid = data_valid;
        if (data_valid)
        {
            message->param[0].id.bank = AudioConfigGetPeqConfig();
            message->param[0].id.band = audioMusicProcessingGetBandFromId(standard_param[0].id);
            message->param[0].id.param_type = audioMusicProcessingGetParamTypeFromId(standard_param[0].id);
            message->param[0].value = audioMusicProcessingConvertValueToGaia(standard_param->value, message->param[0].id);
        }

        MessageSend(audio_plugin, AUDIO_PLUGIN_GET_USER_EQ_PARAMETER_RESPONSE_MSG, message);
    }
}

/****************************************************************************
NAME
    audioMusicProcessingSendGetParamsRespone
DESCRIPTION
    Send get params response message to audio plugin */
static void audioMusicProcessingSendGetParamsResponse(const Task audio_plugin, const bool data_valid, const unsigned number_of_params_sent, const standard_param_t* standard_params)
{
    PRINT(("audioMusicProcessingSendGetParamsResponse: data_valid=%x, number of params=%x\n", (const unsigned) data_valid, number_of_params_sent));
    unsigned number_of_params = (data_valid) ? number_of_params_sent : 0;
    if (audio_plugin)
    {
        unsigned message_size = CALC_MESSAGE_LENGTH_WITH_VARIABLE_PARAMS(AUDIO_PLUGIN_GET_USER_EQ_PARAMETERS_RESPONSE_MSG_T, number_of_params, audio_plugin_user_eq_param_t);
        AUDIO_PLUGIN_GET_USER_EQ_PARAMETERS_RESPONSE_MSG_T* message = PanicUnlessMalloc(message_size);
        memset(message, 0, message_size);
        message->data_valid = data_valid;
        message->number_of_params = (uint16)number_of_params;

        if (data_valid)
        {
            unsigned i = 0;
            for (i = 0; i < number_of_params; i++)
            {
                message->params[i].id.bank = AudioConfigGetPeqConfig();
                message->params[i].id.band = audioMusicProcessingGetBandFromId(standard_params[i].id);
                message->params[i].id.param_type = audioMusicProcessingGetParamTypeFromId(standard_params[i].id);
                message->params[i].value = audioMusicProcessingConvertValueToGaia(standard_params[i].value, message->params[i].id);
            }
        }

        MessageSend(audio_plugin, AUDIO_PLUGIN_GET_USER_EQ_PARAMETERS_RESPONSE_MSG, message);
    }
}

/****************************************************************************
NAME
    isRoleRegistered
DESCRIPTION
    Helper function to determine if a specified role is registered */
static bool isRoleRegistered(const audio_music_processing_role_t role)
{
    return music_processing_data.audio_music_operators[role] != NULL_OPERATOR;
}

/****************************************************************************
NAME
    getProcessingMode
DESCRIPTION
    Helper function to return the processing mode */
static music_processing_mode_t getProcessingMode(const bool enable)
{
    if (enable)
        return music_processing_mode_full_processing;
    else
        return music_processing_mode_passthrough;
}

/****************************************************************************
NAME
    isMultiBandCompanderRole
DESCRIPTION
    Helper function to determine if a specified role is multi-band compander role */
static bool isMultiBandCompanderRole(const audio_music_processing_role_t role)
{
    compander_band_selection_t type = AudioConfigGetMusicProcessingCompanderBand();

    return ((role == audio_music_processing_compander_role) &&
            ((type == two_band_compander) ||(type == three_band_compander)) );
}

static Operator getOperator(audio_music_processing_mbc_t role)
{
    return music_processing_data.mbc_operators[role];
}

/****************************************************************************
NAME
    isMultiBandCompanderRolesRegistered
DESCRIPTION
    Helper function to determine if a specified role is registered */
static bool isMultiBandCompanderRolesRegistered (void)
{
    if(AudioConfigGetMusicProcessingCompanderBand() == two_band_compander)
    {
        return ((getOperator(audio_music_processing_2band_mbc_crossover_role) != NULL_OPERATOR) &&
                (getOperator(audio_music_processing_2band_mbc_low_freq_compander_role) != NULL_OPERATOR) &&
                (getOperator(audio_music_processing_2band_mbc_high_freq_compander_role) != NULL_OPERATOR));
    }

    if(AudioConfigGetMusicProcessingCompanderBand() == three_band_compander)
    {
        return ((getOperator(audio_music_processing_3band_mbc_first_crossover_role) != NULL_OPERATOR) &&
                (getOperator(audio_music_processing_3band_mbc_peq_role) != NULL_OPERATOR) &&
                (getOperator(audio_music_processing_3band_mbc_low_freq_compander_role) != NULL_OPERATOR) &&
                (getOperator(audio_music_processing_3band_mbc_second_crossover_role) != NULL_OPERATOR) &&
                (getOperator(audio_music_processing_3band_mbc_mid_freq_compander_role) != NULL_OPERATOR) &&
                (getOperator(audio_music_processing_3band_mbc_high_freq_compander_role) != NULL_OPERATOR));
    }

    return FALSE;
}

static void set2BandCompanderMode(music_processing_mode_t mode)
{
    /* Mute high freq path incase requested mode is passthrough */
    music_processing_mode_t secondary_path_mode = (mode == music_processing_mode_passthrough) ? music_processing_mode_standby : mode;

    OperatorsSetMusicProcessingMode(getOperator(audio_music_processing_2band_mbc_crossover_role), mode);
    OperatorsSetMusicProcessingMode(getOperator(audio_music_processing_2band_mbc_low_freq_compander_role), mode);
    OperatorsSetMusicProcessingMode(getOperator(audio_music_processing_2band_mbc_high_freq_compander_role), secondary_path_mode);
}

static void set3BandCompanderMode(music_processing_mode_t mode)
{
    /* Mute mid and high freq path incase requested 3band compander mode is passthrough */
    music_processing_mode_t secondary_path_mode = (mode == music_processing_mode_passthrough) ? music_processing_mode_standby : mode;

    OperatorsSetMusicProcessingMode(getOperator(audio_music_processing_3band_mbc_first_crossover_role), mode);
    OperatorsSetMusicProcessingMode(getOperator(audio_music_processing_3band_mbc_peq_role), mode);
    OperatorsSetMusicProcessingMode(getOperator(audio_music_processing_3band_mbc_low_freq_compander_role), mode);
    OperatorsSetMusicProcessingMode(getOperator(audio_music_processing_3band_mbc_second_crossover_role), secondary_path_mode);
    OperatorsSetMusicProcessingMode(getOperator(audio_music_processing_3band_mbc_mid_freq_compander_role), secondary_path_mode);
    OperatorsSetMusicProcessingMode(getOperator(audio_music_processing_3band_mbc_high_freq_compander_role), secondary_path_mode);
}

/****************************************************************************
NAME
    setMultiBandCompanderProcessingMode
DESCRIPTION
    Helper function to set processing mode */
static void setMultiBandCompanderProcessingMode(bool enable)
{
    music_processing_mode_t mode = getProcessingMode(enable);

    if(AudioConfigGetMusicProcessingCompanderBand() == two_band_compander)
        set2BandCompanderMode(mode);

    if(AudioConfigGetMusicProcessingCompanderBand() == three_band_compander)
        set3BandCompanderMode(mode);
}

void AudioMusicProcessingRegisterRole(audio_music_processing_role_t role, Operator op)
{
    /* Currently we only support 1 set of music processing roles
     * This is a limitation in the implementation only, not in dsp capabilities */
    if (isRoleRegistered(role))
        Panic();
    music_processing_data.audio_music_operators[role] = op;
}

void AudioMusicProcessingUnregisterRole(audio_music_processing_role_t role)
{
    music_processing_data.audio_music_operators[role] = NULL_OPERATOR;
}

void AudioMusicProcessingEnableProcessing(audio_music_processing_role_t role, bool enable)
{
    if (isRoleRegistered(role))
    {
        OperatorsSetMusicProcessingMode(music_processing_data.audio_music_operators[role], getProcessingMode(enable));
    }
    else if(isMultiBandCompanderRole(role) && isMultiBandCompanderRolesRegistered())
    {
        setMultiBandCompanderProcessingMode(enable);
    }
    else
    {
        PRINT(("AudioMusicProcessing EnableProcessing: Role not registered 0x%x\n", (unsigned)role));
    }
}

void AudioMusicProcessingRegisterMultiBandCompanderRole(audio_music_processing_mbc_t role, Operator op)
{
    if (isMultiBandCompanderRolesRegistered())
        Panic();

    music_processing_data.mbc_operators[role] = op;
}

void AudioMusicProcessingUnregisterMultiBandCompanderRole(audio_music_processing_mbc_t role)
{
    music_processing_data.mbc_operators[role] = NULL_OPERATOR;
}

static ucid_peq_t convertConfigToUcid(peq_config_t config)
{
    switch (config)
    {
        case peq_config_0:
            return ucid_peq_resampler_0;

        case peq_config_1:
            return ucid_peq_resampler_1;

        case peq_config_2:
            return ucid_peq_resampler_2;

        case peq_config_3:
            return ucid_peq_resampler_3;

        case peq_config_4:
            return ucid_peq_resampler_4;

        case peq_config_5:
            return ucid_peq_resampler_5;

        case peq_config_6:
            return ucid_peq_resampler_6;

        case peq_config_default:
        default:
            Panic();
            break;
    }
    /* Keep compiler happy */
    return 0;
}

static bool isUserPeqConfigUpdateRequired(peq_config_t requested_config)
{
    peq_config_t current_config = AudioConfigGetPeqConfig();
	
    return ((current_config != requested_config) && (requested_config != peq_config_default));
}

void AudioMusicProcessingSelectUserPeqConfig(peq_config_t config)
{
    /* Once a user config is selected it is not possible to directly select the default config again */
    /* A workaround is to have a user config with NUM_STAGES = 0 and select that. */
    audio_music_processing_role_t peq_role = audio_music_processing_user_peq_role;
    if (isRoleRegistered(peq_role))
    {
        if(isUserPeqConfigUpdateRequired(config))
        {
            AudioConfigSetPeqConfig(config);
            ucid_peq_t ucid = convertConfigToUcid(config);
            OperatorsStandardSetUCID(music_processing_data.audio_music_operators[peq_role], (unsigned)ucid);
        }
        else
        {
            PRINT(("AudioMusicProcessing SelectUserPeqConfig: User PEQ is not updated\n"));
        }
    }
    else
    {
        PRINT(("AudioMusicProcessing SelectUserPeqConfig: User PEQ role not registered\n"));
    }
}

peq_config_t AudioMusicProcessingGetPeqConfig(void)
{
    return AudioConfigGetPeqConfig();
}

void AudioMusicProcessingSetUserEqParameter(const audio_plugin_user_eq_param_t* param)
{
    AudioConfigSetUserEqParameter(param);
}

void AudioMusicProcessingApplyUserEqParameters(void)
{
    const audio_music_processing_role_t peq_role = audio_music_processing_user_peq_role;
    const Operator peq_op = music_processing_data.audio_music_operators[peq_role];
    if (peq_op)
    {
        unsigned number_of_params = AudioConfigGetNumberOfEqParams();
        if (number_of_params > 0)
        {
            set_params_data_t* set_params_data = OperatorsCreateSetParamsData(number_of_params);

            unsigned i;
            for (i = 0; i < number_of_params; i++)
            {
                audio_plugin_user_eq_param_t* param = AudioConfigGetUserEqParameter(i);
                set_params_data->standard_params[i].id = audioMusicProcessingMakeParamId(param->id.band, param->id.param_type);
                set_params_data->standard_params[i].value = audioMusicProcessingConvertValueFromGaia(param->value, param->id);
            }
            AudioConfigClearUserEqParams();

            OperatorsStandardSetParameters(music_processing_data.audio_music_operators[peq_role], set_params_data);
            free(set_params_data);
        }
    }
}

void AudioMusicProcessingClearUserEqParameters(void)
{
    AudioConfigClearUserEqParams();
}

/****************************************************************************
NAME
    createGetParamsData
DESCRIPTION
    Allocate and populate a get_params_data structure with the supplied parameter ids. This structure
    is passed to OperatorsGetParameters() and the parameter values are filled in by this function.
*/
static get_params_data_t* createGetParamsData(const unsigned number_of_params, const audio_plugin_user_eq_param_id_t* param_ids)
{
    unsigned i;
    get_params_data_t* get_params_data = OperatorsCreateGetParamsData(number_of_params);

    for (i = 0; i < number_of_params; i++)
    {
        get_params_data->standard_params[i].id = audioMusicProcessingMakeParamId(param_ids[i].band, param_ids[i].param_type);
    }
    return get_params_data;
}

/****************************************************************************
NAME
    isGetParamsResponseValid
DESCRIPTION
    Decode the dsp get params response into a valid/invalid value
*/
static bool isGetParamsResponseValid(obpm_result_state_t result)
{
    if (result == obpm_ok)
        return TRUE;
    /* A result of obpm_not_ready implies the dsp has returned default parameters, and the data is valid */
    if (result == obpm_not_ready)
        return TRUE;
    return FALSE;
}

/****************************************************************************
NAME
    areRequestedParamsInCurrentBank
DESCRIPTION
    When requesting dsp eq parameters it is possible to request values from a bank other than the selected bank.
    On crescendo it is not possible to access these values. This function returns a boolean value indicating if
    the requested values are accessible or not.
*/
static bool areRequestedParamsInCurrentBank(const unsigned number_of_params, const audio_plugin_user_eq_param_id_t* param_ids)
{
    unsigned i;
    const unsigned current_eq_bank = AudioConfigGetPeqConfig();
    for (i = 0; i < number_of_params; i++)
    {
        if (param_ids[i].bank != current_eq_bank)
            return FALSE;
    }
    return TRUE;
}

static get_params_data_t* createStandardGetParamsFromParamIds(const unsigned number_of_params, const audio_plugin_user_eq_param_id_t* param_ids)
{
    const Operator peq_op = music_processing_data.audio_music_operators[audio_music_processing_user_peq_role];

    get_params_data_t* get_params_data = createGetParamsData(number_of_params, param_ids);
    if (areRequestedParamsInCurrentBank(number_of_params, param_ids))
    {
        OperatorsStandardGetParameters(peq_op, get_params_data);
    }
    else
    {
        get_params_data->result = obpm_invalid_parameter;
    }
    return get_params_data;
}

void AudioMusicProcessingGetUserEqParameter(const audio_plugin_user_eq_param_id_t* param_id, Task audio_plugin)
{
    if ((param_id != NULL) && (audio_plugin != NULL))
    {
        const unsigned number_of_params = 1;
        get_params_data_t* get_params_data = createStandardGetParamsFromParamIds(number_of_params, param_id);
        const bool data_valid = isGetParamsResponseValid(get_params_data->result);
        audioMusicProcessingSendGetParamResponse(audio_plugin, data_valid, get_params_data->standard_params);
        free(get_params_data);
    }
}

void AudioMusicProcessingGetUserEqParameters(const unsigned number_of_params, const audio_plugin_user_eq_param_id_t* param_ids, Task audio_plugin)
{
    if ((number_of_params > 0) && (param_ids != NULL) && (audio_plugin != NULL))
    {
        get_params_data_t* get_params_data = createStandardGetParamsFromParamIds(number_of_params, param_ids);
        const bool data_valid = isGetParamsResponseValid(get_params_data->result);
        audioMusicProcessingSendGetParamsResponse(audio_plugin, data_valid, number_of_params, get_params_data->standard_params);
        free(get_params_data);
    }
}

#ifdef HOSTED_TEST_ENVIRONMENT
/****************************************************************************
DESCRIPTION
    Reset any static variables
    This is only intended for unit test and will panic if called in a release build.
*/
void AudioMusicProcessingTestReset(void)
{
    audio_music_processing_role_t role;
    audio_music_processing_mbc_t mbc_role;

    for (role = audio_music_processing_compander_role; role < max_music_processing_roles; role++)
        AudioMusicProcessingUnregisterRole(role);

    for (mbc_role = audio_music_processing_2band_mbc_crossover_role; mbc_role < max_music_processing_mbc_roles; mbc_role++)
        AudioMusicProcessingUnregisterMultiBandCompanderRole(mbc_role);
}
#endif
