// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifdef DEBUG_ON
   #define DEBUG_SBCENC
   #define PROFILE_SBCENC
#endif

#include "sbc.h"
#include "global_variables_encdec.asm"
#include "global_variables_encoder.asm"
#include "reset_encoder.asm"
#include "putbits.asm"
#include "crc_calc.asm"
#include "analysis_subband_filter.asm"
#include "calc_bit_allocation.asm"
#include "write_audio_samples.asm"
#include "calc_scale_factors.asm"
#include "write_scale_factors.asm"
#include "quantize_samples.asm"
#ifndef SBC_WBS_ONLY
#include "write_padding_bits.asm"
#include "write_frame_header.asm"
#include "init_encoder.asm"
#include "calc_frame_length.asm"
#include "joint_stereo_encode.asm"
#include "encoder_set_parameters.asm"
#include "frame_encode.asm"
#endif
#include "wbs_sco_encode.asm"
