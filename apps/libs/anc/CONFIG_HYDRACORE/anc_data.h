/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_data.h

DESCRIPTION
    Encapsulation of the ANC VM Library data.
*/

#ifndef ANC_DATA_H_
#define ANC_DATA_H_

#include "anc.h"
#include "anc_sm.h"
#include "anc_config_data.h"

#include <csrtypes.h>

/*! @brief The ANC fine tune gain step value that can be set using the library.

    Used by the ANC library to cycle through the fine tune gain setting.
 */
typedef enum
{
    anc_fine_tune_gain_step_min,
    anc_fine_tune_gain_step_default = anc_fine_tune_gain_step_min,
    anc_fine_tune_gain_step_positive,
    anc_fine_tune_gain_step_negative,
    anc_fine_tune_gain_step_max = anc_fine_tune_gain_step_negative
} anc_fine_tune_gain_step_t;

/******************************************************************************
NAME
    ancDataInitialise

DESCRIPTION
    This function is responsible allocating any required memory for the library
    data and must be called before any other library data function is called.

RETURNS
    bool indicating if the library data was successfully initialised.
*/
bool ancDataInitialise(void);

/******************************************************************************
NAME
    ancDataDeinitialise

DESCRIPTION
    This function is responsible for de-initialising the library
    data.

RETURNS
    bool indicating if the memory for library data was successfully deinitialised.
*/
bool ancDataDeinitialise(void);

/******************************************************************************
NAME
    ancDataSetState/ancDataGetState

DESCRIPTION
    Simple setter and getter functions for the ANC VM library state.
*/
void ancDataSetState(anc_state state);
anc_state ancDataGetState(void);


/******************************************************************************
NAME
    ancDataSetMicParams/ancDataGetMicParams

DESCRIPTION
    Simple setter and getter functions for the ANC Microphone configuration.
*/
void ancDataSetMicParams(anc_mic_params_t *mic_params);
anc_mic_params_t* ancDataGetMicParams(void);


/******************************************************************************
NAME
    ancDataSetMode/ancDataGetMode

DESCRIPTION
    Simple setter and getter functions for the ANC Mode.
*/
bool ancDataSetMode(anc_mode_t mode);
anc_mode_t ancDataGetMode(void);

/******************************************************************************
NAME
    ancDataGetConfigData

DESCRIPTION
    Simple getter function for the ANC config data.
*/
anc_config_t * ancDataGetConfigData(void);

/******************************************************************************
NAME
    ancDataGetCurrentModeConfig

DESCRIPTION
    Simple getter function for the config of the current ANC mode.
*/
anc_mode_config_t * ancDataGetCurrentModeConfig(void);

/******************************************************************************
NAME
    ancDataSetFineTuneGain/ancDataGetFineTuneGain

DESCRIPTION
    Simple setter and getter functions for the ANC Fine Tune Gain.
*/
void ancDataSetFineTuneGain(anc_fine_tune_gain_step_t step);
anc_fine_tune_gain_step_t ancDataGetFineTuneGain(void);

/******************************************************************************
NAME
    ancDataRetrieveAndPopulateTuningData

DESCRIPTION
    Function responsible for reading tuning tool params into ANC config data
*/
bool ancDataRetrieveAndPopulateTuningData(anc_mode_t set_mode);

/******************************************************************************
NAME
    ancDataIsLeftChannelConfigurable
    ancDataIsRightChannelConfigurable

DESCRIPTION
    Function to determine whether left/right channels are to be configured
*/
bool ancDataIsLeftChannelConfigurable(void);
bool ancDataIsRightChannelConfigurable(void);

/******************************************************************************
NAME
    ancConfigDataGetMicForMicPath

DESCRIPTION
    Function responsible for returning the mic config for a particular ANC mic
*/
audio_mic_params * ancConfigDataGetMicForMicPath(anc_path_enable mic_path);

/******************************************************************************
NAME
    ancConfigDataGetHardwareGainForMicPath

DESCRIPTION
    Function responsible for returning the hardware gain of a particular ANC mic
*/
uint32 ancConfigDataGetHardwareGainForMicPath(anc_path_enable mic_path);

#endif
