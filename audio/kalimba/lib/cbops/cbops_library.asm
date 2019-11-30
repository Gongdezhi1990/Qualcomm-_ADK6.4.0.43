// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************


#ifdef DEBUG_ON
   #define ENABLE_PROFILER_MACROS
   #define CBOPS_DEBUG
   #define CBOPS_STREAM_DEBUG
#endif

#include "profiler.h"
#include "cbops.asm"
#include "operators/cbops_copy_op.asm"
#include "operators/cbops_compress_copy_op.asm"
#include "operators/cbops_mono_to_stereo.asm"
#include "operators/cbops_stereo_3d_enhance_op.asm"
#include "operators/cbops_dc_remove.asm"
#include "operators/cbops_limited_copy.asm"
#include "operators/cbops_mix.asm"
#include "operators/cbops_noise_gate.asm"
#include "operators/cbops_one_to_two_chan_copy.asm"
#include "operators/cbops_shift.asm"
#include "operators/cbops_sidetone_mix.asm"
#include "operators/cbops_silence_clip_detect.asm"
#include "operators/cbops_status_check_gain.asm"
#include "operators/cbops_upsample_mix.asm"
#include "operators/cbops_volume.asm"
#include "operators/cbops_volume_basic.asm"
#include "operators/cbops_warp_and_shift.asm"
#include "operators/cbops_rate_adjustment_and_shift.asm"
#include "operators/cbops_scale.asm"
#include "operators/cbops_two_to_one_chan_copy.asm"
#include "operators/cbops_eq.asm"
#include "operators/cbops_dither_and_shift.asm"
#include "operators/cbops_user_filter.asm"
#include "operators/cbops_cross_mix.asm"
#include "operators/cbops_univ_mix_op.asm"
#include "operators/cbops_s_to_m_op.asm"

