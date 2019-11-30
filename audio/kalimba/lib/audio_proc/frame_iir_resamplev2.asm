// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    IIR resampler framesync Processing Module
//
// DESCRIPTION:
//    This processing mdoule uses an IIR and a FIR filter combination to
//    perform sample rate conversion.  The utilization of the IIR
//    filter allows a lower order FIR filter to be used to obtain
//    an equivalent frequency response.  The result is that the
//    IIR resampler uses less MIPs than the current polyphase FIR method.
//    It also provides a better frequency response.
//
//    To further reduce coefficients for a given resampling up to two
//    complete filter stages are supported.  The configurations include.
//          IIR --> FIR(10)
//          FIR(10) --> IIR
//          FIR(6) --> FIR(10) --> IIR
//          FIR(6) --> IIR --> FIR(10)
//          IIR --> FIR(10) --> IIR --> FIR(10)
//          IIR --> FIR(10) --> FIR(10) --> IIR
//          FIR(10) --> IIR --> IIR --> FIR(10)
//          FIR(10) --> IIR --> FIR(10) --> IIR
//
//    The IIR filter may be from 9th order to 19 order.
//
//    The FIR filters are implemented in a polyphase configuration. The FIR(6)
//    filter uses a 6th order polyphase kernal and the FIR(10) filter uses a
//    10th order polyphase kernal.  The filters are symetrical so only half the
//    coefficients need to be stored.
//
//    The operator utilizes its own history buffers and in-place operation is supported
//    for downsampling.  Because frame processing is used a fixed frame size only
//    resampling ratios where the numerator or denominator is one are supported.
//      Such as 1:2, 2:1, 1:4, etc.
//    As the frame size is fixed available data/space in buffers is not validated
//
// When using the operator the following data structure is used:
//
//
// *****************************************************************************

#include "stack.h"
#include "portability_macros.h"

#ifdef KYMERA

#include "frame_iir_resamplerv2_asm_defs.h"
#include "iir_resamplerv2_common_asm_defs.h"

#else

#include "frame_iir_resamplerv2_static.h"

#endif

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif



// Private Library Exports
.PUBLIC $frame.iir_resamplev2.Initialize;
.PUBLIC $frame.iir_resamplev2.Process;

#ifdef KYMERA
#define IIR_RESAMPLEV2_FRAMESYNC_PM	EXT_DEFINED_PM
#endif


// *****************************************************************************
// MODULE:
//    $frame.iir_resamplev2.Initialize
//
// DESCRIPTION:
//    Initialize function for IIR resampler processing module
//
// INPUTS:
//    - r7 = pointer to module structure
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//    all
//
// *****************************************************************************
.MODULE $M.frame.iir_resamplev2.initialize;
   .CODESEGMENT IIR_RESAMPLEV2_FRAMESYNC_PM;

   $frame.iir_resamplev2.Initialize:
   
//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.FRAME_IIR_RESAMPLEV2_ASM.FRAME.IIR_RESAMPLEV2.INITIALIZE.PATCH_ID_0, r4)     // peq_coeff_patchers1
//#endif
   

   /* History Buffers appended to end of data structure. This does not use multi-channel framework,
    * therefore can directly get to the common, channel and working fields in the param struct.
    * It always works with a single in and out buffer.
    */
   r4 = r7 + $frame_iir_resamplerv2.iir_resampler_op_struct.WORKING_FIELD;
   r0 = r7 + $frame_iir_resamplerv2.iir_resampler_op_struct.COMMON_FIELD;
   r8 = r7 + $frame_iir_resamplerv2.iir_resampler_op_struct.CHANNEL_FIELD;
   jump $reset_iir_resampler;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $frame.iir_resamplev2.process
//
// DESCRIPTION:
//    Process function for IIR resampler processing module
//
// INPUTS:
//    - r7  = pointer to module structure
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//    everything
//
// *****************************************************************************

.MODULE $M.frame.iir_resamplev2.process;
   .CODESEGMENT IIR_RESAMPLEV2_FRAMESYNC_PM;

   $frame.iir_resamplev2.Process:

   push rLink;
   
//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.FRAME_IIR_RESAMPLEV2_ASM.FRAME.IIR_RESAMPLEV2.PROCESS.PATCH_ID_0, r1)     // peq_coeff_patchers1
//#endif
   
   M0 = MK1;

   I0 = r7;
   push r7,             r0=M[I0,M0];      //INPUT_PTR_FIELD
   // Get Input Buffer
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B1;
   I1  = r0;
   r10 = r3,            r0=M[I0,M0];      // OUTPUT_PTR_FIELD
   L1  = r1;
   // Get output buffer
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B5;
   L5 = r1;
   I5 = r0;

   /* History Buffers appended to end of data structure. This does not use multi-channel framework,
    * therefore can directly get to the common, channel and working fields in the param struct.
    * It always works with a single in and out buffer.
    */
   r4 = r7 + $frame_iir_resamplerv2.iir_resampler_op_struct.WORKING_FIELD;
   r0 = r7 + $frame_iir_resamplerv2.iir_resampler_op_struct.COMMON_FIELD;
   r8 = r7 + $frame_iir_resamplerv2.iir_resampler_op_struct.CHANNEL_FIELD;
   call $iir_perform_resample;

   // Restore Data Object
   pop r8;

   // Update output frame size
   r3 = r7;    // amount produced by resampler
   r0 = M[r8 + $frame_iir_resamplerv2.iir_resampler_op_struct.OUTPUT_FRAME_PTR_FIELD];
   call $frmbuffer.set_frame_size;

   // pop rLink from stack
   jump $pop_rLink_and_rts;


.ENDMODULE;





