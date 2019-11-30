// *****************************************************************************
// Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

.MODULE $M.CVC_SEND.PASS_THROUGH;
   .CODESEGMENT PM;

// r7 - mode object
// r8 - mode ptr
$M.CVC_SEND.Set_PassThroughGains_1MIC:
   r4 = M[r8];
   NULL = r4 - $M.CVC_SEND.SYSMODE.STANDBY;
   if GT jump passthroughgains_1mic;
   // If we are in either stand by or static mode then zero the output
   // Standby - Zero Signal
   r0 = NULL;
   r1 = 1;
   jump setgains_1mic;
passthroughgains_1mic:
   // PassThrough Gains set from Parameters
   r2 = M[r7 + $M.SET_MODE_GAIN.PARAM_POINTER];
   r0 = M[r2 + 0*ADDR_PER_WORD];
   r1 = M[r2 + 1*ADDR_PER_WORD];
setgains_1mic:
   M[r7 + $M.SET_MODE_GAIN.MANT_LEFT]    = r0;
   M[r7 + $M.SET_MODE_GAIN.EXP]  = r1;
   rts;


// r7 - mode object
// r8 - mode ptr
$M.CVC_SEND.Set_PassThroughGains_2MIC:
  
   r2 = M[r7 + $M.SET_MODE_GAIN.PARAM_POINTER];
   r5 = M[r2];
   r0 = r5;
   
   r4 = M[r8];
   NULL = r4 - $M.CVC_SEND.SYSMODE.PASS_THRU_LEFT;
   if Z r5 = NULL;
   if Z jump passthroughgains;
   NULL = r4 - $M.CVC_SEND.SYSMODE.PASS_THRU_RIGHT;
   if Z r0 = NULL;
   if Z jump passthroughgains;

   // Standby - Zero Signal
   r5 = NULL;
   r0 = NULL;
   r1 = 1;
   jump setgains;
passthroughgains:
   // PassThrough Gains set from Parameters
   r1 = M[r2 + 1*ADDR_PER_WORD];
setgains:
   M[r7 + $M.SET_MODE_GAIN.MANT_LEFT]    = r0;
   M[r7 + $M.SET_MODE_GAIN.MANT_RIGHT]   = r5;
   M[r7 + $M.SET_MODE_GAIN.EXP]  = r1;
   rts;

   
// -----------------------------------------------------------------------------
//inputs 
//   r7 = &snd_streams
//   r8 = &adc_gain_dm1
//   r9 = $root 
// -----------------------------------------------------------------------------
$M.CVC_SEND.stream_map_select:
   // Get Mode
   r0 = M[r9 + $cvc_send.data.cur_mode_ptr];
   r0 = M[r0];

   r1 = $cvc_send.stream.adc_right;
   Null = r0 - $M.CVC_SEND.SYSMODE.PASS_THRU_RIGHT;
   if Z jump set_input_ptr;
  
   r1 = $cvc_send.stream.adc_mic3;
   Null = r0 - $M.CVC_SEND.SYSMODE.PASS_THRU_MIC3;
   if Z jump set_input_ptr;

   r1 = $cvc_send.stream.adc_mic4;
   Null = r0 - $M.CVC_SEND.SYSMODE.PASS_THRU_MIC4;
   if Z jump set_input_ptr;

   // Default connect to mic1
   r1 = $cvc_send.stream.adc_left;

set_input_ptr:

   r2 = M[r7 + r1];
   M[r8 + $M.audio_proc.stream_gain.OFFSET_INPUT_PTR] = r2;
   rts;

.ENDMODULE;
