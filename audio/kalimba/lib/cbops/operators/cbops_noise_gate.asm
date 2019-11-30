// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Noise gate operator
//
// DESCRIPTION:
//    Operator that provides a noise gate to improve the ADC performance
// in silence.
//
// @verbatim
/*
//  Signal: \   /\                       /|   /\     /-\
//           | /  -~~~~~~~~~~~~~~~~~~~~~/ |  |  \  -/   -/
//           |/                            \/    \/
//               |
//  monostable stops    monostable's output
//  triggering here     goes low here
//                       |
//
//        ---------------\                    ------------
//  Gain:   Full gain     \                 /   Full gain
//                         \   No gain    /
//                          \___________/
//
//                      decay           attack
//
//
//  Output: \   /\                            /\     /-\
//           | /  -~~~~~~~---------------/|  |  \  -/   -/
//           |/                            \/    \/
//                         noise has been
//                         reduced here
*/
// @endverbatim
//
// Set the trigger level at which the amplitude is classed as at
// noise like levels eg. -54dBFS (to remove ADC noise)
// @verbatim
// sample_amplitude = power(10, (-54/20) )
//                  = 0.001995
//
// noise_gate_monostable_trigger_level = 0.001995;
// @endverbatim
//
// Set the number of samples of amplitude below the trigger level before
// activating the noise gate.
// @verbatim
// noise_gate_monostable_count_init = 2000
// @endverbatim
//
// Step size per sample to decrease the gain by during the decay portion
// @verbatim
// eg. to decay over 25000 samples = 2^23 * (1/25000) = 335
// noise_gate_decay_step = 335;
// @endverbatim
//
// Step size per sample to increase the gain by during the attack portion
// @verbatim
// eg. to attack over 2500 samples = 2^23 * (1/2500) = 3355
// noise_gate_attack_step = 3355
// @endverbatim
//
// When using the operator the following data structure is used:
//    - $cbops.noise_gate.INPUT_START_INDEX_FIELD = The index of the input
//       buffer
//    - $cbops.noise_gate.OUTPUT_START_INDEX_FIELD = The index of the output
//       buffer
//    - $cbops.noise_gate.MONOSTABLE_COUNT_FIELD = leave initialised to zero
//    - $cbops.noise_gate.DECAYATTACK_COUNT_FIELD = leave initialised to zero
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.noise_gate;
   .DATASEGMENT DM;

   // settings that seem to work well:
   .VAR/DM1  $cbops.noise_gate.monostable_trigger_level = 0.001995;
   .VAR/DM1  $cbops.noise_gate.monostable_count_init    = 5000;
   .VAR/DM1  $cbops.noise_gate.decay_step               = 200;
   .VAR/DM1  $cbops.noise_gate.attack_step              = 30000;

   // ** function vector **
   .VAR $cbops.noise_gate[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.noise_gate.reset,             // reset function
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.noise_gate.main;              // main function

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $cbops.noise_gate.reset
//
// DESCRIPTION:
//    The reset function for the noise gate operator
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0
//
// *****************************************************************************

.MODULE $M.cbops.noise_gate.reset;
   .CODESEGMENT CBOPS_NOISE_GATE_RESET_PM;
   .DATASEGMENT DM;

   // ** reset function **
   $cbops.noise_gate.reset:
   // reset various state information
   r0 = M[$cbops.noise_gate.monostable_count_init];
   M[r8 + $cbops.noise_gate.MONOSTABLE_COUNT_FIELD] = r0;
   r0 = 1.0;
   M[r8 + $cbops.noise_gate.DECAYATTACK_COUNT_FIELD] = r0;

   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.noise_gate.main
//
// DESCRIPTION:
//    Operator that provides a noise gate to improve the ADC performance
//    in silence
//
// INPUTS:
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator structure
//    - r10 = the number of samples to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r3, r10, rMAC, I0, I4, L0, L4
//
// *****************************************************************************

.MODULE $M.cbops.noise_gate.main;
   .CODESEGMENT CBOPS_NOISE_GATE_MAIN_PM;
   .DATASEGMENT DM;

   // ** main routine **
   $cbops.noise_gate.main:
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_noise_gate[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      push rLink;
      r0 = &$cbops.profile_noise_gate;
      call $profiler.start;
   #endif

   // get the offset to the read buffer to use
   r0 = M[r8 + $cbops.noise_gate.INPUT_START_INDEX_FIELD];

   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;

   // get the offset to the write buffer to use
   r0 = M[r8 + $cbops.noise_gate.OUTPUT_START_INDEX_FIELD];

   // get the output buffer write address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the output buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;

   // read in state fields
   r1 = M[r8 + $cbops.noise_gate.MONOSTABLE_COUNT_FIELD];
   r2 = M[r8 + $cbops.noise_gate.DECAYATTACK_COUNT_FIELD];

   r3 = M[$cbops.noise_gate.monostable_count_init];

   do loop;
      rMAC = M[I0,1];
      r0 = rMAC;
      if NEG r0 = - r0;
      Null = r0 - M[$cbops.noise_gate.monostable_trigger_level];
      // if signal level high enough reset monostable
      if POS r1 = r3;
      r1 = r1 - 1;
      if NEG r1 = 0;

      // implement attack and decay slopes
      if Z jump noise_gate_decay;
      noise_gate_attack:
         r0 = r2 + M[$cbops.noise_gate.attack_step];
         if NEG jump noise_gate_off;
         r2 = r0;
         jump noise_gate_active;
      noise_gate_decay:
         r2 = r2 - M[$cbops.noise_gate.decay_step];
         if NEG r2 = 0;
      noise_gate_active:
         rMAC = rMAC * r2 (frac);
      noise_gate_off:
      M[I4,1] = rMAC;
   loop:

   // save back state fields
   M[r8 + $cbops.noise_gate.MONOSTABLE_COUNT_FIELD] = r1;
   M[r8 + $cbops.noise_gate.DECAYATTACK_COUNT_FIELD] = r2;

   // zero the registers
   L0 = 0;
   L4 = 0;

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_noise_gate;
      call $profiler.stop;
      pop rLink;
   #endif
   rts;

.ENDMODULE;
