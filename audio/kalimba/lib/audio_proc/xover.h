// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************
#ifndef AUDIO_XOVER_HEADER_INCLUDED
#define AUDIO_XOVER_HEADER_INCLUDED

#ifdef KYMERA
#include "portability_macros.h"
#include "cbuffer_asm.h"
#else
#include "core_library.h"
#endif

                        
   // xover filter type
   .CONST $audio_proc.xover_2band.filter.BYPASS                 0;
   .CONST $audio_proc.xover_2band.filter.BUTTER                 1;
   .CONST $audio_proc.xover_2band.filter.LR                     2;
   .CONST $audio_proc.xover_2band.filter.APC                    3;
   // xover filter precision type (peq filter type)
   .CONST $audio_proc.xover_2band.filter_prec.SH                0;
   .CONST $audio_proc.xover_2band.filter_prec.HQ                1;
   .CONST $audio_proc.xover_2band.filter_prec.DH                2;
   
   .CONST $audio_proc.xover_2band.BW_MAX_CASES                  12;
   .CONST $audio_proc.xover_2band.LR_MAX_CASES                  6;
   .CONST $audio_proc.xover_2band.PEQ_STAGE1_COEFF_OFFSET       3*ADDR_PER_WORD;
   
   
   // xover_2band parameter structure
   .CONST $audio_proc.xover_2band.parameter.XOVER_CONFIG        0;
   .CONST $audio_proc.xover_2band.parameter.NUM_BANDS           ADDR_PER_WORD + $audio_proc.xover_2band.parameter.XOVER_CONFIG;
   .CONST $audio_proc.xover_2band.parameter.XOVER_CORE_TYPE     ADDR_PER_WORD + $audio_proc.xover_2band.parameter.NUM_BANDS;
   .CONST $audio_proc.xover_2band.parameter.LP_TYPE             ADDR_PER_WORD + $audio_proc.xover_2band.parameter.XOVER_CORE_TYPE;
   .CONST $audio_proc.xover_2band.parameter.LP_FC               ADDR_PER_WORD + $audio_proc.xover_2band.parameter.LP_TYPE;
   .CONST $audio_proc.xover_2band.parameter.LP_ORDER            ADDR_PER_WORD + $audio_proc.xover_2band.parameter.LP_FC;
   .CONST $audio_proc.xover_2band.parameter.HP_TYPE             ADDR_PER_WORD + $audio_proc.xover_2band.parameter.LP_ORDER;
   .CONST $audio_proc.xover_2band.parameter.HP_FC               ADDR_PER_WORD + $audio_proc.xover_2band.parameter.HP_TYPE;
   .CONST $audio_proc.xover_2band.parameter.HP_ORDER            ADDR_PER_WORD + $audio_proc.xover_2band.parameter.HP_FC;
   .CONST $audio_proc.xover_2band.parameter.STRUCT_SIZE         ADDR_PER_WORD + $audio_proc.xover_2band.parameter.HP_ORDER;

   // xover_2band structure
   // -------------------------------- START PUBLIC SECTION ---------------------------------
   .CONST $audio_proc.xover_2band.INPUT_FRAME_ADDR_FIELD        0;
   .CONST $audio_proc.xover_2band.OUTPUT_LOW_FRAME_ADDR_FIELD   ADDR_PER_WORD + $audio_proc.xover_2band.INPUT_FRAME_ADDR_FIELD;
   .CONST $audio_proc.xover_2band.OUTPUT_HIGH_FRAME_ADDR_FIELD  ADDR_PER_WORD + $audio_proc.xover_2band.OUTPUT_LOW_FRAME_ADDR_FIELD;
   .CONST $audio_proc.xover_2band.INPUT_ADDR_FIELD              ADDR_PER_WORD + $audio_proc.xover_2band.OUTPUT_HIGH_FRAME_ADDR_FIELD;
   .CONST $audio_proc.xover_2band.OUTPUT_ADDR_FIELD_LOW         ADDR_PER_WORD + $audio_proc.xover_2band.INPUT_ADDR_FIELD;
   .CONST $audio_proc.xover_2band.OUTPUT_ADDR_FIELD_HIGH        ADDR_PER_WORD + $audio_proc.xover_2band.OUTPUT_ADDR_FIELD_LOW;
   .CONST $audio_proc.xover_2band.SAMPLE_RATE_FIELD             ADDR_PER_WORD + $audio_proc.xover_2band.OUTPUT_ADDR_FIELD_HIGH;
   .CONST $audio_proc.xover_2band.PARAM_PTR_FIELD               ADDR_PER_WORD + $audio_proc.xover_2band.SAMPLE_RATE_FIELD;
   // -------------------------------- START INTERNAL SECTION ---------------------------------
   .CONST $audio_proc.xover_2band.SAMPLES_TO_PROCESS            ADDR_PER_WORD + $audio_proc.xover_2band.PARAM_PTR_FIELD;
   .CONST $audio_proc.xover_2band.PEQ_OBJECT_PTR_LOW_FREQ       ADDR_PER_WORD + $audio_proc.xover_2band.SAMPLES_TO_PROCESS;
   .CONST $audio_proc.xover_2band.PEQ_OBJECT_PTR_HIGH_FREQ      ADDR_PER_WORD + $audio_proc.xover_2band.PEQ_OBJECT_PTR_LOW_FREQ;
   .CONST $audio_proc.xover_2band.PEQ_CAP_PARAMS_LOW_FREQ       ADDR_PER_WORD + $audio_proc.xover_2band.PEQ_OBJECT_PTR_HIGH_FREQ;
   .CONST $audio_proc.xover_2band.PEQ_CAP_PARAMS_HIGH_FREQ      ADDR_PER_WORD + $audio_proc.xover_2band.PEQ_CAP_PARAMS_LOW_FREQ;
   .CONST $audio_proc.xover_2band.STRUC_SIZE                    1 + ($audio_proc.xover_2band.PEQ_CAP_PARAMS_HIGH_FREQ >> LOG2_ADDR_PER_WORD);
   

#endif // AUDIO_xover_2band_HEADER_INCLUDED

