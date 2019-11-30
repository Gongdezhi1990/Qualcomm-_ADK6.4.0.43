/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_hardware_aec_chain_config.h

DESCRIPTION
       Implementation of AEC chain construction functions
*/

#ifndef _AUDIO_HARDWARE_AEC_CHAIN_CONFIGS_H_
#define _AUDIO_HARDWARE_AEC_CHAIN_CONFIGS_H_

#include <chain.h>

typedef enum _audio_hardware_aec_roles
{
    aec_role_aec
} audio_hardware_aec_roles_t;

/****************************************************************************
NAME
    hardwareAecGetChainConfig
DESCRIPTION
    Get the chain configuration for the AEC object
RETURNS
    AEC chain configuration
*/
const chain_config_t *hardwareAecGetChainConfig(void);

#endif /* _AUDIO_HARDWARE_AEC_CHAIN_CONFIGS_H_ */
