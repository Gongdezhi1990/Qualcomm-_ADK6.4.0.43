// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

#include "portability_macros.h"
#include "stack.h"

#include "passthr_mode.h"

#include "cvc_receive_cap_asm.h"

#include "shared_memory_ids_asm_defs.h"
#include "frame_iir_resamplerv2_asm_defs.h"
#include "iir_resamplerv2_common_asm_defs.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif

#define uses_RCV_FREQPROC  (uses_RCV_NS || uses_AEQ)
#define uses_RCV_VAD       (uses_RCV_AGC || uses_AEQ)


// *****************************************************************************
// MODULE:
//    unsigned CVC_RCV_CAP_Create(CVC_RECEIVE_OP_DATA *op_extra_data);
//
// DESCRIPTION:
//
// MODIFICATIONS:
//
// INPUTS:
//    - r0 - Extended Data
//
// OUTPUTS:
//    - r0 - status
//
// TRASHED REGISTERS:
//
// CPU USAGE:
// *****************************************************************************
.MODULE $M.CVC_RCV_CAP.Create;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

$_CVC_RCV_CAP_Create:
   push rLink;
   pushm <r8,r9>;

   r8 = r0; // Extended Data

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($CVC_RCV_CAP_CREATE.PATCH_ID_0, r1)
#endif

   call $cvc_recv.register_component;

   r9 = M[r8 + $CVC_RCV_CAP.ROOT.CAPABILITY_ID];

   //  Allocate Scratch Data
   r0 = r8;
   r1 = $M.CVC_RCV_CAP.data.DynTable_Main;
   r2 = r9;
   call $_DynLoaderScratchReserve;
   NULL = r0;
   if Z CALL_PANIC_OR_JUMP(create_failed);

   //
   //  Allocate Shared Variables
   //

   // CVCLIB table
   r0 = r8  + $CVC_RCV_CAP.ROOT.CVCLIB_TABLE_FIELD;
   call $_cvclib_table_alloc;
   r0 = M[r8  + $CVC_RCV_CAP.ROOT.CVCLIB_TABLE_FIELD];
   if Z CALL_PANIC_OR_JUMP(create_failed);


   // Allocate VAD & DC Blocker Resources
#if uses_RCV_VAD ||  uses_DCBLOCK
   // r1 -> wideband flag
   r1 = 2;
   Null = r9 - $M.CVC_RECV_UWB_CAP_ID;
   if NZ r1 = 1;
   Null = r9 - $M.CVC_RECV_WB_CAP_ID;
   if LT r1 = 0;
   r0 = r8 + $CVC_RCV_CAP.ROOT.VAD_DCB_COEFFS_PTR_FIELD;
   call $_vad400_AllocMem;
   r0 = M[r8 + $CVC_RCV_CAP.ROOT.VAD_DCB_COEFFS_PTR_FIELD];
   if Z  CALL_PANIC_OR_JUMP(create_failed);
#endif

   // Allocate OMS Resources
#if uses_RCV_NS
   //WB flag
   r1 = 0;
   Null = r9 - $M.CVC_RECV_WB_CAP_ID;
   if GE r1 = 1;
   r0 = r8 + $CVC_RCV_CAP.ROOT.OMS_CONFIG_PTR_FIELD;
   call $_oms270_AllocMem;
   r0 = M[r8 + $CVC_RCV_CAP.ROOT.OMS_CONFIG_PTR_FIELD];
   if Z CALL_PANIC_OR_JUMP(create_failed);
#endif

  // Allocate FB resources
#if uses_RCV_FREQPROC
   r0 = r8 + $CVC_RCV_CAP.ROOT.ANAL_FB_CONFIG_PTR_FIELD;
   r1 = 0;
   Null = r9 - $M.CVC_RECV_WB_CAP_ID;
   if GE r1 = 1;
   r2 = 0;
   call $_dyn_load_filter_bank_config;
   r0 = M[r8 + $CVC_RCV_CAP.ROOT.ANAL_FB_CONFIG_PTR_FIELD];
   if Z CALL_PANIC_OR_JUMP(create_failed);

   r0 = r8 + $CVC_RCV_CAP.ROOT.SYNTH_FB_CONFIG_PTR_FIELD;
   r1 = r9 - $M.CVC_RECV_NB_CAP_ID;
   r2 = 0;
   call $_dyn_load_filter_bank_config;
   r0 = M[r8 + $CVC_RCV_CAP.ROOT.SYNTH_FB_CONFIG_PTR_FIELD];
   if Z CALL_PANIC_OR_JUMP(create_failed);

   r0 = r8 + $CVC_RCV_CAP.ROOT.FB_SPLIT_PTR_FIELD;
   call $_dyn_load_filter_bank_split;
   r0 = M[r8 + $CVC_RCV_CAP.ROOT.FB_SPLIT_PTR_FIELD];
   if Z CALL_PANIC_OR_JUMP(create_failed);
#endif

#if defined(INSTALL_OPERATOR_CVC_32K) || defined(INSTALL_OPERATOR_CVC_48K)
   r0 = NULL;
   r2 = $shared_memory_ids.IIRV2_RESAMPLER_Up_1_Down_2;  // SWB: 32k -> 16k
   r3 = $shared_memory_ids.IIRV2_RESAMPLER_Up_1_Down_3;  // FB:  48k -> 16k

   r1 = M[r8 + $CVC_RCV_CAP.ROOT.APPLY_RESAMPLE_FIELD];
   Null = r1 - RCV_VARIANT_SWB;  
   if Z r0 = r2;
   Null = r1 - RCV_VARIANT_FB;
   if Z r0 = r3;  

   NULL = r0;
   if Z jump jp_resample_setup_done;
      //setup down sampling filter
      call $_iir_resamplerv2_allocate_config_by_id;
      M[r8 + $CVC_RCV_CAP.ROOT.DOWNSAMPLE_CONFIG_FIELD] = r0;                                             
      if Z CALL_PANIC_OR_JUMP(create_failed);
 
      r2 = $shared_memory_ids.IIRV2_RESAMPLER_Up_2_Down_1;  // SWB: 16k -> 32k
      r3 = $shared_memory_ids.IIRV2_RESAMPLER_Up_3_Down_1;  // FB:  16k -> 48k
     
      r1 = M[r8 + $CVC_RCV_CAP.ROOT.APPLY_RESAMPLE_FIELD];
      Null = r1 - RCV_VARIANT_SWB;
      if Z r0 = r2;
      Null = r1 - RCV_VARIANT_FB;
      if Z r0 = r3;

      //setup up-sampling filter
      call $_iir_resamplerv2_allocate_config_by_id;
      M[r8 + $CVC_RCV_CAP.ROOT.UPSAMPLE_CONFIG_FIELD] = r0;
      if Z CALL_PANIC_OR_JUMP(create_failed);

   jp_resample_setup_done:
#endif

   // Set the current mode pointer field

   r10 = r8 + $CVC_RCV_CAP.ROOT.CUR_MODE;
   M[r8 + $CVC_RCV_CAP.ROOT.CUR_MODE_PTR_FIELD] = r10;

   //  Allocate Persistent Data
   r0 = r8;
   r1 = $M.CVC_RCV_CAP.data.DynTable_Main;
   r2 = $M.CVC_RCV_CAP.data.DynTable_Linker;
   r3 = r9;
   call $_DynLoaderProcessDynamicAllocations;
   NULL = r0;
   if Z CALL_PANIC_OR_JUMP(create_failed);

   // Setup upsampler
   Null = r9 - $M.CVC_RECV_FE_CAP_ID;
   if NZ jump skip_rcv_fe_resampler_setup;
        r0 = $shared_memory_ids.IIRV2_RESAMPLER_Up_2_Down_1;
        call $_iir_resamplerv2_allocate_config_by_id;
        r1 = r0;
        if Z CALL_PANIC_OR_JUMP(create_failed);
        r0 = M[r8 + $CVC_RCV_CAP.ROOT.UPSAMPLE_PTR_FIELD];
        r0 = r0 + $frame_iir_resamplerv2.iir_resampler_op_struct.COMMON_FIELD;
        call $_iir_resamplerv2_set_config;

skip_rcv_fe_resampler_setup:
  

create_succeeded:
   r0 = 0;                                                         // create succeeded
   popm <r8,r9>;
   pop rLink;
   rts;
create_failed:
   r0 = 1;                                                         // create failed
   popm <r8,r9>;
   pop rLink;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    void CVC_RCV_CAP_Destroy(CVC_RECEIVE_OP_DATA *op_extra_data);
//
// DESCRIPTION:
//
// MODIFICATIONS:
//
// INPUTS:
//    - r0 - Extended Data
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//
// CPU USAGE:
// *****************************************************************************
.MODULE $M.CVC_RCV_CAP.Destroy;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

$_CVC_RCV_CAP_Destroy:
   push rLink;
   push r8;

   r8 = r0;  // extended data

   // unregister component
   call $Security.UnregisterComponent;

   //  deallocate Scratch Data
   r0 = M[r8 + $CVC_RCV_CAP.ROOT.SCRATCH_ALLOC_PTR_FIELD];
   if NZ call $_DynLoaderDeregisterScratch;
   M[r8 + $CVC_RCV_CAP.ROOT.SCRATCH_ALLOC_PTR_FIELD]=NULL;


   r0 = M[r8  + $CVC_RCV_CAP.ROOT.CVCLIB_TABLE_FIELD];
   if NZ call $_cvclib_table_release;
   M[r8  + $CVC_RCV_CAP.ROOT.CVCLIB_TABLE_FIELD] = NULL;

   r0 = M[r8 + $CVC_RCV_CAP.ROOT.UPSAMPLE_PTR_FIELD];
   r0 = M[r0 + ($frame_iir_resamplerv2.iir_resampler_op_struct.COMMON_FIELD + $iir_resamplerv2_common.iir_resampler_common_struct.FILTER_FIELD)];
   if NZ call $_iir_resamplerv2_release_config;
   
#if defined(INSTALL_OPERATOR_CVC_32K) || defined(INSTALL_OPERATOR_CVC_48K)
   r0 = M[r8 + $CVC_RCV_CAP.ROOT.DOWNSAMPLE_CONFIG_FIELD];
   if NZ call $_iir_resamplerv2_release_config;
   M[r8 + $CVC_RCV_CAP.ROOT.DOWNSAMPLE_CONFIG_FIELD] = NULL; 
   
   r0 = M[r8 + $CVC_RCV_CAP.ROOT.UPSAMPLE_CONFIG_FIELD];
   if NZ call $_iir_resamplerv2_release_config;
   M[r8 + $CVC_RCV_CAP.ROOT.UPSAMPLE_CONFIG_FIELD] = NULL; 
#endif

   r0 = M[r8 + $CVC_RCV_CAP.ROOT.VAD_DCB_COEFFS_PTR_FIELD];
   if NZ call $_vad400_ReleaseMem;
   M[r8 + $CVC_RCV_CAP.ROOT.VAD_DCB_COEFFS_PTR_FIELD]=NULL;

   r0 = M[r8 + $CVC_RCV_CAP.ROOT.OMS_CONFIG_PTR_FIELD];
   if NZ call $_dms100_ReleaseMem;
   M[r8 + $CVC_RCV_CAP.ROOT.OMS_CONFIG_PTR_FIELD]=NULL;

   r0 = M[r8 + $CVC_RCV_CAP.ROOT.ANAL_FB_CONFIG_PTR_FIELD];
   if NZ call $_dyn_free_filter_bank_config;
   M[r8 + $CVC_RCV_CAP.ROOT.ANAL_FB_CONFIG_PTR_FIELD]=NULL;

   r0 = M[r8 + $CVC_RCV_CAP.ROOT.SYNTH_FB_CONFIG_PTR_FIELD];
   if NZ call $_dyn_free_filter_bank_config;
   M[r8 + $CVC_RCV_CAP.ROOT.SYNTH_FB_CONFIG_PTR_FIELD]=NULL;

   r0 = M[r8 + $CVC_RCV_CAP.ROOT.FB_SPLIT_PTR_FIELD];
   if NZ call $_dyn_free_filter_bank_split;
   M[r8 + $CVC_RCV_CAP.ROOT.FB_SPLIT_PTR_FIELD]=NULL;

   //  deallocate Persistent Data
   r0 = M[r8 + $CVC_RCV_CAP.ROOT.INST_ALLOC_PTR_FIELD];
   if NZ call $_DynLoaderReleaseDynamicAllocations;
   M[r8 + $CVC_RCV_CAP.ROOT.INST_ALLOC_PTR_FIELD]=NULL;

   pop r8;
   pop rLink;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    void CVC_RCV_CAP_Process(CVC_RECEIVE_OP_DATA *op_extra_data);
//
// DESCRIPTION:
//
// MODIFICATIONS:
//
// INPUTS:
//    - r0 - Extended Data
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//
// CPU USAGE:
// *****************************************************************************
.MODULE $M.CVC_RCV_CAP;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

$_CVC_RCV_CAP_Process:
   push rLink;

   push r0;    // Save Root Object. As local variable

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($CVC_RCV_CAP_PROCESS.PATCH_ID_0, r1)
#endif

   r1 = r0;
   r0 = M[r0 + $CVC_RCV_CAP.ROOT.INST_ALLOC_PTR_FIELD];
   r1 = M[r1 + $CVC_RCV_CAP.ROOT.SCRATCH_ALLOC_PTR_FIELD];
   if NZ call $_DynLoaderScratchCommit;

   r0 = M[SP - 1*ADDR_PER_WORD];  // extended data
   NULL = M[r0 + $CVC_RCV_CAP.ROOT.ALGREINIT];
   if NZ call ReInitialize;    // preserves FP

   // Set Current Mode (r1)
   r0 = M[SP - 1*ADDR_PER_WORD];  // extended data
   r1 = M[r0 + $CVC_RCV_CAP.ROOT.HOST_MODE];
   r3 = M[r0 + $CVC_RCV_CAP.ROOT.OBPM_MODE];
   r2 = M[r0 + $CVC_RCV_CAP.ROOT.OVR_CONTROL];
   Null = r2 AND $M.CVC_RECV.CONTROL.MODE_OVERRIDE;
   if NZ r1 = r3;
   
   // Process Data and save current mode
   call $cvc_recv.process_data;


   // AA - Updated AGC gain value to be persisted, here
   r0 = M[SP - 1*ADDR_PER_WORD];  // extended data
   #if uses_RCV_AGC
   r3 = M[r0 + $CVC_RCV_CAP.ROOT.MODULES_PTR_FIELD];
   r1 = M[r3 + $CVC_RCV_CAP.MODULE.AGC_PTR_FIELD];
   r2 = M[r1 + $M.agc400.OFFSET_G_REAL_FIELD];
   M[r0 + $CVC_RCV_CAP.ROOT.AGC_STATE] = r2;
   #endif

   pop r0;     // remove local variables

   // Release committed scratch
   NULL = M[r0 + $CVC_RCV_CAP.ROOT.SCRATCH_ALLOC_PTR_FIELD];
   if NZ call $_scratch_free;

   pop rLink;
   rts;


ReInitialize:

   push rLink;
   push r9;
   push r0;    // root object

   M[r0 + $CVC_RCV_CAP.ROOT.ALGREINIT]  = Null;
   // AA - Zero frame counter here

   r9 = M[r0 + $CVC_RCV_CAP.ROOT.MODULES_PTR_FIELD];
   r3 = M[r0 + $CVC_RCV_CAP.ROOT.PARAMS_PTR_FIELD];
   r0=  M[r3 + $M.CVC_RECV.PARAMETERS.OFFSET_RCV_CONFIG];

#if uses_AEQ
   r2 = M[r9 + $CVC_RCV_CAP.MODULE.AEQ_PTR_FIELD];
   M[r2 + $M.AdapEq.CONTROL_WORD_FIELD]=r0;
#endif
#if uses_RCV_AGC
   // AGC bypass control
   r2 = M[r9 + $CVC_RCV_CAP.MODULE.AGC_PTR_FIELD];
   M[r2 + $M.agc400.OFFSET_SYS_CON_WORD_FIELD]=r0;
   r1 = $M.CVC_RECV.CONFIG.RCVAGCBYP;
   M[r2 + $M.agc400.OFFSET_BYPASS_BIT_MASK_FIELD]=r1;

   // AGC persistent bypass control
   r1 = $M.CVC_RECV.CONFIG.BYPASS_AGCPERSIST;
   M[r2 + $M.agc400.OFFSET_BYPASS_PERSIST_FIELD]=r1;

   r1 = M[SP - 1*ADDR_PER_WORD];
   r1 = M[r1 + $CVC_RCV_CAP.ROOT.AGC_STATE];
   M[r2 + $M.agc400.OFFSET_PERSISTED_GAIN_FIELD] = r1;
  // AA - set AGC persistent gain value here here
#endif


   // -----------------------------------------------------------------------------
   // OMS rcv module bypass/hold  and High Resolution mode
   // -----------------------------------------------------------------------------
#if uses_RCV_NS
   r2 = M[r9 + $CVC_RCV_CAP.MODULE.HARM_PTR_FIELD];
   r1=M[r3 + $M.CVC_RECV.PARAMETERS.OFFSET_OMS_HI_RES_MODE];
   r1 = 1 - r1;
   M[r2 + $harmonicity.FLAG_BYPASS_FIELD]=r1;
   r2 = M[r9 + $CVC_RCV_CAP.MODULE.OMS_PTR_FIELD];
   r1 = r0 AND $M.CVC_RECV.CONFIG.RCVOMSBYP;
   M[r2 + $M.oms270.BYPASS_FIELD] = r1;

   //oms270rcv harm value pointer import
   r0 = M[r9 + $CVC_RCV_CAP.MODULE.HARM_PTR_FIELD];
   r1 = r0 + $harmonicity.HARM_VALUE_FIELD;
   Null = M[r0 + $harmonicity.FLAG_BYPASS_FIELD];
   if NZ r1 = 0;
   M[r2 + $M.oms270.PTR_HARM_VALUE_FIELD] = r1;
#endif

   r0 = M[SP - 1*ADDR_PER_WORD];  // root object   
   r0 = M[r0 + $CVC_RCV_CAP.ROOT.INIT_TABLE_PTR_FIELD];
   call $_run_frame_proc_function_table;

   pop r0;     // remove local variables
   pop r9;
   jump $pop_rLink_and_rts;



// -r7 = resampler
// -r8 = variant
$cvc.fe.frame_resample_process:
   Null = r8 - $M.CVC_RECV_FE_CAP_ID;
   if NZ rts;
   jump $frame.iir_resamplev2.Process;


// -r8 = variant
$cvc.rcv_peq.process:
   Null = r8 - $M.CVC_RECV_WB_CAP_ID;
   if GE rts;
   jump $audio_proc.peq.process;

// -r8 = variant
$cvc.rcv_peq.process_wb:
   Null = r8 - $M.CVC_RECV_WB_CAP_ID;
   if LT rts;
   jump $audio_proc.peq.process;

Set_PassThroughGains:
   r4 = M[r8];
   NULL = r4 - $M.CVC_RECV.SYSMODE.PASS_THRU;
   if Z jump passthroughgains;

   // Standby - Zero Signal
   r2 = NULL;
   r3 = 1;
   jump setgains;
passthroughgains:
   // PassThrough Gains set from Parameters
   r0 = M[r7 + $M.SET_MODE_GAIN.PARAM_POINTER];
   r2 = M[r0];
   r3 = M[r0 + 1*ADDR_PER_WORD];
setgains:
   M[r7 + $M.SET_MODE_GAIN.MANT_LEFT] = r2;
   M[r7 + $M.SET_MODE_GAIN.EXP]       = r3;
   rts;

#if defined(INSTALL_OPERATOR_CVC_32K) || defined(INSTALL_OPERATOR_CVC_48K)
// cvc rcv resampler functions, r8: rcv_ewb
$cvc.rcv.frame_resample_process:
   Null = r8;
   if Z rts;
   jump $frame.iir_resamplev2.Process;
#endif

.ENDMODULE;
