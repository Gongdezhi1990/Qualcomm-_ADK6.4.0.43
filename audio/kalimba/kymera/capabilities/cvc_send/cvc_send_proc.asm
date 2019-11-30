// *****************************************************************************
// Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

#include "stack.h"
#include "portability_macros.h"
#include "shared_volume_control_asm_defs.h"
#include "cvc_send_gen_asm.h"
#include "cvc_send_cap_asm.h"

#include "shared_memory_ids_asm_defs.h"
#include "frame_iir_resamplerv2_asm_defs.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif

// *****************************************************************************
// MODULE:
//    void CVC_SEND_CAP_Config_auto_1mic(CVC_SEND_OP_DATA *op_extra_data, unsigned data_variant);
//    void CVC_SEND_CAP_Config_auto_2mic(CVC_SEND_OP_DATA *op_extra_data, unsigned data_variant);
//    void CVC_SEND_CAP_Config_headset_1mic(CVC_SEND_OP_DATA *op_extra_data, unsigned data_variant);
//    void CVC_SEND_CAP_Config_headset_2mic_mono(CVC_SEND_OP_DATA *op_extra_data, unsigned data_variant);
//    void CVC_SEND_CAP_Config_headset_2mic_binaural(CVC_SEND_OP_DATA *op_extra_data, unsigned data_variant);
//    void CVC_SEND_CAP_Config_speaker_1mic(CVC_SEND_OP_DATA *op_extra_data, unsigned data_variant);
//    void CVC_SEND_CAP_Config_speaker_2mic(CVC_SEND_OP_DATA *op_extra_data, unsigned data_variant);
//    void CVC_SEND_CAP_Config_speaker_3mic(CVC_SEND_OP_DATA *op_extra_data, unsigned data_variant);
//    void CVC_SEND_CAP_Config_speaker_4mic(CVC_SEND_OP_DATA *op_extra_data, unsigned data_variant);
//
// DESCRIPTION:
//    CVC SEND Configuration (per capability) (C-callable)
//
// MODIFICATIONS:
//
// INPUTS:
//    r0 - Extended Data
//    r1 - data_variant
//
// OUTPUTS:
//    - cVc send configuration data
//
// TRASHED REGISTERS:
//
// CPU USAGE:
// *****************************************************************************
#define $cvc_send.mic_config.ONE_MIC               MIC_CONFIG_DEFAULT
#define $cvc_send.mic_config.AUTO_2MIC             MIC_CONFIG_DEFAULT
#define $cvc_send.mic_config.HEADSET_MONO          MIC_CONFIG_ENDFIRE
#define $cvc_send.mic_config.HEADSET_BINAURAL      MIC_CONFIG_BROADSIDE
#define $cvc_send.mic_config.SPEAKER_2MIC          MIC_CONFIG_BROADSIDE + MIC_CONFIG_ENDFIRE
#define $cvc_send.mic_config.SPEAKER_MULTI_MIC     MIC_CONFIG_BROADSIDE

.MODULE $M.CVC_SEND_CAP.config.auto_1mic;
   .CODESEGMENT PM;

$_CVC_SEND_CAP_Config_auto_1mic:
   r2 = $cvc_send.mic_config.ONE_MIC;
   r3 = $cvc_send.AUTO;
   I3 = $M.CVC_SEND_CAP.auto_data_1mic.DynTable_Main;
   I7 = $M.CVC_SEND_CAP.auto_data_1mic.DynTable_Linker;
   r10 = 1;
   jump $cvc_send.dyn_config;
.ENDMODULE;


.MODULE $M.CVC_SEND_CAP.config.auto_2mic;
   .CODESEGMENT PM;

$_CVC_SEND_CAP_Config_auto_2mic:
   r2 = $cvc_send.mic_config.AUTO_2MIC;
   r3 = $cvc_send.AUTO;
   I3 = $M.CVC_SEND_CAP.auto_data_2mic.DynTable_Main;
   I7 = $M.CVC_SEND_CAP.auto_data_2mic.DynTable_Linker;
   r10 = 2;
   jump $cvc_send.dyn_config;
.ENDMODULE;


.MODULE $M.CVC_SEND_CAP.config.headset_1mic;
   .CODESEGMENT PM;

$_CVC_SEND_CAP_Config_headset_1mic:
   r2 = $cvc_send.mic_config.ONE_MIC;
   r3 = $cvc_send.HEADSET;
   I3 = $M.CVC_SEND_CAP.headset_data_1mic.DynTable_Main;
   I7 = $M.CVC_SEND_CAP.headset_data_1mic.DynTable_Linker;
   r10 = 1;
   jump $cvc_send.dyn_config;
.ENDMODULE;


.MODULE $M.CVC_SEND_CAP.config.headset_2mic;
   .CODESEGMENT PM;

$_CVC_SEND_CAP_Config_headset_2mic_binaural:
   r2 = $cvc_send.mic_config.HEADSET_BINAURAL;
   jump Config_headset_2mic;

$_CVC_SEND_CAP_Config_headset_2mic_mono:
   r2 = $cvc_send.mic_config.HEADSET_MONO;

Config_headset_2mic:
   r3 = $cvc_send.HEADSET;
   I3 = $M.CVC_SEND_CAP.headset_data_2mic.DynTable_Main;
   I7 = $M.CVC_SEND_CAP.headset_data_2mic.DynTable_Linker;
   r10 = 2;
   jump $cvc_send.dyn_config;
.ENDMODULE;


.MODULE $M.CVC_SEND_CAP.config.speaker_1mic;
   .CODESEGMENT PM;

$_CVC_SEND_CAP_Config_speaker_1mic:
   r2 = $cvc_send.mic_config.ONE_MIC;
   r3 = $cvc_send.SPEAKER;
   I3 = $M.CVC_SEND_CAP.speaker_data_1mic.DynTable_Main;
   I7 = $M.CVC_SEND_CAP.speaker_data_1mic.DynTable_Linker;
   r10 = 1;
   jump $cvc_send.dyn_config;
.ENDMODULE;


.MODULE $M.CVC_SEND_CAP.config.speaker_2mic;
   .CODESEGMENT PM;

$_CVC_SEND_CAP_Config_speaker_2mic:
   r2 = $cvc_send.mic_config.SPEAKER_2MIC;
   r3 = $cvc_send.SPEAKER;
   I3 = $M.CVC_SEND_CAP.speaker_data_2mic.DynTable_Main;
   I7 = $M.CVC_SEND_CAP.speaker_data_2mic.DynTable_Linker;
   r10 = 2;
   jump $cvc_send.dyn_config;
.ENDMODULE;

.MODULE $M.CVC_SEND_CAP.config.speaker_3mic;
   .CODESEGMENT PM;

$_CVC_SEND_CAP_Config_speaker_3mic:
   r2 = $cvc_send.mic_config.SPEAKER_MULTI_MIC;
   r3 = $cvc_send.SPEAKER;
   I3 = $M.CVC_SEND_CAP.speaker_data_3mic.DynTable_Main;
   I7 = $M.CVC_SEND_CAP.speaker_data_3mic.DynTable_Linker;
   r10 = 3;
   jump $cvc_send.dyn_config;
.ENDMODULE;

.MODULE $M.CVC_SEND_CAP.config.speaker_4mic;
   .CODESEGMENT PM;

$_CVC_SEND_CAP_Config_speaker_4mic:
   r2 = $cvc_send.mic_config.SPEAKER_MULTI_MIC;
   r3 = $cvc_send.SPEAKER;
   I3 = $M.CVC_SEND_CAP.speaker_data_4mic.DynTable_Main;
   I7 = $M.CVC_SEND_CAP.speaker_data_4mic.DynTable_Linker;
   r10 = 4;
   jump $cvc_send.dyn_config;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cvc_send.dyn_config
//
// DESCRIPTION:
//    Dynanic memory configuration
//
// MODIFICATIONS:
//
// INPUTS:
//    r0 - Extended Data
//    r1 - data_variant
//    r2 - mic_config
//    r3 - major_config
//   r10 - num_mics
//    I3 - DynTable_Main
//    I7 - DynTable_Linker
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//
// CPU USAGE:
// *****************************************************************************
.MODULE $M.CVC_SEND_CAP.dyn_config;
   .CODESEGMENT PM;

$cvc_send.dyn_config:
   M[r0 + $CVC_SEND_CAP.ROOT.NUM_MICS] = r10;
   M[r0 + $CVC_SEND_CAP.ROOT.DATA_VARIANT] = r1;
   M[r0 + $CVC_SEND_CAP.ROOT.MIC_CONFIG] = r2;
   M[r0 + $CVC_SEND_CAP.ROOT.MAJOR_CONFIG] = r3;
   r2 = I3;
   r3 = I7;
   M[r0 + $CVC_SEND_CAP.ROOT.DYN_MAIN]   = r2;
   M[r0 + $CVC_SEND_CAP.ROOT.DYN_LINKER] = r3;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    unsigned CVC_SEND_CAP_Create(CVC_SEND_OP_DATA *op_extra_data);
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
.MODULE $M.CVC_SEND_CAP.Create;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

$_CVC_SEND_CAP_Create:
   push rLink;
   pushm <r8,r9>;

   r8 = r0; // Extended Data

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($CVC_SEND_CAP_CREATE.PATCH_ID_0, r1)
#endif   

   //
   //  Reserve Scratch Data
   //
   r0 = r8;
   r1 = M[r8 + $CVC_SEND_CAP.ROOT.DYN_MAIN];
   r2 = M[r8 + $CVC_SEND_CAP.ROOT.DATA_VARIANT];
   call $_DynLoaderScratchReserve;
   NULL = r0;
   if Z CALL_PANIC_OR_JUMP(create_failed);

   //
   //  Allocate Shared Variables
   //

   // CVCLIB table
   r0 = r8  + $CVC_SEND_CAP.ROOT.CVCLIB_TABLE;
   call $_cvclib_table_alloc;
   r0 = M[r8  + $CVC_SEND_CAP.ROOT.CVCLIB_TABLE];
   if Z CALL_PANIC_OR_JUMP(create_failed);

   // filter_bank split table
   r0 = r8  + $CVC_SEND_CAP.ROOT.FFTSPLIT_TABLE;
   call $_dyn_load_filter_bank_split;
   r0 = M[r8  + $CVC_SEND_CAP.ROOT.FFTSPLIT_TABLE];
   if Z CALL_PANIC_OR_JUMP(create_failed);

   // filter_bank configuration table
   //    headset      - Hanning window
   //    auto/speaker - Custom window
   r2 = M[r8  + $CVC_SEND_CAP.ROOT.MAJOR_CONFIG];
   r2 = r2 - $cvc_send.HEADSET;
   r1 = M[r8 + $CVC_SEND_CAP.ROOT.IS_WIDEBAND];
   r0 = r8 + $CVC_SEND_CAP.ROOT.FILTERBANK_CONFIG;
   call $_dyn_load_filter_bank_config;
   r0 = M[r8 + $CVC_SEND_CAP.ROOT.FILTERBANK_CONFIG];
   if Z CALL_PANIC_OR_JUMP(create_failed);

   // OMS_IN
   r1 = M[r8 + $CVC_SEND_CAP.ROOT.IS_WIDEBAND]; //WB flag
   r0 = r8  + $CVC_SEND_CAP.ROOT.OMS_CONST;
   call $_oms270_AllocMem;
   r0 = M[r8  + $CVC_SEND_CAP.ROOT.OMS_CONST];
   if Z CALL_PANIC_OR_JUMP(create_failed);

   // SEND NS
   // speaker 2mic: linear, others: PBP
   // r2 -> PBP(0)/Linear(1) flag
   r2 = 1;
   r0 = M[r8 + $CVC_SEND_CAP.ROOT.MAJOR_CONFIG];
   Null = r0 - $cvc_send.SPEAKER;
   if NZ r2 = 0;
   r0 = M[r8 + $CVC_SEND_CAP.ROOT.NUM_MICS];
   NULL = r0 - 1;
   if Z r2 = 0;
   r0 = r8  + $CVC_SEND_CAP.ROOT.DMS100_MODE;
   r1 = M[r8 + $CVC_SEND_CAP.ROOT.IS_WIDEBAND];
   call $_dms100_AllocMem;
   r0 = M[r8  + $CVC_SEND_CAP.ROOT.DMS100_MODE];
   if Z CALL_PANIC_OR_JUMP(create_failed);

   // ASF
   r0 = M[r8 + $CVC_SEND_CAP.ROOT.NUM_MICS];
   NULL = r0 - 1;
   if Z jump jp_no_asf_share;
      r0 = r8 + $CVC_SEND_CAP.ROOT.ASF_MODE_TABLE;
      r1 = M[r8 + $CVC_SEND_CAP.ROOT.IS_WIDEBAND];
      call $_asf100_AllocMem;
      r0 = M[r8 + $CVC_SEND_CAP.ROOT.ASF_MODE_TABLE];
      if Z CALL_PANIC_OR_JUMP(create_failed);
   jp_no_asf_share:

   // DCBLOCK or REF_VAD
   r0 = r8 + $CVC_SEND_CAP.ROOT.VAD_DC_COEFFS;
   r1 = M[r8 + $CVC_SEND_CAP.ROOT.IS_WIDEBAND];
   call $_vad400_AllocMem;
   r0 = M[r8 + $CVC_SEND_CAP.ROOT.VAD_DC_COEFFS];
   if Z CALL_PANIC_OR_JUMP(create_failed);

	// AEC
   r0 = r8 + $CVC_SEND_CAP.ROOT.AEC_MODE;
   r1 = M[r8 + $CVC_SEND_CAP.ROOT.IS_WIDEBAND];
   call $_aec510_AllocMem;
   r0 = M[r8 + $CVC_SEND_CAP.ROOT.AEC_MODE];
   if Z CALL_PANIC_OR_JUMP(create_failed);

#if defined(INSTALL_OPERATOR_CVC_32K) || defined(INSTALL_OPERATOR_CVC_48K)
   // Insert resamplers?
   r1 = M[r8 + $CVC_SEND_CAP.ROOT.DATA_VARIANT];
   r0 = 0;
   r2 = $shared_memory_ids.IIRV2_RESAMPLER_Up_1_Down_2;  // SWB: 32k -> 16k 
   r3 = $shared_memory_ids.IIRV2_RESAMPLER_Up_1_Down_3;  // FB:  48k -> 16k 
   Null = r1 - DATA_VARIANT_SWB;
   if Z r0 = r2;
   Null = r1 - DATA_VARIANT_FB;
   if Z r0 = r3;

   NULL = r0;
   if Z jump jp_resample_done;

      //setup down sampling filter
      call $_iir_resamplerv2_allocate_config_by_id;
      M[r8 + $CVC_SEND_CAP.ROOT.DOWNSAMPLE_CONFIG_FIELD] = r0;  //r0: allocated filter pointer
      if Z CALL_PANIC_OR_JUMP(create_failed);

      //Output: check which up sampling filter to use 
      r2 = $shared_memory_ids.IIRV2_RESAMPLER_Up_2_Down_1;  // SWB: 16k -> 32k
      r3 = $shared_memory_ids.IIRV2_RESAMPLER_Up_3_Down_1;  // FB:  16k -> 48k
      r1 = M[r8 + $CVC_SEND_CAP.ROOT.DATA_VARIANT];
      Null = r1 - DATA_VARIANT_SWB;
      if Z r0 = r2;
      Null = r1 - DATA_VARIANT_FB;
      if Z r0 = r3;

      //setup up-sampling filter
      call $_iir_resamplerv2_allocate_config_by_id;
      M[r8 + $CVC_SEND_CAP.ROOT.UPSAMPLE_CONFIG_FIELD] = r0;    //r0: allocated filter pointer
      if Z CALL_PANIC_OR_JUMP(create_failed);
jp_resample_done:
#endif

   // Set the default mode and current mode pointer field
   r0 = r8 + $CVC_SEND_CAP.ROOT.CUR_MODE;
   M[r8 + $CVC_SEND_CAP.ROOT.CUR_MODE_PTR_FIELD] = r0;

   // Set MGDC persistent state pointer
   r0 = r8 + $CVC_SEND_CAP.ROOT.MGDC_STATE_FIELD;
   M[r8 + $CVC_SEND_CAP.ROOT.MGDC_STATE_PTR_FIELD] = r0;

   //
   //  Allocate Persistent Data
   //
   r0 = r8;
   r1 = M[r8 + $CVC_SEND_CAP.ROOT.DYN_MAIN];
   r2 = M[r8 + $CVC_SEND_CAP.ROOT.DYN_LINKER];
   r3 = M[r8 + $CVC_SEND_CAP.ROOT.DATA_VARIANT];
   call $_DynLoaderProcessDynamicAllocations;
   NULL = r0;
   if Z CALL_PANIC_OR_JUMP(create_failed);

   r0 = 0;                                                         // create succeeded
create_done:
   popm <r8,r9>;
   pop rLink;
   rts;
create_failed:
   r0 = 1;                                                         // create failed
   jump create_done;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    void CVC_SEND_CAP_Destroy(CVC_SEND_OP_DATA *op_extra_data);
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
.MODULE $M.CVC_SEND_CAP.Destroy;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

$_CVC_SEND_CAP_Destroy:
   push rLink;
   push r8;

   r8 = r0;  // extended data

   // unregister component
   call $Security.UnregisterComponent;

   // Release Scratch Data
   r0 = M[r8 + $CVC_SEND_CAP.ROOT.SCRATCH_ALLOC_PTR_FIELD];
   if NZ call $_DynLoaderDeregisterScratch;
   M[r8 + $CVC_SEND_CAP.ROOT.SCRATCH_ALLOC_PTR_FIELD]=NULL;

   // Release Shared Variables
   r0 = M[r8  + $CVC_SEND_CAP.ROOT.CVCLIB_TABLE];
   if NZ call $_cvclib_table_release;
   M[r8  + $CVC_SEND_CAP.ROOT.CVCLIB_TABLE] = NULL;

   r0 = M[r8 + $CVC_SEND_CAP.ROOT.ASF_MODE_TABLE];
   if NZ call $_asf100_ReleaseMem;
   M[r8 + $CVC_SEND_CAP.ROOT.ASF_MODE_TABLE] = NULL;

   r0 = M[r8 + $CVC_SEND_CAP.ROOT.DMS100_MODE];
   if NZ call $_dms100_ReleaseMem;
   M[r8 + $CVC_SEND_CAP.ROOT.DMS100_MODE]= NULL;

   r0 = M[r8 + $CVC_SEND_CAP.ROOT.OMS_CONST];
   if NZ call $_dms100_ReleaseMem;
   M[r8 + $CVC_SEND_CAP.ROOT.OMS_CONST]=NULL;

   r0 = M[r8 + $CVC_SEND_CAP.ROOT.VAD_DC_COEFFS];
   if NZ call $_vad400_ReleaseMem;
   M[r8 + $CVC_SEND_CAP.ROOT.VAD_DC_COEFFS]=NULL;

   r0 = M[r8  + $CVC_SEND_CAP.ROOT.FFTSPLIT_TABLE];
   if NZ call $_dyn_free_filter_bank_split;
   M[r8  + $CVC_SEND_CAP.ROOT.FFTSPLIT_TABLE]=NULL;

   r0 = M[r8 + $CVC_SEND_CAP.ROOT.FILTERBANK_CONFIG];
   if NZ call $_dyn_free_filter_bank_config;
   M[r8 + $CVC_SEND_CAP.ROOT.FILTERBANK_CONFIG]=NULL;

   r0 = M[r8 + $CVC_SEND_CAP.ROOT.AEC_MODE];
   if NZ call $_aec510_ReleaseMem;
   M[r8 + $CVC_SEND_CAP.ROOT.AEC_MODE] = NULL;

#if defined(INSTALL_OPERATOR_CVC_32K) || defined(INSTALL_OPERATOR_CVC_48K)
   r0 = M[r8 + $CVC_SEND_CAP.ROOT.DOWNSAMPLE_CONFIG_FIELD];
   if NZ call $_iir_resamplerv2_release_config;
   M[r8 + $CVC_SEND_CAP.ROOT.DOWNSAMPLE_CONFIG_FIELD]=NULL;

   r0 = M[r8 + $CVC_SEND_CAP.ROOT.UPSAMPLE_CONFIG_FIELD];
   if NZ call $_iir_resamplerv2_release_config;
   M[r8 + $CVC_SEND_CAP.ROOT.UPSAMPLE_CONFIG_FIELD]=NULL;
#endif

   //  deallocate Persistent Data
   r0 = M[r8 + $CVC_SEND_CAP.ROOT.INST_ALLOC_PTR_FIELD];
   if NZ call $_DynLoaderReleaseDynamicAllocations;
   M[r8 + $CVC_SEND_CAP.ROOT.INST_ALLOC_PTR_FIELD]=NULL;

   pop r8;
   pop rLink;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    void CVC_SEND_CAP_Initialize(CVC_SEND_OP_DATA *op_extra_data);
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
.MODULE $M.CVC_SEND_CAP.Initialize;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

$_CVC_SEND_CAP_Initialize:
   M[r0 + $CVC_SEND_CAP.ROOT.ALGREINIT]  = Null;
   r1 = M[r0 + $CVC_SEND_CAP.ROOT.CVC_DATA_ROOT_FIELD];
   r0 = M[r0 + $CVC_SEND_CAP.ROOT.INIT_TABLE_PTR_FIELD];
   jump $_run_frame_proc_function_table;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    void CVC_SEND_CAP_Process(CVC_SEND_OP_DATA *op_extra_data);
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
.MODULE $M.CVC_SEND_CAP.Process;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

$_CVC_SEND_CAP_Process:
   push rLink;
   push r8;

   r8 = r0;  // extended data

   r0 = M[r8 + $CVC_SEND_CAP.ROOT.INST_ALLOC_PTR_FIELD];
   r1 = M[r8 + $CVC_SEND_CAP.ROOT.SCRATCH_ALLOC_PTR_FIELD];
   if NZ call $_DynLoaderScratchCommit;

   r0 = r8;  // extended data
   NULL = M[r8 + $CVC_SEND_CAP.ROOT.ALGREINIT];
   if NZ call $_CVC_SEND_CAP_Initialize;    // preserves FP

   // Pass in Current Mode & Overide Control
   r1 = M[r8 + $CVC_SEND_CAP.ROOT.CUR_MODE];
   r2 = M[r8 + $CVC_SEND_CAP.ROOT.OVR_CONTROL];
   call $cvc_send.process_data;
   
   // r2 -> Status Table
   r2 = M[r8 + $CVC_SEND_CAP.ROOT.STATUS_TABLE_PTR_FIELD];
   r2 = r2 - $M.CVC_SEND.STATUS.COMPILED_CONFIG;

   // Update NDVC shared
   // r2 -> Status Table
   r1 = M[r8 + $CVC_SEND_CAP.ROOT.NDVC_SHARE_PTR_FIELD];

   // Update NDVC shared variable with NDVC current volume level
   r3 = M[r2 + $M.CVC_SEND.STATUS.NDVC_VOL_ADJ_OFFSET];
   r3 = M[r3];
   M[r1+$shared_volume_control._shared_volume_struct.NDVC_NOISE_LEVEL_FIELD] = r3;
   r3 = M[r2 + $M.CVC_SEND.STATUS.NDVC_NOISE_EST_OFFSET];
   r3 = M[r3];
   M[r1+$shared_volume_control._shared_volume_struct.NDVC_FILTER_SUM_LPDNZ_FIELD] = r3;

   // Release committed scratch
   NULL =  M[r8 + $CVC_SEND_CAP.ROOT.SCRATCH_ALLOC_PTR_FIELD];
   if NZ call $_scratch_free;

   pop r8;     // remove local variables
   pop rLink;
   rts;

#if defined(INSTALL_OPERATOR_CVC_32K) || defined(INSTALL_OPERATOR_CVC_48K)
// cvc send resampler functions
$cvc.snd.frame_resample_process:
   Null = r8 - DATA_VARIANT_UWB;
   if LE rts;
   jump $frame.iir_resamplev2.Process;
#endif
   

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    bool cvc_send_opmsg_get_voice_quality(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
//
// DESCRIPTION:
//
// MODIFICATIONS:
//
// INPUTS:
//    - r0 - op_data ptr
//    - r1 - message_data ptr
//    - r2 - resp_length ptr
//    - r3 - resp_data ptr ptr
//
// OUTPUTS:
//    - r0 - result
//
// TRASHED REGISTERS:
//    - 
//
// CPU USAGE:
// *****************************************************************************
.MODULE $M.cvc_send_opmsg_get_voice_quality;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

$_cvc_send_opmsg_get_voice_quality:
   
#if defined(PATCH_LIBS)
   pushm <r0,r1,r2,r3>;
   LIBS_SLOW_SW_ROM_PATCH_POINT($CVC_Send_Set_Voice_Quality.PATCH_ID, r1)
   popm <r0,r1,r2,r3>;
#endif

   r0 = 1;
   rts;
   
.ENDMODULE;