// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifdef DEBUG_ON
   #define DEBUG_SBCDEC
   #define PROFILE_SBCDEC
#endif

#include "sbc.h"
#include "global_variables_encdec.asm"
#include "global_variables_decoder.asm"
#include "silence_decoder.asm"
#include "getbits.asm"
#include "crc_calc.asm"
#include "synthesis_subband_filter.asm"
#include "calc_bit_allocation.asm"
#include "read_audio_samples.asm"
#include "read_scale_factors.asm"
#include "calc_scale_factors.asm"
#include "sample_reconstruction.asm"
#ifndef SBC_WBS_ONLY
#include "init_decoder.asm"
#include "reset_decoder.asm"
#include "calc_frame_length.asm"
#include "joint_stereo_decode.asm"
#include "find_sync.asm"
#include "read_frame_header.asm"
#include "read_padding_bits.asm"
#include "frame_decode.asm"
#endif
#include "wbs_sco_decode.asm"
