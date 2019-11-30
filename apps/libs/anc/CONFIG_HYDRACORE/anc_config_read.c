/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_config_read.c

DESCRIPTION

*/

#include <stdlib.h>
#include <string.h>
#include "anc.h"
#include "anc_debug.h"
#include "anc_config_data.h"
#include "anc_data.h"
#include "anc_config_read.h"
#include "anc_tuning_data.h"
#include <ps.h>

#define DAWTH 32
/* Convert x into 1.(DAWTH - 1) format */
#define FRACTIONAL(x) ( (int)( (x) * ((1l<<(DAWTH-1)) - 1) ))

/*
 * \brief Inline assembly helper function for performing a fractional multiplication
 *
 * \param a The first value for the multiplication in fractional encoding
 * \param b The value to multiply a by in fractional encoding
 * \return Multiplication result in fractional encoding.
 */
#if defined(__KALIMBA__) && !defined(__GNUC__)
asm int frac_mult(int a, int b)
{
    @{} = @{a} * @{b} (frac);
}
#else
#define frac_mult(a, b) ((int)((((long long)a)*(b)) >> (DAWTH-1))) 
#endif

static uint16 getTuningItem16Bit(uint16 * data)
{
    return (data[1]);
}

static uint32 getTuningItem32Bit(uint16 * data)
{
    return ((data[0] << 16) | data[1]);
}

static uint16 convertCoefficientFromStoredFormat(uint32 coefficient)
{
    return frac_mult(coefficient,FRACTIONAL(1.0/16.0));
}

static unsigned getCoefficientAtIndex(unsigned index, uint16 * data)
{
    return getTuningItem32Bit(&data[index * 2]);
}

static void populateCoefficients(iir_config_t * iir_config, uint16 * data)
{
    unsigned index;
    for(index = 0; index < NUMBER_OF_IIR_COEFFICIENTS; index++)
    {
        iir_config->coefficients[index] = convertCoefficientFromStoredFormat(getCoefficientAtIndex(index, data));
    }
}

static void populateInstance(anc_instance_config_t * instance, uint16 * audio_ps_data)
{
    populateCoefficients(&instance->feed_forward_a.iir_config, &audio_ps_data[FFA_COEFFICIENTS_OFFSET]);

    instance->feed_forward_a.lpf_config.lpf_shift1 = getTuningItem16Bit(&audio_ps_data[FFA_LPF_SHIFT_1_OFFSET]);
    instance->feed_forward_a.lpf_config.lpf_shift2 = getTuningItem16Bit(&audio_ps_data[FFA_LPF_SHIFT_2_OFFSET]);

    instance->feed_forward_a.dc_filter_config.filter_shift = getTuningItem16Bit(&audio_ps_data[FFA_DC_FILTER_SHIFT_OFFSET]);
    instance->feed_forward_a.dc_filter_config.filter_enable = getTuningItem16Bit(&audio_ps_data[FFA_DC_FILTER_ENABLE_OFFSET]);

    instance->feed_forward_a.gain_config.gain = getTuningItem16Bit(&audio_ps_data[FFA_GAIN_OFFSET]);
    instance->feed_forward_a.gain_config.gain_shift = getTuningItem16Bit(&audio_ps_data[FFA_GAIN_SHIFT_OFFSET]);

    instance->feed_forward_a.upconvertor_config.dmic_x2_ff = getTuningItem32Bit(&audio_ps_data[FFA_DMIC_X2_ENABLE_OFFSET]);

    populateCoefficients(&instance->feed_forward_b.iir_config, &audio_ps_data[FFB_COEFFICIENTS_OFFSET]);

    instance->feed_forward_b.lpf_config.lpf_shift1 = getTuningItem16Bit(&audio_ps_data[FFB_LPF_SHIFT_1_OFFSET]);
    instance->feed_forward_b.lpf_config.lpf_shift2 = getTuningItem16Bit(&audio_ps_data[FFB_LPF_SHIFT_2_OFFSET]);

    instance->feed_forward_b.dc_filter_config.filter_shift = getTuningItem16Bit(&audio_ps_data[FFB_DC_FILTER_SHIFT_OFFSET]);
    instance->feed_forward_b.dc_filter_config.filter_enable = getTuningItem16Bit(&audio_ps_data[FFB_DC_FILTER_ENABLE_OFFSET]);

    instance->feed_forward_b.gain_config.gain = getTuningItem16Bit(&audio_ps_data[FFB_GAIN_OFFSET]);
    instance->feed_forward_b.gain_config.gain_shift = getTuningItem16Bit(&audio_ps_data[FFB_GAIN_SHIFT_OFFSET]);

    instance->feed_forward_b.upconvertor_config.dmic_x2_ff = getTuningItem32Bit(&audio_ps_data[FFB_DMIC_X2_ENABLE_OFFSET]);

    populateCoefficients(&instance->feed_back.iir_config, &audio_ps_data[FB_COEFFICIENTS_OFFSET]);

    instance->feed_back.lpf_config.lpf_shift1 = getTuningItem16Bit(&audio_ps_data[FB_LPF_SHIFT_1_OFFSET]);
    instance->feed_back.lpf_config.lpf_shift2 = getTuningItem16Bit(&audio_ps_data[FB_LPF_SHIFT_2_OFFSET]);

    instance->feed_back.gain_config.gain = getTuningItem16Bit(&audio_ps_data[FB_GAIN_OFFSET]);
    instance->feed_back.gain_config.gain_shift = getTuningItem16Bit(&audio_ps_data[FB_GAIN_SHIFT_OFFSET]);

    instance->small_lpf.small_lpf_config.filter_shift = getTuningItem16Bit(&audio_ps_data[SMALL_LPF_SHIFT_OFFSET]);
    instance->small_lpf.small_lpf_config.filter_enable = getTuningItem16Bit(&audio_ps_data[SMALL_LPF_ENABLE_OFFSET]);

    instance->enable_mask = ((getTuningItem16Bit(&audio_ps_data[ENABLE_FFA_OFFSET]) << ENABLE_BIT_FFA)
                                | (getTuningItem16Bit(&audio_ps_data[ENABLE_FFB_OFFSET]) << ENABLE_BIT_FFB)
                                | (getTuningItem16Bit(&audio_ps_data[ENABLE_FB_OFFSET]) << ENABLE_BIT_FB)
                                | (getTuningItem16Bit(&audio_ps_data[ENABLE_OUT_OFFSET]) << ENABLE_BIT_OUT));
}

static void populateHardwareGains(hardware_gains_t * hardware_gains, uint16 * audio_ps_data)
{
    hardware_gains->feed_forward_mic_left = getTuningItem32Bit(&audio_ps_data[GAIN_FEED_FORWARD_MIC_A_OFFSET]);
    hardware_gains->feed_forward_mic_right = getTuningItem32Bit(&audio_ps_data[GAIN_FEED_FORWARD_MIC_B_OFFSET]);
    hardware_gains->feed_back_mic_left = getTuningItem32Bit(&audio_ps_data[GAIN_FEED_BACK_MIC_A_OFFSET]);
    hardware_gains->feed_back_mic_right = getTuningItem32Bit(&audio_ps_data[GAIN_FEED_BACK_MIC_B_OFFSET]);
    hardware_gains->dac_output_left = getTuningItem32Bit(&audio_ps_data[GAIN_DAC_OUTPUT_A_OFFSET]);
    hardware_gains->dac_output_right = getTuningItem32Bit(&audio_ps_data[GAIN_DAC_OUTPUT_B_OFFSET]);
}

static bool readTuningKey(uint32 key, uint16 * read_buffer)
{
    uint16 total_key_length = 0;
    return ((PsReadAudioKey(key, read_buffer, ANC_TUNING_CONFIG_DATA_SIZE,
                ANC_TUNING_CONFIG_HEADER_SIZE, &total_key_length) == ANC_TUNING_CONFIG_DATA_SIZE)
            && (total_key_length == ANC_TUNING_CONFIG_TOTAL_SIZE));
}

static bool populateTuningConfigData(anc_config_t * config_data, anc_mode_t set_mode)
{
    uint16 read_buffer[ANC_TUNING_CONFIG_DATA_SIZE];
    bool value = TRUE;

    if(readTuningKey(ANC_MODE_CONFIG_KEY(set_mode), read_buffer))
    {
       populateInstance(&config_data->mode.instance[ANC_INSTANCE_0_INDEX], &read_buffer[INSTANCE_0_OFFSET]);
       populateInstance(&config_data->mode.instance[ANC_INSTANCE_1_INDEX], &read_buffer[INSTANCE_1_OFFSET]);
       populateHardwareGains(&config_data->hardware_gains, read_buffer);
    }
    else
    {
       value = FALSE;
    }
    
    return value;   
}

static bool isPsKeyValid(uint16 key, uint16 number_of_elements)
{
    return (PsRetrieve(key, NULL, 0) == number_of_elements);
}

static void populateDeviceSpecificHardwareGains(hardware_gains_t * hardware_gains)
{
    if(isPsKeyValid(ANC_HARDWARE_TUNING_KEY, production_hardware_gain_index_max))
    {
        uint16 gains[production_hardware_gain_index_max];

        PsRetrieve(ANC_HARDWARE_TUNING_KEY, gains, production_hardware_gain_index_max);

        hardware_gains->feed_forward_mic_left = (gains[production_hardware_gain_index_feed_forward_mic_a_low_16]
                                                    | (gains[production_hardware_gain_index_feed_forward_mic_a_high_16] << 16));
        hardware_gains->feed_forward_mic_right = (gains[production_hardware_gain_index_feed_forward_mic_b_low_16]
                                                    | (gains[production_hardware_gain_index_feed_forward_mic_b_high_16] << 16));
        hardware_gains->feed_back_mic_left = (gains[production_hardware_gain_index_feed_back_mic_a_low_16]
                                                 | (gains[production_hardware_gain_index_feed_back_mic_a_high_16] << 16));
        hardware_gains->feed_back_mic_right = (gains[production_hardware_gain_index_feed_back_mic_b_low_16]
                                                 | (gains[production_hardware_gain_index_feed_back_mic_b_high_16] << 16));
        hardware_gains->dac_output_left = (gains[production_hardware_gain_index_dac_a_low_16]
                                              | (gains[production_hardware_gain_index_dac_a_high_16] << 16));
        hardware_gains->dac_output_right = (gains[production_hardware_gain_index_dac_b_low_16]
                                              | (gains[production_hardware_gain_index_dac_b_high_16] << 16));
    }
}

static void populateAncPathGain(gain_config_t * gain_config, uint16 ps_gain)
{
    gain_config->gain = ps_gain;
    gain_config->gain_shift = (ps_gain >> 8);
}

static void populateDeviceSpecificAncPathGains(anc_config_t * config_data)
{
    if(isPsKeyValid(ANC_PATH_TUNING_KEY, production_anc_path_gain_index_max))
    {
        uint16 path_tuning_data[production_anc_path_gain_index_max];

        PsRetrieve(ANC_PATH_TUNING_KEY, path_tuning_data, production_anc_path_gain_index_max);

        populateAncPathGain(&config_data->mode.instance[ANC_INSTANCE_0_INDEX].feed_forward_a.gain_config,
                path_tuning_data[production_anc_path_gain_index_instance_0_feed_forward_a]);
        populateAncPathGain(&config_data->mode.instance[ANC_INSTANCE_0_INDEX].feed_forward_b.gain_config,
                path_tuning_data[production_anc_path_gain_index_instance_0_feed_forward_b]);
        populateAncPathGain(&config_data->mode.instance[ANC_INSTANCE_0_INDEX].feed_back.gain_config,
                path_tuning_data[production_anc_path_gain_index_instance_0_feed_back]);

        populateAncPathGain(&config_data->mode.instance[ANC_INSTANCE_1_INDEX].feed_forward_a.gain_config,
                path_tuning_data[production_anc_path_gain_index_instance_1_feed_forward_a]);
        populateAncPathGain(&config_data->mode.instance[ANC_INSTANCE_1_INDEX].feed_forward_b.gain_config,
                path_tuning_data[production_anc_path_gain_index_instance_1_feed_forward_b]);
        populateAncPathGain(&config_data->mode.instance[ANC_INSTANCE_1_INDEX].feed_back.gain_config,
                path_tuning_data[production_anc_path_gain_index_instance_1_feed_back]);
    }
}


static void populateDeviceSpecificTuningConfigData(anc_config_t * config_data)
{
    populateDeviceSpecificHardwareGains(&config_data->hardware_gains);
    populateDeviceSpecificAncPathGains(config_data);
}


bool ancConfigReadPopulateAncData(anc_config_t * config_data, anc_mode_t set_mode)
{
    bool value = populateTuningConfigData(config_data, set_mode);
    populateDeviceSpecificTuningConfigData(config_data);

    return value;
}
