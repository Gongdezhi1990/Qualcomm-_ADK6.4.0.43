// *****************************************************************************
// Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// $Revision: #1 $ $Date: 2017/07/05 $
// *****************************************************************************

#ifndef _CVC_SEND_DATA_ASM_H
#define _CVC_SEND_DATA_ASM_H

#include "cvc_modules.h"
#include "cvc_send_gen_asm.h" 
#include "cvc_send_cap_asm.h"

// -----------------------------------------------------------------------------
// CVC SEND DATA ROOT STRUCTURE
// -----------------------------------------------------------------------------
.CONST $cvc_send.data.config_flag      0*ADDR_PER_WORD;
.CONST $cvc_send.data.cur_mode_ptr     1*ADDR_PER_WORD;
.CONST $cvc_send.data.param            2*ADDR_PER_WORD;
.CONST $cvc_send.data.harm_obj         3*ADDR_PER_WORD;
.CONST $cvc_send.data.dms100_obj       4*ADDR_PER_WORD;
.CONST $cvc_send.data.one              5*ADDR_PER_WORD;
.CONST $cvc_send.data.zero             6*ADDR_PER_WORD;
.CONST $cvc_send.data.use              ADDR_PER_WORD + $cvc_send.data.zero;
.CONST $cvc_send.data.mic_mode         ADDR_PER_WORD + $cvc_send.data.use;
.CONST $cvc_send.data.end_fire         ADDR_PER_WORD + $cvc_send.data.mic_mode;
.CONST $cvc_send.data.hfk_config       ADDR_PER_WORD + $cvc_send.data.end_fire;
.CONST $cvc_send.data.dmss_config      ADDR_PER_WORD + $cvc_send.data.hfk_config;
.CONST $cvc_send.data.wind_flag        ADDR_PER_WORD + $cvc_send.data.dmss_config;
.CONST $cvc_send.data.echo_flag        ADDR_PER_WORD + $cvc_send.data.wind_flag;
.CONST $cvc_send.data.vad_flag         ADDR_PER_WORD + $cvc_send.data.echo_flag;
.CONST $cvc_send.data.TP_mode          ADDR_PER_WORD + $cvc_send.data.vad_flag;
.CONST $cvc_send.data.aec_inactive     ADDR_PER_WORD + $cvc_send.data.TP_mode;
.CONST $cvc_send.data.fftwin_power     ADDR_PER_WORD + $cvc_send.data.aec_inactive;
.CONST $cvc_send.data.power_adjust     ADDR_PER_WORD + $cvc_send.data.fftwin_power;
.CONST $cvc_send.data.dmss_obj         ADDR_PER_WORD + $cvc_send.data.power_adjust;
.CONST $cvc_send.data.STRUC_SIZE       1 + ($cvc_send.data.dmss_obj >> LOG2_ADDR_PER_WORD);

.CONST $cvc_send.stream.adc_left       MK1 * 2;
.CONST $cvc_send.stream.adc_right      MK1 * 3;
.CONST $cvc_send.stream.adc_mic3       MK1 * 4;
.CONST $cvc_send.stream.adc_mic4       MK1 * 5;

#endif // _CVC_SEND_DATA_ASM_H
