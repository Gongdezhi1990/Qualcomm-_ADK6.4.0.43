/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_tuning_data.h

DESCRIPTION

*/

#include <operators.h>

#ifndef ANC_TUNING_DATA_H_
#define ANC_TUNING_DATA_H_

#define CONFIG_DSP_BASE                         (50)
#define CONFIG_DSP(x)                           (CONFIG_DSP_BASE + x)

#define ANC_HARDWARE_TUNING_KEY                 (CONFIG_DSP(1))
#define ANC_PATH_TUNING_KEY                     (CONFIG_DSP(2))

#define UCID_PS_SHIFT                           (1)
#define CAPID_PS_SHIFT                          (7)
#define CAPABILITY_ID_ANC_TUNING                (0x4082)
#define AUDIO_PS_ANC_TUNING(ucid)               ((CAPABILITY_ID_ANC_TUNING << CAPID_PS_SHIFT) | (ucid << UCID_PS_SHIFT))

#define ANC_MODE_CONFIG_KEY(ucid)               (AUDIO_PS_ANC_TUNING(ucid))

/* Contents of this enum describe the tuning parameter offsets and are provided by the tuning tool without modification */
typedef enum
{
    OFFSET_ANC_USECASE_L,
    OFFSET_FF_A_MIC_SENSITIVITY_L,
    OFFSET_FF_B_MIC_SENSITIVITY_L,
    OFFSET_FF_A_FE_GAIN_L,
    OFFSET_FF_B_FE_GAIN_L,
    OFFSET_SPKR_RECEIVE_SENSITIVITY_L,
    OFFSET_SPKR_RECEIVER_IMPEDANCE_L,
    OFFSET_SPKR_RECEIVER_PA_GAIN_L,
    OFFSET_FF_A_ENABLE_L,
    OFFSET_FF_B_ENABLE_L,
    OFFSET_FB_ENABLE_L,
    OFFSET_FFA_IN_ENABLE_L,
    OFFSET_FFB_IN_ENABLE_L,
    OFFSET_FF_A_INPUT_DEVICE_L,
    OFFSET_FF_B_INPUT_DEVICE_L,
    OFFSET_FB_MON_L,
    OFFSET_FF_OUT_ENABLE_L,
    OFFSET_SMLPF_ENABLE_L,
    OFFSET_FF_FLEX_ENABLE_L,
    OFFSET_FF_A_GAIN_ENABLE_L,
    OFFSET_FF_B_GAIN_ENABLE_L,
    OFFSET_FB_GAIN_ENABLE_L,
    OFFSET_FF_A_DCFLT_ENABLE_L,
    OFFSET_FF_B_DCFLT_ENABLE_L,
    OFFSET_DMIC_X2_FF_A_ENABLE_L,
    OFFSET_DMIC_X2_FF_B_ENABLE_L,
    OFFSET_ANC_FF_A_SHIFT_L,
    OFFSET_ANC_FF_B_SHIFT_L,
    OFFSET_ANC_FB_SHIFT_L,
    OFFSET_ANC_FF_A_DEN_COEFF0_L,
    OFFSET_ANC_FF_A_DEN_COEFF1_L,
    OFFSET_ANC_FF_A_DEN_COEFF2_L,
    OFFSET_ANC_FF_A_DEN_COEFF3_L,
    OFFSET_ANC_FF_A_DEN_COEFF4_L,
    OFFSET_ANC_FF_A_DEN_COEFF5_L,
    OFFSET_ANC_FF_A_DEN_COEFF6_L,
    OFFSET_ANC_FF_A_NUM_COEFF0_L,
    OFFSET_ANC_FF_A_NUM_COEFF1_L,
    OFFSET_ANC_FF_A_NUM_COEFF2_L,
    OFFSET_ANC_FF_A_NUM_COEFF3_L,
    OFFSET_ANC_FF_A_NUM_COEFF4_L,
    OFFSET_ANC_FF_A_NUM_COEFF5_L,
    OFFSET_ANC_FF_A_NUM_COEFF6_L,
    OFFSET_ANC_FF_A_NUM_COEFF7_L,
    OFFSET_ANC_FF_A_GAIN_SCALE_L,
    OFFSET_ANC_FF_A_GAIN_SCALE_DEFAULT_L,
    OFFSET_ANC_FF_A_GAIN_L,
    OFFSET_ANC_FF_B_DEN_COEFF0_L,
    OFFSET_ANC_FF_B_DEN_COEFF1_L,
    OFFSET_ANC_FF_B_DEN_COEFF2_L,
    OFFSET_ANC_FF_B_DEN_COEFF3_L,
    OFFSET_ANC_FF_B_DEN_COEFF4_L,
    OFFSET_ANC_FF_B_DEN_COEFF5_L,
    OFFSET_ANC_FF_B_DEN_COEFF6_L,
    OFFSET_ANC_FF_B_NUM_COEFF0_L,
    OFFSET_ANC_FF_B_NUM_COEFF1_L,
    OFFSET_ANC_FF_B_NUM_COEFF2_L,
    OFFSET_ANC_FF_B_NUM_COEFF3_L,
    OFFSET_ANC_FF_B_NUM_COEFF4_L,
    OFFSET_ANC_FF_B_NUM_COEFF5_L,
    OFFSET_ANC_FF_B_NUM_COEFF6_L,
    OFFSET_ANC_FF_B_NUM_COEFF7_L,
    OFFSET_ANC_FF_B_GAIN_SCALE_L,
    OFFSET_ANC_FF_B_GAIN_SCALE_DEFAULT_L,
    OFFSET_ANC_FF_B_GAIN_L,
    OFFSET_ANC_FB_DEN_COEFF0_L,
    OFFSET_ANC_FB_DEN_COEFF1_L,
    OFFSET_ANC_FB_DEN_COEFF2_L,
    OFFSET_ANC_FB_DEN_COEFF3_L,
    OFFSET_ANC_FB_DEN_COEFF4_L,
    OFFSET_ANC_FB_DEN_COEFF5_L,
    OFFSET_ANC_FB_DEN_COEFF6_L,
    OFFSET_ANC_FB_NUM_COEFF0_L,
    OFFSET_ANC_FB_NUM_COEFF1_L,
    OFFSET_ANC_FB_NUM_COEFF2_L,
    OFFSET_ANC_FB_NUM_COEFF3_L,
    OFFSET_ANC_FB_NUM_COEFF4_L,
    OFFSET_ANC_FB_NUM_COEFF5_L,
    OFFSET_ANC_FB_NUM_COEFF6_L,
    OFFSET_ANC_FB_NUM_COEFF7_L,
    OFFSET_ANC_FB_GAIN_SCALE_L,
    OFFSET_ANC_FB_GAIN_SCALE_DEFAULT_L,
    OFFSET_ANC_FB_GAIN_L,
    OFFSET_ANC_FF_A_LPF_SHIFT0_L,
    OFFSET_ANC_FF_A_LPF_SHIFT1_L,
    OFFSET_ANC_FF_B_LPF_SHIFT0_L,
    OFFSET_ANC_FF_B_LPF_SHIFT1_L,
    OFFSET_ANC_FB_LPF_SHIFT0_L,
    OFFSET_ANC_FB_LPF_SHIFT1_L,
    OFFSET_FF_A_DCFLT_SHIFT_L,
    OFFSET_FF_B_DCFLT_SHIFT_L,
    OFFSET_SM_LPF_SHIFT_L,
    OFFSET_ANC_USECASE_R,
    OFFSET_FF_A_MIC_SENSITIVITY_R,
    OFFSET_FF_B_MIC_SENSITIVITY_R,
    OFFSET_FF_A_FE_GAIN_R,
    OFFSET_FF_B_FE_GAIN_R,
    OFFSET_SPKR_RECEIVE_SENSITIVITY_R,
    OFFSET_SPKR_RECEIVER_IMPEDANCE_R,
    OFFSET_SPKR_RECEIVER_PA_GAIN_R,
    OFFSET_FF_A_ENABLE_R,
    OFFSET_FF_B_ENABLE_R,
    OFFSET_FB_ENABLE_R,
    OFFSET_FFA_IN_ENABLE_R,
    OFFSET_FFB_IN_ENABLE_R,
    OFFSET_FF_A_INPUT_DEVICE_R,
    OFFSET_FF_B_INPUT_DEVICE_R,
    OFFSET_FB_MON_R,
    OFFSET_FF_OUT_ENABLE_R,
    OFFSET_SMLPF_ENABLE_R,
    OFFSET_FF_FLEX_ENABLE_R,
    OFFSET_FF_A_GAIN_ENABLE_R,
    OFFSET_FF_B_GAIN_ENABLE_R,
    OFFSET_FB_GAIN_ENABLE_R,
    OFFSET_FF_A_DCFLT_ENABLE_R,
    OFFSET_FF_B_DCFLT_ENABLE_R,
    OFFSET_DMIC_X2_FF_A_ENABLE_R,
    OFFSET_DMIC_X2_FF_B_ENABLE_R,
    OFFSET_ANC_FF_A_SHIFT_R,
    OFFSET_ANC_FF_B_SHIFT_R,
    OFFSET_ANC_FB_SHIFT_R,
    OFFSET_ANC_FF_A_DEN_COEFF0_R,
    OFFSET_ANC_FF_A_DEN_COEFF1_R,
    OFFSET_ANC_FF_A_DEN_COEFF2_R,
    OFFSET_ANC_FF_A_DEN_COEFF3_R,
    OFFSET_ANC_FF_A_DEN_COEFF4_R,
    OFFSET_ANC_FF_A_DEN_COEFF5_R,
    OFFSET_ANC_FF_A_DEN_COEFF6_R,
    OFFSET_ANC_FF_A_NUM_COEFF0_R,
    OFFSET_ANC_FF_A_NUM_COEFF1_R,
    OFFSET_ANC_FF_A_NUM_COEFF2_R,
    OFFSET_ANC_FF_A_NUM_COEFF3_R,
    OFFSET_ANC_FF_A_NUM_COEFF4_R,
    OFFSET_ANC_FF_A_NUM_COEFF5_R,
    OFFSET_ANC_FF_A_NUM_COEFF6_R,
    OFFSET_ANC_FF_A_NUM_COEFF7_R,
    OFFSET_ANC_FF_A_GAIN_SCALE_R,
    OFFSET_ANC_FF_A_GAIN_SCALE_DEFAULT_R,
    OFFSET_ANC_FF_A_GAIN_R,
    OFFSET_ANC_FF_B_DEN_COEFF0_R,
    OFFSET_ANC_FF_B_DEN_COEFF1_R,
    OFFSET_ANC_FF_B_DEN_COEFF2_R,
    OFFSET_ANC_FF_B_DEN_COEFF3_R,
    OFFSET_ANC_FF_B_DEN_COEFF4_R,
    OFFSET_ANC_FF_B_DEN_COEFF5_R,
    OFFSET_ANC_FF_B_DEN_COEFF6_R,
    OFFSET_ANC_FF_B_NUM_COEFF0_R,
    OFFSET_ANC_FF_B_NUM_COEFF1_R,
    OFFSET_ANC_FF_B_NUM_COEFF2_R,
    OFFSET_ANC_FF_B_NUM_COEFF3_R,
    OFFSET_ANC_FF_B_NUM_COEFF4_R,
    OFFSET_ANC_FF_B_NUM_COEFF5_R,
    OFFSET_ANC_FF_B_NUM_COEFF6_R,
    OFFSET_ANC_FF_B_NUM_COEFF7_R,
    OFFSET_ANC_FF_B_GAIN_SCALE_R,
    OFFSET_ANC_FF_B_GAIN_SCALE_DEFAULT_R,
    OFFSET_ANC_FF_B_GAIN_R,
    OFFSET_ANC_FB_DEN_COEFF0_R,
    OFFSET_ANC_FB_DEN_COEFF1_R,
    OFFSET_ANC_FB_DEN_COEFF2_R,
    OFFSET_ANC_FB_DEN_COEFF3_R,
    OFFSET_ANC_FB_DEN_COEFF4_R,
    OFFSET_ANC_FB_DEN_COEFF5_R,
    OFFSET_ANC_FB_DEN_COEFF6_R,
    OFFSET_ANC_FB_NUM_COEFF0_R,
    OFFSET_ANC_FB_NUM_COEFF1_R,
    OFFSET_ANC_FB_NUM_COEFF2_R,
    OFFSET_ANC_FB_NUM_COEFF3_R,
    OFFSET_ANC_FB_NUM_COEFF4_R,
    OFFSET_ANC_FB_NUM_COEFF5_R,
    OFFSET_ANC_FB_NUM_COEFF6_R,
    OFFSET_ANC_FB_NUM_COEFF7_R,
    OFFSET_ANC_FB_GAIN_SCALE_R,
    OFFSET_ANC_FB_GAIN_SCALE_DEFAULT_R,
    OFFSET_ANC_FB_GAIN_R,
    OFFSET_ANC_FF_A_LPF_SHIFT0_R,
    OFFSET_ANC_FF_A_LPF_SHIFT1_R,
    OFFSET_ANC_FF_B_LPF_SHIFT0_R,
    OFFSET_ANC_FF_B_LPF_SHIFT1_R,
    OFFSET_ANC_FB_LPF_SHIFT0_R,
    OFFSET_ANC_FB_LPF_SHIFT1_R,
    OFFSET_FF_A_DCFLT_SHIFT_R,
    OFFSET_FF_B_DCFLT_SHIFT_R,
    OFFSET_SM_LPF_SHIFT_R,

    MAX_TUNING_ITEMS
} tuning_elements_in_audio_ps_t;

#define ANC_TUNING_CONFIG_HEADER_SIZE   (3)
#define ANC_TUNING_CONFIG_DATA_SIZE     (MAX_TUNING_ITEMS * (sizeof(uint32) / sizeof(uint16)))
#define ANC_TUNING_CONFIG_TOTAL_SIZE    (ANC_TUNING_CONFIG_HEADER_SIZE + ANC_TUNING_CONFIG_DATA_SIZE)

#define getPsPayloadOffset(x)           (x * (sizeof(uint32) / sizeof(uint16)))

#define INSTANCE_0_OFFSET               getPsPayloadOffset(OFFSET_ANC_USECASE_L)
#define INSTANCE_1_OFFSET               getPsPayloadOffset(OFFSET_ANC_USECASE_R)

#define FFA_COEFFICIENTS_OFFSET         getPsPayloadOffset(OFFSET_ANC_FF_A_DEN_COEFF0_L)
#define FFA_LPF_SHIFT_1_OFFSET          getPsPayloadOffset(OFFSET_ANC_FF_A_LPF_SHIFT0_L)
#define FFA_LPF_SHIFT_2_OFFSET          getPsPayloadOffset(OFFSET_ANC_FF_A_LPF_SHIFT1_L)
#define FFA_DC_FILTER_SHIFT_OFFSET      getPsPayloadOffset(OFFSET_FF_A_DCFLT_SHIFT_L)
#define FFA_DC_FILTER_ENABLE_OFFSET     getPsPayloadOffset(OFFSET_FF_A_DCFLT_ENABLE_L)
#define FFA_GAIN_OFFSET                 getPsPayloadOffset(OFFSET_ANC_FF_A_GAIN_L)
#define FFA_GAIN_SHIFT_OFFSET           getPsPayloadOffset(OFFSET_ANC_FF_A_SHIFT_L)

#define FFB_COEFFICIENTS_OFFSET         getPsPayloadOffset(OFFSET_ANC_FF_B_DEN_COEFF0_L)
#define FFB_LPF_SHIFT_1_OFFSET          getPsPayloadOffset(OFFSET_ANC_FF_B_LPF_SHIFT0_L)
#define FFB_LPF_SHIFT_2_OFFSET          getPsPayloadOffset(OFFSET_ANC_FF_B_LPF_SHIFT1_L)
#define FFB_DC_FILTER_SHIFT_OFFSET      getPsPayloadOffset(OFFSET_FF_B_DCFLT_SHIFT_L)
#define FFB_DC_FILTER_ENABLE_OFFSET     getPsPayloadOffset(OFFSET_FF_B_DCFLT_ENABLE_L)
#define FFB_GAIN_OFFSET                 getPsPayloadOffset(OFFSET_ANC_FF_B_GAIN_L)
#define FFB_GAIN_SHIFT_OFFSET           getPsPayloadOffset(OFFSET_ANC_FF_B_SHIFT_L)

#define FB_COEFFICIENTS_OFFSET          getPsPayloadOffset(OFFSET_ANC_FB_DEN_COEFF0_L)
#define FB_LPF_SHIFT_1_OFFSET           getPsPayloadOffset(OFFSET_ANC_FB_LPF_SHIFT0_L)
#define FB_LPF_SHIFT_2_OFFSET           getPsPayloadOffset(OFFSET_ANC_FB_LPF_SHIFT1_L)
#define FB_GAIN_OFFSET                  getPsPayloadOffset(OFFSET_ANC_FB_GAIN_L)
#define FB_GAIN_SHIFT_OFFSET            getPsPayloadOffset(OFFSET_ANC_FB_SHIFT_L)

#define SMALL_LPF_SHIFT_OFFSET          getPsPayloadOffset(OFFSET_SM_LPF_SHIFT_L)
#define SMALL_LPF_ENABLE_OFFSET         getPsPayloadOffset(OFFSET_SMLPF_ENABLE_L)

#define ENABLE_FFA_OFFSET               getPsPayloadOffset(OFFSET_FF_A_ENABLE_L)
#define ENABLE_FFB_OFFSET               getPsPayloadOffset(OFFSET_FF_B_ENABLE_L)
#define ENABLE_FB_OFFSET                getPsPayloadOffset(OFFSET_FB_ENABLE_L)
#define ENABLE_OUT_OFFSET               getPsPayloadOffset(OFFSET_FF_OUT_ENABLE_L)

#define GAIN_FEED_FORWARD_MIC_A_OFFSET  getPsPayloadOffset(OFFSET_FF_A_FE_GAIN_L)
#define GAIN_FEED_FORWARD_MIC_B_OFFSET  getPsPayloadOffset(OFFSET_FF_A_FE_GAIN_R)
#define GAIN_FEED_BACK_MIC_A_OFFSET     getPsPayloadOffset(OFFSET_FF_B_FE_GAIN_L)
#define GAIN_FEED_BACK_MIC_B_OFFSET     getPsPayloadOffset(OFFSET_FF_B_FE_GAIN_R)
#define GAIN_DAC_OUTPUT_A_OFFSET        getPsPayloadOffset(OFFSET_SPKR_RECEIVER_PA_GAIN_L)
#define GAIN_DAC_OUTPUT_B_OFFSET        getPsPayloadOffset(OFFSET_SPKR_RECEIVER_PA_GAIN_R)

#define FFA_DMIC_X2_ENABLE_OFFSET       getPsPayloadOffset(OFFSET_DMIC_X2_FF_A_ENABLE_L)
#define FFB_DMIC_X2_ENABLE_OFFSET       getPsPayloadOffset(OFFSET_DMIC_X2_FF_B_ENABLE_L)

#define ENABLE_BIT_FFA                  (0)
#define ENABLE_BIT_FFB                  (1)
#define ENABLE_BIT_FB                   (2)
#define ENABLE_BIT_OUT                  (3)

typedef enum
{
    production_anc_path_gain_index_instance_0_feed_forward_a,
    production_anc_path_gain_index_instance_0_feed_forward_b,
    production_anc_path_gain_index_instance_0_feed_back,
    production_anc_path_gain_index_instance_1_feed_forward_a,
    production_anc_path_gain_index_instance_1_feed_forward_b,
    production_anc_path_gain_index_instance_1_feed_back,
    production_anc_path_gain_index_max
} production_anc_path_gains_in_ps_keys_t;

typedef enum
{
    production_hardware_gain_index_feed_forward_mic_a_low_16,
    production_hardware_gain_index_feed_forward_mic_a_high_16,
    production_hardware_gain_index_feed_forward_mic_b_low_16,
    production_hardware_gain_index_feed_forward_mic_b_high_16,
    production_hardware_gain_index_feed_back_mic_a_low_16,
    production_hardware_gain_index_feed_back_mic_a_high_16,
    production_hardware_gain_index_feed_back_mic_b_low_16,
    production_hardware_gain_index_feed_back_mic_b_high_16,
    production_hardware_gain_index_dac_a_low_16,
    production_hardware_gain_index_dac_a_high_16,
    production_hardware_gain_index_dac_b_low_16,
    production_hardware_gain_index_dac_b_high_16,
    production_hardware_gain_index_max
} production_hardware_gains_in_ps_keys_t;

#endif
