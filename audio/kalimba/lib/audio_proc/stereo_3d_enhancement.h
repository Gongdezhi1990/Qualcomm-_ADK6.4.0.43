// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************
#ifndef AUDIO_STEREO_3D_ENHANCEMENT_HEADER_INCLUDED
#define AUDIO_STEREO_3D_ENHANCEMENT_HEADER_INCLUDED

// stereo_3d_enhancement data object definitions
.CONST   $stereo_3d_enhancement.INPUT_CH1_PTR_BUFFER_FIELD      0*ADDR_PER_WORD;
.CONST   $stereo_3d_enhancement.INPUT_CH2_PTR_BUFFER_FIELD      1*ADDR_PER_WORD;
.CONST   $stereo_3d_enhancement.OUTPUT_CH1_PTR_BUFFER_FIELD     2*ADDR_PER_WORD;
.CONST   $stereo_3d_enhancement.OUTPUT_CH2_PTR_BUFFER_FIELD     3*ADDR_PER_WORD;
.CONST   $stereo_3d_enhancement.DELAY_1_STRUC_FIELD             4*ADDR_PER_WORD;
.CONST   $stereo_3d_enhancement.DELAY_2_STRUC_FIELD             5*ADDR_PER_WORD;
.CONST   $stereo_3d_enhancement.COEFF_STRUC_FIELD               6*ADDR_PER_WORD;
.CONST   $stereo_3d_enhancement.REFLECTION_DELAY_SAMPLES_FIELD  7*ADDR_PER_WORD;
.CONST   $stereo_3d_enhancement.MIX_FIELD                       8*ADDR_PER_WORD;
.CONST   $stereo_3d_enhancement.SE_CONFIG_FIELD                 9*ADDR_PER_WORD;
.CONST   $stereo_3d_enhancement.ENABLE_BIT_MASK_FIELD           10*ADDR_PER_WORD;
.CONST   $stereo_3d_enhancement.STRUC_SIZE                      11;

// A good delay (in samples) for the reflections
.CONST   $stereo_3d_enhancement.REFLECTION_DELAY                618;
// 2048 + NUM_SAMPLE_PER_FRAME
.CONST   $stereo_3d_enhancement.DELAY_BUFFER_SIZE               2208; // in words (arch4)

#endif // AUDIO_STEREO_3D_ENHANCEMENT_HEADER_INCLUDED
