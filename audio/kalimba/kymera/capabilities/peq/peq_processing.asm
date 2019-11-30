// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

// ASM function for peq operator data processing
// The function(s) obey the C compiler calling convention (see documentation, CS-124812-UG)

#include "core_library.h"
#include "cbuffer_asm.h"
#include "portability_macros.h"
#include "peq.h"
#include "peq_gen_asm.h"
#include "stack.h"
#include "coeff_gen.h"
#include "peq_wrapper_defs.h"
#include "peq_wrapper_asm_defs.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif


// *****************************************************************************
// MODULE:
//    $_peq_processing
//
// DESCRIPTION:
//    Data processing function.
//
// INPUTS:
//    - r0 = PEQ "extra data" object
//    - r1 = peq_channels *first
//    - r2 = number of samples to process
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
.MODULE $M.peq_proc;
    .CODESEGMENT PM;
$_peq_processing:

   // for now, we assume we get to the buffer parameters directly with some offset constants
   PUSH_ALL_C
   
#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($peq_cap.PEQ_PROCESSING_ASM.PEQ_PROC.PEQ_PROCESSING.PATCH_ID_0, r4)     // cap_peq_patchers
#endif
   
   

   // set r9 to extra_op_data
   r9 = r0;
   // number of samples to process, we don't check validity, caller responsible
   r4 = r2;
   // Pointer to first channel
   r6 = r1;


   // for(ptr=first_active;ptr;ptr=ptr->next_active)
   peq_channel_loop:
         // get read pointer, size and start addresses of input buffer
         r0 = M[r6 + $peq_wrapper.multi_chan_channel_struc_struct.SINK_BUFFER_PTR_FIELD];
         call $cbuffer.get_read_address_and_size_and_start_address;
         push r2;
         pop B4;
         I4 = r0;
         L4 = r1;
   
         // get write pointer, size and start addresses of input buffer
         r0 = M[r6 + $peq_wrapper.multi_chan_channel_struc_struct.SOURCE_BUFFER_PTR_FIELD];
         call $cbuffer.get_write_address_and_size_and_start_address;
         push r2;
         pop B0;
         I0 = r0;
         L0 = r1;
   
         // call PEQ stream-based processing entry point
         pushm <r4,r6,r9>;
         pushm <I0,I4,L0,L4>;
         pushm <B0,B4>;


         // call the approriate process function
         r5 = M[r9 + $peq_wrapper.peq_exop_struct.PEQ_PROC_FUNC_FIELD];
		 // check bypass flag - force passthrough if set
		 r3 = M[r9 + ($peq_wrapper.peq_exop_struct.PEQ_CAP_PARAMS_FIELD + $M.PEQ.PARAMETERS.OFFSET_PEQ_CONFIG)];
		 r7 = $M.peq_proc.peq_pass_through;
         Null = $M.PEQ.CONFIG.BYPASS AND r3; 
		 if NZ r5 = r7;
		 // apply the PEQ to the current channel
         r7 = M[r6 + $peq_wrapper.peq_channels_struct.PEQ_OBJECT_FIELD];
		 
         call r5;
		 
         popm <B0,B4>;
         popm <I0,I4,L0,L4>;
         popm <r4,r6,r9>;

         // Compute amount to advance read/write pointer
         r0  = r4 * MK1 (int);
         M3  = r0;
         r0 = M[I0,M3], r1 = M[I4,M3];
                 
         // update read address in input buffer
         r0  = M[r6 + $peq_wrapper.multi_chan_channel_struc_struct.SINK_BUFFER_PTR_FIELD];
         r1 = I4;
         call $cbuffer.set_read_address;
         // update write address in output buffer
         r0 = M[r6 + $peq_wrapper.multi_chan_channel_struc_struct.SOURCE_BUFFER_PTR_FIELD];
         r1 = I0;
         call $cbuffer.set_write_address;
 
   // next chan
   r6 = M[r6 + $peq_wrapper.multi_chan_channel_struc_struct.NEXT_ACTIVE_FIELD];
   if NZ jump peq_channel_loop;

   // Clear B0,B4
   push NULL;
   B0 = M[SP-MK1];
   pop B4;
   // L0 and L4 reset in POP_ALL_C

   POP_ALL_C
   rts;

peq_mute:
   r10 = r4;
   r0 = 0;
   do mute_loop;
         M[I0, MK1] = r0;
   mute_loop:
   rts;
peq_pass_through:
   r10 = r4;
   r0 = M[I4, MK1];
   do bypass_loop;
      r0 = M[I4, MK1], M[I0, MK1] = r0;
   bypass_loop:
   rts;


.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $_peq_initialize
//
// DESCRIPTION:
//    Initialize function
//
// INPUTS:
//    - r0 = PEQ "extra data" object
//    - r1 = peq_channels *first
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
.MODULE $M.peq_initialize;
    .CODESEGMENT PM;
$_peq_initialize:

   PUSH_ALL_C
   
   r9 = r0;
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($peq_cap.PEQ_PROCESSING_ASM.PEQ_INITIALIZE.PEQ_INITIALIZE.PATCH_ID_0, r4)     // cap_peq_patchers
#endif   
   r4 = M[r9 + $peq_wrapper.peq_exop_struct.CUR_MODE_FIELD];

   // pass-thru mode
   r3 = $M.peq_proc.peq_pass_through;
   Null = r4 - $M.PEQ.SYSMODE.PASS_THRU;
   if Z jump jp_channels_done;

   // Mute
   r3 = $M.peq_proc.peq_mute;
   Null = r4 - $M.PEQ.SYSMODE.FULL;
   if NZ jump jp_channels_done;

   // First active channel
   r5 = r1;
   if Z jump jp_channels_done;

   peq_channel_loop:
      pushm <r5,r9>;
      // initilize all of the peq data objects
      r7 = M[r5 + $peq_wrapper.peq_channels_struct.PEQ_OBJECT_FIELD];

      // Associate coefficients */
      r0 = M[r9 + $peq_wrapper.peq_exop_struct.PEQ_COEFF_PARAMS_FIELD];   
      M[r7 + $audio_proc.peq.PARAM_PTR_FIELD]=r0;
   
      r0 = M[r9 + ($peq_wrapper.peq_exop_struct.PEQ_CAP_PARAMS_FIELD + $peq_wrapper._tag_PEQ_PARAMETERS_struct.OFFSET_CORE_TYPE_FIELD)];
      Null = r0 - PEQ_CORE_MODE_DH;
      if Z jump dh_option;
      Null = r0 - PEQ_CORE_MODE_HQ;
      if Z jump hq_option; 
         // sh_option
         call $audio_proc.sh_peq.zero_delay_data;
         call $audio_proc.sh_peq.initialize;
         r3 = $audio_proc.sh_peq.process_op;
         jump done_init_filter;
      hq_option:
         call $audio_proc.hq_peq.zero_delay_data;
         call $audio_proc.hq_peq.initialize;
         r3 = $audio_proc.hq_peq.process_op;
         jump done_init_filter;
      dh_option:
         call $audio_proc.dh_peq.zero_delay_data;
         call $audio_proc.dh_peq.initialize;
         r3 = $audio_proc.dh_peq.process_op;  
      done_init_filter:

      popm <r5,r9>;
      // next chan
      r5 = M[r5 + $peq_wrapper.multi_chan_channel_struc_struct.NEXT_ACTIVE_FIELD];
   if NZ jump peq_channel_loop;

jp_channels_done:
   // Save PEQ function 
   M[r9 + $peq_wrapper.peq_exop_struct.PEQ_PROC_FUNC_FIELD] = r3;
   // clear reinit flag
   M[r9 + $peq_wrapper.peq_exop_struct.REINITFLAG_FIELD] = Null;

   POP_ALL_C
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $_peq_compute_coefficients
// extern void peq_compute_coefficients(PEQ_OP_DATA *op_data,t_peq_params *dest);
//
// DESCRIPTION:
//    compute the filter coefficients in stages
//       Parameer load status < 0 to restart coeffient computation
//       Parameer load status == 0 if computation is completed for all stages
//       Parameer load status > 0 indicates stage to compute coefficient for
//
// INPUTS:
//    - r0 = PEQ "extra data" object
//    - r1 = destination pointer 
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************

.CONST $peq_wrapper.PARAMETERS_STAGE_SIZE    ($peq_wrapper._tag_PEQ_PARAMETERS_struct.OFFSET_STAGE2_TYPE_FIELD - $peq_wrapper._tag_PEQ_PARAMETERS_struct.OFFSET_STAGE1_TYPE_FIELD);

.MODULE $M.peq_compute_coefficients;
    .CODESEGMENT PM;
$_peq_compute_coefficients:

   PUSH_ALL_C  

#if 0 
   // generate filter coefficients from OBPM parameters
   r6 = M[r0 + $peq_wrapper.peq_exop_struct.SAMPLE_RATE_FIELD];
   r7 = r1;
   r8 = r0 + ($peq_wrapper.peq_exop_struct.PEQ_CAP_PARAMS_FIELD + $peq_wrapper._tag_PEQ_PARAMETERS_struct.OFFSET_NUM_BANDS_FIELD);
  
   push r0;
   call $peq_cap_gen_coeffs;
   pop r0;

   // Signal coefficients updated
   M[r0 + $peq_wrapper.peq_exop_struct.PARAMETERLOADSTATUS_FIELD]=NULL;

#else

   r9 = r0;
   r7 = r1;

   r0 = M[r9 + $peq_wrapper.peq_exop_struct.PARAMETERLOADSTATUS_FIELD];
   if POS jump jp_gain_calc_done;     
       // i0 = &Cur_params+num_bands
       I0 = r9 + ($peq_wrapper.peq_exop_struct.PEQ_CAP_PARAMS_FIELD + $peq_wrapper._tag_PEQ_PARAMETERS_struct.OFFSET_NUM_BANDS_FIELD);
       // i1 = address of "coefficient params" object
       I1 = r7;            

       // first copy number of EQ bands to coefficient storage.
       r0 = m[I0,MK1];
       m[I1,MK1] = r0;

       // Set Up Gains
       pushm <r7,r9>;
       call $M.peq_gen_coeffs.calcPreGain.call_entry;
       popm <r7,r9>;

       // Do 1st stage
       r0 = 0;
   jp_gain_calc_done:

   // r0 is param_status, r1=num_stages
   r1 = M[r7];
   if Z jump jp_done;

       // Pointer to stage in coefficients
       r7 = r7 + $peq_wrapper.t_peq_params_struct.COEFFS_SCALES_FIELD;
       r2 = r0 * ($peq_wrapper.t_coeffs_scales_struct.STRUC_SIZE * ADDR_PER_WORD) (int);
       I1 = r7 + r2;
    
       // Pointer to stage in parameters
       I0 = r9 + ($peq_wrapper.peq_exop_struct.PEQ_CAP_PARAMS_FIELD + $M.PEQ.PARAMETERS.OFFSET_STAGE1_TYPE); 
       r2 = r0 * $peq_wrapper.PARAMETERS_STAGE_SIZE (int);  
       I0 = I0 + r2;
    
       // generate filter coefficients for stage from OBPM parameters
       pushm <r0,r1,r9>;
       r6 = M[r9 + $peq_wrapper.peq_exop_struct.SAMPLE_RATE_FIELD];
       call $M.peq_gen_coeffs.coefCalcBiquadBand.call_entry;
       popm <r0,r1,r9>;

jp_done:
   // Update parameter load status
   r0 = r0 + 1;         // stage++;
   NULL = r1 - r0;      // if (num_stages <= stage)stage=0;
   if LE r0 = NULL;
   M[r9 + $peq_wrapper.peq_exop_struct.PARAMETERLOADSTATUS_FIELD]=r0;

#endif

   POP_ALL_C
   rts;

.ENDMODULE;




