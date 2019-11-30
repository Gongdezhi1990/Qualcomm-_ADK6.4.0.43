// *****************************************************************************
// Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "portability_macros.h"
#include "mic_grouping.h"
//******************************************************************************
// MODULE:
//    $audioproc.multimic_to_2mic_mapping
//
// DESCRIPTION:
//  Group the 3 and 4 channel microphone inputs to 2-channels 
//   switch S.M_Mic
//    case 3
//        Ibuf_D(:,1) = 2/3*Ibuf_d(:,1)+1/3*Ibuf_d(:,2);
//        Ibuf_D(:,2) = 2/3*Ibuf_d(:,3)+1/3*Ibuf_d(:,2);
//    case 4
//        Ibuf_D(:,1) = 1/2*Ibuf_d(:,1)+1/2*Ibuf_d(:,2);
//        Ibuf_D(:,2) = 1/2*Ibuf_d(:,3)+1/2*Ibuf_d(:,4);
//   end 
// INPUTS:
//    - r7 = Pointer to the mic grouping data object
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    - TBD
//
// CPU USAGE:
//    - TBD
//******************************************************************************
.MODULE $M.audioproc.multimic_to_2mic_mapping;

      .CODESEGMENT   AUDIO_PROC_MULTIMIC_TO_2MIC_MAPPING_PM;

$audioproc.multimic_to_2mic_mapping:

   push rLink;
   //*****************************************************************************
   // Get the input left channel buffer
   //*****************************************************************************
   r0 = M[r7 + $audio_proc.mic_grouping.LEFT_CHANNEL_FIELD];     
#ifdef BASE_REGISTER_MODE  
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B0;
#else
   call $frmbuffer.get_buffer;
#endif
   I0 = r0;    //left_channel
   L0 = r1;
   
   //*****************************************************************************
   // Get the input right channel buffer
   //*****************************************************************************
   r0 = M[r7 + $audio_proc.mic_grouping.RIGHT_CHANNEL_FIELD];     
#ifdef BASE_REGISTER_MODE  
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B4;
#else
   call $frmbuffer.get_buffer;
#endif
   I4 = r0;    // right channel
   L4 = r1;
   
   //*****************************************************************************
   // Get the input mic3 channel buffer
   //*****************************************************************************
   r0 = M[r7 + $audio_proc.mic_grouping.MIC3_CHANNEL_FIELD];     
#ifdef BASE_REGISTER_MODE  
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B1;
#else
   call $frmbuffer.get_buffer;
#endif
   I1 = r0;    // mic3 channel
   L1 = r1;
   
   //*****************************************************************************
   // Call the 3-mic/4-mic to 2-mic mapping function
   //***************************************************************************** 
   r1 = $audioproc.3mic_to_2mic_mapping;
   r2 = $audioproc.4mic_to_2mic_mapping;
   r0 = M[r7 + $audio_proc.mic_grouping.NUM_MICS_FIELD];
   Null = r0 - $audio_proc.mic_grouping.IS_THREE_MICS;
   if NZ r1 = r2;                                     
   call r1;
   
   //*****************************************************************************
   // Clear the length and base registers
   //***************************************************************************** 
   L0 = Null;
   L1 = Null;
   L4 = Null;
   L5 = Null;
   
   push Null;
#ifdef BASE_REGISTER_MODE
   B0 = M[SP-1*ADDR_PER_WORD];
   B1 = M[SP-1*ADDR_PER_WORD];
   B4 = M[SP-1*ADDR_PER_WORD];
#endif
   pop B5;

   jump $pop_rLink_and_rts;


.ENDMODULE;


//******************************************************************************
// MODULE:
//    $audioproc.3mic_to_2mic_mapping
//
// DESCRIPTION:
//  Groups the 3 channel microphone inputs to 2-channels 
//  The arrangement of the mics follows the pattern : Left_mic(1) Middle_mic(2) Right_mic(3)
//        Ibuf_D(:,1) = 2/3 * Ibuf_d(:,1) + 1/3 * Ibuf_d(:,2);
//        Ibuf_D(:,2) = 2/3 * Ibuf_d(:,3) + 1/3 * Ibuf_d(:,2);
// INPUTS:
//    - r7        - Pointer to the mic grouping data object
//    - r3        - Frame size
//    - I0,L0,B0  - Input/output left channel
//    - I4,L4,B4  - Input/output right channel
//    - I1,L1,B1  - Input mic2 channel
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    - TBD
//
// CPU USAGE:
//    - TBD
//******************************************************************************
.MODULE $M.audioproc.3mic_to_2mic_mapping;

      .CODESEGMENT   AUDIO_PROC_3MIC_TO_2MIC_MAPPING_PM;

$audioproc.3mic_to_2mic_mapping:

   push rLink;

   r5 = $audio_proc.mic_grouping.TWO_OVER_THREE;
   r6 = $audio_proc.mic_grouping.ONE_OVER_THREE;
   r10 = r3;
   M1 = MK1;
   r1 = M[I0,0];                                               // preload left_channel
   do three_to_2mic_mapping_loop;
      rMAC = r5 * r1 , r3 = M[I1,MK1] , r2 = M[I4,0];          // Ibuf_D(:,1) = 2/3 * Ibuf_d(:,1) , load middle_channel , load right_channel
      rMAC = rMAC + r6 * r3;                                   // Ibuf_D(:,1) = 2/3 * Ibuf_d(:,1) + 1/3 * Ibuf_d(:,2);
      r4 = r5 * r2;                                            // Ibuf_D(:,2) = 2/3 * Ibuf_d(:,3)
      r4 = r4 + r6 * r3 , M[I0,M1] = rMAC;                     // Ibuf_D(:,2) = 2/3 * Ibuf_d(:,3) + 1/3 * Ibuf_d(:,2) , write left_channel            
      r1 = M[I0,0] , M[I4,MK1] = r4;                           // load left_channel , write right_channel
   three_to_2mic_mapping_loop:

   jump $pop_rLink_and_rts;


.ENDMODULE;


//******************************************************************************
// MODULE:
//    $audioproc.4mic_to_2mic_mapping
//
// DESCRIPTION:
//  Groups the 4 channel microphone inputs to 2-channels 
//  The arrangement of the mics follows the pattern : Left_mic(1) Middle_mic1(2) Middle_mic2(3) Right_mic(4)
//        Ibuf_D(:,1) = 1/2 * Ibuf_d(:,1) + 1/2 * Ibuf_d(:,2);
//        Ibuf_D(:,2) = 1/2 * Ibuf_d(:,3) + 1/2 * Ibuf_d(:,4);
// INPUTS:
//    - r7        - Pointer to the mic grouping data object
//    - r3        - Frame size
//    - I0,L0,B0  - Input/output left channel
//    - I4,L4,B4  - Input/output right channel
//    - I1,L1,B1  - Input mic2 channel
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    - TBD
//
// CPU USAGE:
//    - TBD
//******************************************************************************
.MODULE $M.audioproc.4mic_to_2mic_mapping;

      .CODESEGMENT   AUDIO_PROC_4MIC_TO_2MIC_MAPPING_PM;

$audioproc.4mic_to_2mic_mapping:

   push rLink;

   //*****************************************************************************
   // Get the input mic4 channel buffer
   //*****************************************************************************
   r0 = M[r7 + $audio_proc.mic_grouping.MIC4_CHANNEL_FIELD];     
#ifdef BASE_REGISTER_MODE  
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B5;
#else
   call $frmbuffer.get_buffer;
#endif
   I5 = r0;    // Input mic3 channel
   L5 = r1;
   
   r5 = $audio_proc.mic_grouping.ONE_OVER_TWO;
   r10 = r3;
   M1 = MK1;
   r0 = M[I0,0];                                         // preload left_channel
   do four_to_2mic_mapping_loop;
      rMAC = r5 * r0 , r2 = M[I1,MK1] , r1 = M[I4,0];    // Ibuf_D(:,1) = 1/2 * Ibuf_d(:,1) , load mic2 , load right_mic
      rMAC = rMAC + r5 * r2 , r3 = M[I5,MK1];            // Ibuf_D(:,1) = 1/2 * Ibuf_d(:,1) + 1/2 * Ibuf_d(:,2) , load mic3
      r4 = r5 * r3;                                      // Ibuf_D(:,2) = 1/2 * Ibuf_d(:,3)
      r4 = r4 + r5 * r1 , M[I0,M1] = rMAC;               // Ibuf_D(:,2) = 1/2 * Ibuf_d(:,3) + 1/2 * Ibuf_d(:,4) , write left_channel
      r0 = M[I0,0] , M[I4,MK1] = r4;                     // load left_channel , write right_channel
   four_to_2mic_mapping_loop:
   jump $pop_rLink_and_rts;


.ENDMODULE;
