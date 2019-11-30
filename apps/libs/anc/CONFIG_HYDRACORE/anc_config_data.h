/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_config_data.h

DESCRIPTION
    Architecture specific config data.
*/

#ifndef ANC_CONFIG_DATA_H_
#define ANC_CONFIG_DATA_H_

#include "anc.h"
#include <csrtypes.h>

enum
{
    ANC_INSTANCE_0_INDEX,
    ANC_INSTANCE_1_INDEX,
    ANC_INSTANCE_MAX_INDEX = ANC_INSTANCE_1_INDEX
};

#define NUMBER_OF_ANC_INSTANCES            2
#define NUMBER_OF_IIR_COEFFICIENTS        15


/* ANC Instance config */

typedef struct
{
    uint16 coefficients[NUMBER_OF_IIR_COEFFICIENTS];
} iir_config_t;

typedef struct
{
    uint16 lpf_shift1:4; /* valid values 1-9 */
    uint16 lpf_shift2:4;
    uint16 unused:8;
} lpf_config_t;

typedef struct
{
    lpf_config_t lpf_config;
    iir_config_t iir_config;
} filter_path_config_t;


/* ANC audio path config */

typedef struct
{
    uint16 filter_shift:4; /* valid values 0-11 */
    uint16 filter_enable:1; /* valid values 0/1 */
    uint16 unused:11;
} dc_filter_config_t;

typedef dc_filter_config_t small_lpf_config_t;

typedef struct
{
    uint32 dmic_x2_ff;
} dmic_x2_config_t;

typedef struct
{
    uint16 gain:8; /* valid values 0-255 */
    uint16 unused:8;
    uint16 gain_shift; /* 4 bits, (-4)-(+7) */
} gain_config_t;

typedef struct
{
    iir_config_t iir_config;
    lpf_config_t lpf_config;
    dc_filter_config_t dc_filter_config;
    dmic_x2_config_t upconvertor_config;
    gain_config_t gain_config;
} feed_forward_path_config_t;

typedef struct
{
    iir_config_t iir_config;
    lpf_config_t lpf_config;
    gain_config_t gain_config;
} feed_back_path_config_t;

typedef struct
{
    small_lpf_config_t small_lpf_config;
} small_lpf_path_config_t;

typedef struct
{
    feed_forward_path_config_t feed_forward_a;
    feed_forward_path_config_t feed_forward_b;
    feed_back_path_config_t feed_back;
    small_lpf_path_config_t small_lpf;

    uint16 enable_mask:4;
    uint16 unused:12;
} anc_instance_config_t;

typedef struct
{
    uint32 feed_forward_mic_left;
    uint32 feed_forward_mic_right;
    uint32 feed_back_mic_left;
    uint32 feed_back_mic_right;
    uint32 dac_output_left;
    uint32 dac_output_right;
} hardware_gains_t;

typedef struct
{
    anc_instance_config_t instance[NUMBER_OF_ANC_INSTANCES];
} anc_mode_config_t;

typedef struct
{
    anc_mode_config_t mode;
    hardware_gains_t hardware_gains;
} anc_config_t;


/******************************************************************************
    Function stubs
******************************************************************************/
bool ancDataSetSidetoneGain(uint16 sidetone_gain);
uint16 ancDataGetSidetoneGain(void);
void ancConfigDataUpdateOnStateChange(void);
bool ancConfigDataUpdateOnModeChange(anc_mode_t mode);

#endif
