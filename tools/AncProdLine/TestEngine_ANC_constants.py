################################################################################
#
#  TestEngine_ANC_constants.py
#
#  Copyright (c) 2018 Qualcomm Technologies International, Ltd.
#  All Rights Reserved.
#  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
#
#  Defines for ANC production test API.
#
################################################################################

from collections import namedtuple

# microphone routing configuration - should match the on-chip config done with the ADK configuration tool
# mic instance can be 0,1,2
ANALOG_MIC = 3
DIGITAL_MIC = 6

FFA0_MIC_INST = 0   # mic instance for FFA on ANC0
FFB0_MIC_INST = 0   # mic instance for FFB on ANC0
FFA1_MIC_INST = None   # mic instance for FFA on ANC1
FFB1_MIC_INST = None   # mic instance for FFB on ANC1

# mic channel 0=channel A, 1=channel B
FFA0_MIC_CHAN = 0   # mic channel for FFA on ANC0
FFB0_MIC_CHAN = 1   # mic channel for FFB on ANC0
FFA1_MIC_CHAN = None   # mic channel for FFA on ANC1
FFB1_MIC_CHAN = None   # mic channel for FFB on ANC1

# mic type 3=analog, 6=digital
FFA0_MIC_TYPE = ANALOG_MIC   # mic type for FFA on ANC0
FFB0_MIC_TYPE = ANALOG_MIC   # mic type for FFB on ANC0
FFA1_MIC_TYPE = None   # mic type for FFA on ANC1
FFB1_MIC_TYPE = None   # mic type for FFB on ANC1

# return values from TestEngine functions
TE_FAIL = 0
TE_SUCCESS = 1
TE_UNSUPPORTED = 2

TRANSPORT_USBDBG = 256
TRANSPORT_TRB = 128

# available ANC stream keys
STREAM_ANC_INSTANCE = 0x1100
STREAM_ANC_INPUT = 0x1101
STREAM_ANC_FFA_DC_FILTER_ENABLE = 0x1102
STREAM_ANC_FFA_DC_FILTER_SHIFT = 0x1103
STREAM_ANC_FFB_DC_FILTER_ENABLE = 0x1104
STREAM_ANC_FFB_DC_FILTER_SHIFT = 0x1105
STREAM_ANC_SM_LPF_FILTER_ENABLE = 0x1106
STREAM_ANC_SM_LPF_FILTER_SHIFT = 0x1107
STREAM_ANC_FFA_GAIN = 0x1108
STREAM_ANC_FFA_GAIN_SHIFT = 0x1109
STREAM_ANC_FFB_GAIN = 0x110a
STREAM_ANC_FFB_GAIN_SHIFT = 0x110b
STREAM_ANC_FB_GAIN = 0x110c
STREAM_ANC_FB_GAIN_SHIFT = 0x110d
STREAM_ANC_FFA_ADAPT_ENABLE = 0x110e
STREAM_ANC_FFB_ADAPT_ENABLE = 0x110f
STREAM_ANC_FB_ADAPT_ENABLE = 0x1110
STREAM_ANC_CONTROL = 0x1111

# address of ANC PSkey
ANC_PSKEY_ACTIVE = 0x204100
ANC_PSKEY_LEAKTHROUGH = 0x204102
ANC_DATA_NUM_BYTES = 371

# max and min values for coarse (shift) and fine gains
MAX_GAIN_DB = 42    # max gain in dB for combination of coarse and fine gains
MAX_GAIN = 255      # fine gain max value
MAX_GAIN_SHIFT = 6  # coarse gain max value
MIN_GAIN = 0        # minimum fine gain value
MIN_GAIN_SHIFT = -4 # minimum coarse gain value

# in psdata, first 3 words are header information: number of blocks, start length, and number of parameters
OFFSET_PSDATA_HEADER = 3

AncData = namedtuple ('AncData', ('name', 'device', 'instance', 'channel', 'stream_gain', 'stream_gain_shift'))

FFA0 = AncData('FFA0', FFA0_MIC_TYPE, FFA0_MIC_INST, FFA0_MIC_CHAN, STREAM_ANC_FFA_GAIN, STREAM_ANC_FFA_GAIN_SHIFT)
FFB0 = AncData('FFB0', FFB0_MIC_TYPE, FFB0_MIC_INST, FFB0_MIC_CHAN, STREAM_ANC_FFB_GAIN, STREAM_ANC_FFB_GAIN_SHIFT)
FB0 = AncData('FB0', FFA0_MIC_TYPE, FFA0_MIC_INST, FFA0_MIC_CHAN, STREAM_ANC_FB_GAIN, STREAM_ANC_FB_GAIN_SHIFT)        # FB path gain is associated with FFA mic
FFA1 = AncData('FFA1', FFA1_MIC_TYPE, FFA1_MIC_INST, FFA1_MIC_CHAN, STREAM_ANC_FFA_GAIN, STREAM_ANC_FFA_GAIN_SHIFT)
FFB1 = AncData('FFB1', FFB1_MIC_TYPE, FFB1_MIC_INST, FFB1_MIC_CHAN, STREAM_ANC_FFB_GAIN, STREAM_ANC_FFB_GAIN_SHIFT)
FB1 = AncData('FB1', FFA1_MIC_TYPE, FFA1_MIC_INST, FFA1_MIC_CHAN, STREAM_ANC_FB_GAIN, STREAM_ANC_FB_GAIN_SHIFT)        # FB path gain is associated with FFA mic

# ANC parameters available for tuning, shared between QACT, ANC tuning library in apps P1, and ANC tuning capability in audio subsystem
ANC_PARAM_LIST = [
            'OFFSET_ANC_USECASE_L',
            'OFFSET_FF_A_MIC_SENSITIVITY_L',
            'OFFSET_FF_B_MIC_SENSITIVITY_L',
            'OFFSET_FF_A_FE_GAIN_L',
            'OFFSET_FF_B_FE_GAIN_L',
            'OFFSET_SPKR_RECEIVE_SENSITIVITY_L',
            'OFFSET_SPKR_RECEIVER_IMPEDANCE_L',
            'OFFSET_SPKR_RECEIVER_PA_GAIN_L',
            'OFFSET_FF_A_ENABLE_L',
            'OFFSET_FF_B_ENABLE_L',
            'OFFSET_FB_ENABLE_L',
            'OFFSET_FFA_IN_ENABLE_L',
            'OFFSET_FFB_IN_ENABLE_L',
            'OFFSET_FF_A_INPUT_DEVICE_L',
            'OFFSET_FF_B_INPUT_DEVICE_L',
            'OFFSET_FB_MON_L',
            'OFFSET_FF_OUT_ENABLE_L',
            'OFFSET_SMLPF_ENABLE_L',
            'OFFSET_FF_FLEX_ENABLE_L',
            'OFFSET_FF_A_GAIN_ENABLE_L',
            'OFFSET_FF_B_GAIN_ENABLE_L',
            'OFFSET_FB_GAIN_ENABLE_L',
            'OFFSET_FF_A_DCFLT_ENABLE_L',
            'OFFSET_FF_B_DCFLT_ENABLE_L',
            'OFFSET_DMIC_X2_FF_A_ENABLE_L',
            'OFFSET_DMIC_X2_FF_B_ENABLE_L',
            'OFFSET_ANC_FF_A_SHIFT_L',
            'OFFSET_ANC_FF_B_SHIFT_L',
            'OFFSET_ANC_FB_SHIFT_L',
            'OFFSET_ANC_FF_A_DEN_COEFF0_L',
            'OFFSET_ANC_FF_A_DEN_COEFF1_L',
            'OFFSET_ANC_FF_A_DEN_COEFF2_L',
            'OFFSET_ANC_FF_A_DEN_COEFF3_L',
            'OFFSET_ANC_FF_A_DEN_COEFF4_L',
            'OFFSET_ANC_FF_A_DEN_COEFF5_L',
            'OFFSET_ANC_FF_A_DEN_COEFF6_L',
            'OFFSET_ANC_FF_A_NUM_COEFF0_L',
            'OFFSET_ANC_FF_A_NUM_COEFF1_L',
            'OFFSET_ANC_FF_A_NUM_COEFF2_L',
            'OFFSET_ANC_FF_A_NUM_COEFF3_L',
            'OFFSET_ANC_FF_A_NUM_COEFF4_L',
            'OFFSET_ANC_FF_A_NUM_COEFF5_L',
            'OFFSET_ANC_FF_A_NUM_COEFF6_L',
            'OFFSET_ANC_FF_A_NUM_COEFF7_L',
            'OFFSET_ANC_FF_A_GAIN_SCALE_L',
            'OFFSET_ANC_FF_A_GAIN_SCALE_DEFAULT_L',
            'OFFSET_ANC_FF_A_GAIN_L',
            'OFFSET_ANC_FF_B_DEN_COEFF0_L',
            'OFFSET_ANC_FF_B_DEN_COEFF1_L',
            'OFFSET_ANC_FF_B_DEN_COEFF2_L',
            'OFFSET_ANC_FF_B_DEN_COEFF3_L',
            'OFFSET_ANC_FF_B_DEN_COEFF4_L',
            'OFFSET_ANC_FF_B_DEN_COEFF5_L',
            'OFFSET_ANC_FF_B_DEN_COEFF6_L',
            'OFFSET_ANC_FF_B_NUM_COEFF0_L',
            'OFFSET_ANC_FF_B_NUM_COEFF1_L',
            'OFFSET_ANC_FF_B_NUM_COEFF2_L',
            'OFFSET_ANC_FF_B_NUM_COEFF3_L',
            'OFFSET_ANC_FF_B_NUM_COEFF4_L',
            'OFFSET_ANC_FF_B_NUM_COEFF5_L',
            'OFFSET_ANC_FF_B_NUM_COEFF6_L',
            'OFFSET_ANC_FF_B_NUM_COEFF7_L',
            'OFFSET_ANC_FF_B_GAIN_SCALE_L',
            'OFFSET_ANC_FF_B_GAIN_SCALE_DEFAULT_L',
            'OFFSET_ANC_FF_B_GAIN_L',
            'OFFSET_ANC_FB_DEN_COEFF0_L',
            'OFFSET_ANC_FB_DEN_COEFF1_L',
            'OFFSET_ANC_FB_DEN_COEFF2_L',
            'OFFSET_ANC_FB_DEN_COEFF3_L',
            'OFFSET_ANC_FB_DEN_COEFF4_L',
            'OFFSET_ANC_FB_DEN_COEFF5_L',
            'OFFSET_ANC_FB_DEN_COEFF6_L',
            'OFFSET_ANC_FB_NUM_COEFF0_L',
            'OFFSET_ANC_FB_NUM_COEFF1_L',
            'OFFSET_ANC_FB_NUM_COEFF2_L',
            'OFFSET_ANC_FB_NUM_COEFF3_L',
            'OFFSET_ANC_FB_NUM_COEFF4_L',
            'OFFSET_ANC_FB_NUM_COEFF5_L',
            'OFFSET_ANC_FB_NUM_COEFF6_L',
            'OFFSET_ANC_FB_NUM_COEFF7_L',
            'OFFSET_ANC_FB_GAIN_SCALE_L',
            'OFFSET_ANC_FB_GAIN_SCALE_DEFAULT_L',
            'OFFSET_ANC_FB_GAIN_L',
            'OFFSET_ANC_FF_A_LPF_SHIFT0_L',
            'OFFSET_ANC_FF_A_LPF_SHIFT1_L',
            'OFFSET_ANC_FF_B_LPF_SHIFT0_L',
            'OFFSET_ANC_FF_B_LPF_SHIFT1_L',
            'OFFSET_ANC_FB_LPF_SHIFT0_L',
            'OFFSET_ANC_FB_LPF_SHIFT1_L',
            'OFFSET_FF_A_DCFLT_SHIFT_L',
            'OFFSET_FF_B_DCFLT_SHIFT_L',
            'OFFSET_SM_LPF_SHIFT_L',
            'OFFSET_ANC_USECASE_R',
            'OFFSET_FF_A_MIC_SENSITIVITY_R',
            'OFFSET_FF_B_MIC_SENSITIVITY_R',
            'OFFSET_FF_A_FE_GAIN_R',
            'OFFSET_FF_B_FE_GAIN_R',
            'OFFSET_SPKR_RECEIVE_SENSITIVITY_R',
            'OFFSET_SPKR_RECEIVER_IMPEDANCE_R',
            'OFFSET_SPKR_RECEIVER_PA_GAIN_R',
            'OFFSET_FF_A_ENABLE_R',
            'OFFSET_FF_B_ENABLE_R',
            'OFFSET_FB_ENABLE_R',
            'OFFSET_FFA_IN_ENABLE_R',
            'OFFSET_FFB_IN_ENABLE_R',
            'OFFSET_FF_A_INPUT_DEVICE_R',
            'OFFSET_FF_B_INPUT_DEVICE_R',
            'OFFSET_FB_MON_R',
            'OFFSET_FF_OUT_ENABLE_R',
            'OFFSET_SMLPF_ENABLE_R',
            'OFFSET_FF_FLEX_ENABLE_R',
            'OFFSET_FF_A_GAIN_ENABLE_R',
            'OFFSET_FF_B_GAIN_ENABLE_R',
            'OFFSET_FB_GAIN_ENABLE_R',
            'OFFSET_FF_A_DCFLT_ENABLE_R',
            'OFFSET_FF_B_DCFLT_ENABLE_R',
            'OFFSET_DMIC_X2_FF_A_ENABLE_R',
            'OFFSET_DMIC_X2_FF_B_ENABLE_R',
            'OFFSET_ANC_FF_A_SHIFT_R',
            'OFFSET_ANC_FF_B_SHIFT_R',
            'OFFSET_ANC_FB_SHIFT_R',
            'OFFSET_ANC_FF_A_DEN_COEFF0_R',
            'OFFSET_ANC_FF_A_DEN_COEFF1_R',
            'OFFSET_ANC_FF_A_DEN_COEFF2_R',
            'OFFSET_ANC_FF_A_DEN_COEFF3_R',
            'OFFSET_ANC_FF_A_DEN_COEFF4_R',
            'OFFSET_ANC_FF_A_DEN_COEFF5_R',
            'OFFSET_ANC_FF_A_DEN_COEFF6_R',
            'OFFSET_ANC_FF_A_NUM_COEFF0_R',
            'OFFSET_ANC_FF_A_NUM_COEFF1_R',
            'OFFSET_ANC_FF_A_NUM_COEFF2_R',
            'OFFSET_ANC_FF_A_NUM_COEFF3_R',
            'OFFSET_ANC_FF_A_NUM_COEFF4_R',
            'OFFSET_ANC_FF_A_NUM_COEFF5_R',
            'OFFSET_ANC_FF_A_NUM_COEFF6_R',
            'OFFSET_ANC_FF_A_NUM_COEFF7_R',
            'OFFSET_ANC_FF_A_GAIN_SCALE_R',
            'OFFSET_ANC_FF_A_GAIN_SCALE_DEFAULT_R',
            'OFFSET_ANC_FF_A_GAIN_R',
            'OFFSET_ANC_FF_B_DEN_COEFF0_R',
            'OFFSET_ANC_FF_B_DEN_COEFF1_R',
            'OFFSET_ANC_FF_B_DEN_COEFF2_R',
            'OFFSET_ANC_FF_B_DEN_COEFF3_R',
            'OFFSET_ANC_FF_B_DEN_COEFF4_R',
            'OFFSET_ANC_FF_B_DEN_COEFF5_R',
            'OFFSET_ANC_FF_B_DEN_COEFF6_R',
            'OFFSET_ANC_FF_B_NUM_COEFF0_R',
            'OFFSET_ANC_FF_B_NUM_COEFF1_R',
            'OFFSET_ANC_FF_B_NUM_COEFF2_R',
            'OFFSET_ANC_FF_B_NUM_COEFF3_R',
            'OFFSET_ANC_FF_B_NUM_COEFF4_R',
            'OFFSET_ANC_FF_B_NUM_COEFF5_R',
            'OFFSET_ANC_FF_B_NUM_COEFF6_R',
            'OFFSET_ANC_FF_B_NUM_COEFF7_R',
            'OFFSET_ANC_FF_B_GAIN_SCALE_R',
            'OFFSET_ANC_FF_B_GAIN_SCALE_DEFAULT_R',
            'OFFSET_ANC_FF_B_GAIN_R',
            'OFFSET_ANC_FB_DEN_COEFF0_R',
            'OFFSET_ANC_FB_DEN_COEFF1_R',
            'OFFSET_ANC_FB_DEN_COEFF2_R',
            'OFFSET_ANC_FB_DEN_COEFF3_R',
            'OFFSET_ANC_FB_DEN_COEFF4_R',
            'OFFSET_ANC_FB_DEN_COEFF5_R',
            'OFFSET_ANC_FB_DEN_COEFF6_R',
            'OFFSET_ANC_FB_NUM_COEFF0_R',
            'OFFSET_ANC_FB_NUM_COEFF1_R',
            'OFFSET_ANC_FB_NUM_COEFF2_R',
            'OFFSET_ANC_FB_NUM_COEFF3_R',
            'OFFSET_ANC_FB_NUM_COEFF4_R',
            'OFFSET_ANC_FB_NUM_COEFF5_R',
            'OFFSET_ANC_FB_NUM_COEFF6_R',
            'OFFSET_ANC_FB_NUM_COEFF7_R',
            'OFFSET_ANC_FB_GAIN_SCALE_R',
            'OFFSET_ANC_FB_GAIN_SCALE_DEFAULT_R',
            'OFFSET_ANC_FB_GAIN_R',
            'OFFSET_ANC_FF_A_LPF_SHIFT0_R',
            'OFFSET_ANC_FF_A_LPF_SHIFT1_R',
            'OFFSET_ANC_FF_B_LPF_SHIFT0_R',
            'OFFSET_ANC_FF_B_LPF_SHIFT1_R',
            'OFFSET_ANC_FB_LPF_SHIFT0_R',
            'OFFSET_ANC_FB_LPF_SHIFT1_R',
            'OFFSET_FF_A_DCFLT_SHIFT_R',
            'OFFSET_FF_B_DCFLT_SHIFT_R',
            'OFFSET_SM_LPF_SHIFT_R',
            ]
