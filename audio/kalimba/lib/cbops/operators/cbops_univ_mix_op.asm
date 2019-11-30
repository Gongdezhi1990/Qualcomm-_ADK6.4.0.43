// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Universal mixer operator
//
// DESCRIPTION:
//    This operator allows primary and secondary channels to be scaled,
//    upsampled or downsampled, and then mixed. This currently assumes
//    processing of stereo or mono data streams (but NOT a mixture of the two!).
//
//    The primary and secondary channel can be operated alone or both together.
//    The gain/attenuation of each channel can be set independently. Each
//    channel can be upsampled (using an integer upsampling factor) followed by
//    linear interpolation. Example FIR filters are defined to support integer
//    upsampling by factors of 2, 4, and 6. The interpolation stage allows the
//    operator to also deliver output sampling rates which are not a multiple
//    of the input sampling rate.
//
//    Downsampling can be achieved by choosing an appropriate FIR response and
//    using a phaseStep value to select the output samples (or those used
//    to interpolate the output sample. When performing downsampling by an
//    integer factor FIR responses should not be scaled.
//
//    The operator is called from cbops.copy. This provides either the
//    primary or the secondary input channel. When both channels are active
//    the secondary channel is handled internally to the operator. This approach
//    is necessary because the cbops.copy is not designed to handle multiple input
//    sources at different sampling rates.
//
//    The fundamental volume of each channel is controlled by two values (a
//    fractional gain factor and a shift value). These allow flexible control
//    of the channel gains for gain>1 and gain<1 whilst maintaining the
//    dynamic range. In addition, there is a volume ramping function which
//    ramps the volume on a channel to a pre-determined level when the target gain
//    is changed. This is intended to allow the gain of the channels to be
//    varied gracefully as mixing is enabled and disabled (i.e. gain is
//    is reduced when mixing to prevent overflows).
//
//    Overview of the way the mixer handles the I/O within the cbops framework:
//    -------------------------------------------------------------------------
//
//    The primary channel is always handled using the standard cbops processing.
//    The secondary channel when used in conjunction with the primary (i.e. when mixing)
//    is handled internally to the operator but this is done in a way which mimics the
//    standard cbops.copy operation. If the secondary is used on its own then it uses
//    the standard cbops processing. The standard cbops handling of primary and secondary
//    can not be used in general because of the differing amounts of input data
//    consumed due to the essentially arbitrary sampling rates.
//    The way the univ_mixer operator is controlled in the framework is basically governed
//    by the available samples in each of the input buffers and the space in the
//    output buffer. Crucially, the system is configured such that the primary and
//    secondary channel outputs have the same rate and so can be easily mixed into
//    a single output buffer. Mono/stereo operation is very similar with stereo
//    being essentially two instances of a mono channel(in the same cbops chain).
//
//    Procedure used to determine how much data is processed on each call to cbops.copy:
//    ----------------------------------------------------------------------------------
//
//    1/  Determine the number of available i/p samples for prim and sec channels
//    2/  Determine the output buffer space in samples (only one buffer for both channels here)
//    3/  Calculate the number of o/p samples that would be generated for prim and sec
//        based on 1/ and the channel sampling rate ratio (reduce by a small delta to overcome
//        precision effects)
//    4/  Take the smaller of the prim and sec o/p sample count
//    5/  Limit the o/p sample count so it doesn?t exceed the available o/p buffer space from 2/
//    6/  Process the data from prim until the number of o/p samples from 5/ has been produced
//    7/  Process the data from sec until the number of o/p samples from 5/ has been produced
//        (these are mixed in-place in the output buffer)
//    8/  Set the number of input samples consumed by the prim channel
//    9/  Set the number of input samples consumed by the sec channel
//    10/ Set the number of output samples produced
//
//    These calculations are distributed amongst the cbops.copy,
//    cbops.univ_mix_op.amount_to_use, and the main operator process function.
//
//    The amount of input data the main cbops operator function processes from
//    each channel is determined only after the generation of a specified number
//    of output samples and purely by the amount consumed from that channel.
//    Under the current scheme of things this means that it is not possible to add a
//    cbops.operator into the chain before the mixer since the amount of data it must
//    process is not known before the mixer processing has completed. However, the
//    amount of data that the mixer will produce is known prior to the mixer processing.
//    It would therefore be possible to add a cbops.operator following the univ_mixer operator
//    in the chain but it would need to employ a special "amount_to_use" function to
//    determine how much data was output from the mixer operator.
//
//    ---------------------------------------------------------------------------
//
//    When using the operator a structure with the following data elements is used:
//
//    $cbops.univ_mix_op.INPUT_START_INDEX_FIELD                   = Index of the first input buffer
//    $cbops.univ_mix_op.OUTPUT_START_INDEX_FIELD                  = Index of the first output buffer
//    $cbops.univ_mix_op.MIXER_PRIMARY_COPY_STRUCT_ADDR_FIELD      = Pointer back to the primary mixer copy structure
//    $cbops.univ_mix_op.MIXER_SECONDARY_COPY_STRUCT_ADDR_FIELD    = Pointer back to the secondary mixer copy structure
//    $cbops.univ_mix_op.COMMON_PARAM_STRUCT_ADDR_FIELD            = Pointer to a structure of common mixer parameters
//    $cbops.univ_mix_op.PRIMARY_UPSAMPLER_STRUCT_ADDR_FIELD       = Pointer to the primary channel upsampler structure
//    $cbops.univ_mix_op.SECONDARY_UPSAMPLER_STRUCT_ADDR_FIELD     = Pointer to the secondary channel upsampler structure
//    $cbops.univ_mix_op.OUTPUT_UPSAMPLER_STRUCT_ADDR_FIELD        = Pointer to the output channel upsampler structure
//
//    ---------------------------------------------------------------------------
//    The gain/upsampling/interpolation for each channel is defined by a
//    structure with the following elements:
//
//    --------------------------------------------------------------------------
//    Upsampler parameter structure fields (left & right channels may be shared)
//    --------------------------------------------------------------------------

//    $cbops.univ_mix_op.params.INPUT_GAIN_FACTOR_FIELD        = Gain factor(fractional multiply)
//    $cbops.univ_mix_op.params.INPUT_GAIN_SHIFT_FIELD         = Gain shift (integer n, giving 2^n gain value)
//
//    $cbops.univ_mix_op.params.RAMP_GAIN_WHEN_MIXING_FIELD    = Desired gain when mixing (fractional, 0.0-1.0) for the mixing volume adjustment
//    $cbops.univ_mix_op.params.TARGET_RAMP_GAIN_ADJUST_FIELD  = Target gain (fractional, 0.0-1.0) for the mixing volume adjustment
//    $cbops.univ_mix_op.params.NUM_RAMP_SAMPLES_FIELD         = Number of input samples between volume steps in the volume ramp
//    $cbops.univ_mix_op.params.RAMP_STEP_SHIFT_FIELD          = Shift (negative integer) used for each volume step adjustment
//    $cbops.univ_mix_op.params.RAMP_DELTA_FIELD               = Additional (fractional) value used for each volume step adjustment
//
//    $cbops.univ_mix_op.params.UPSAMPLING_FACTOR_FIELD        = Integer upsampling factor
//    $cbops.univ_mix_op.params.INPUT_RATE_FIELD               = Input sampling rate
//    $cbops.univ_mix_op.params.INVERSE_INPUT_RATE_FIELD       = 1/(input sampling rate)
//    $cbops.univ_mix_op.params.OUTPUT_RATE_FIELD              = Output sampling rate
//    $cbops.univ_mix_op.params.INTERP_PHASE_STEP_FIELD        = Phase step = ((Interp input rate / Interp output rate) - 1) >> 3
//
//    $cbops.univ_mix_op.params.RESAMPLE_COEFS_ADDR_FIELD      = FIR coefficient table pointer
//    $cbops.univ_mix_op.params.RESAMPLE_COEFS_SIZE_FIELD      = Number of coefficients
//    $cbops.univ_mix_op.params.RESAMPLE_BUFFER_SIZE_FIELD     = Number of data buffer elements
//    --------------------------------------------------------------------------

//    --------------------------------------------------------------------------
//    Upsampler data structure fields (left & right channels always unique)
//    --------------------------------------------------------------------------

//    $cbops.univ_mix_op.data.PARAMETER_ADDR_FIELD             = Pointer to the upsampler parameter structure (shown above)
//
//    $cbops.univ_mix_op.data.INPUT_BUFFER_ADDR_FIELD          = Input buffer pointer
//    $cbops.univ_mix_op.data.INPUT_BUFFER_LENGTH_FIELD        = Length of input buffer
//    $cbops.univ_mix_op.data.OUTPUT_BUFFER_ADDR_FIELD         = Output buffer pointer
//    $cbops.univ_mix_op.data.OUTPUT_BUFFER_LENGTH_FIELD       = Length of output buffer
//
//    $cbops.univ_mix_op.data.INPUT_SAMPLES_REQUESTED_FIELD    = Requested (available) samples to process
//    $cbops.univ_mix_op.data.OUTPUT_SAMPLES_REQUESTED_FIELD   = Requested number of output samples to generate
//
//    $cbops.univ_mix_op.data.INPUT_SAMPLES_READ_FIELD         = Number of input samples read
//    $cbops.univ_mix_op.data.OUTPUT_SAMPLES_WRITTEN_FIELD     = Number of output samples written

//    $cbops.univ_mix_op.data.CURRENT_RAMP_GAIN_ADJUST_FIELD   = Current gain (fractional) for the mixing volume adjustment
//    $cbops.univ_mix_op.data.CURRENT_RAMP_SAMPLE_COUNT_FIELD  = Input sample count between volume steps in the volume ramp
//    $cbops.univ_mix_op.data.RAMP_CALLBACK_FIELD              = Callback function pointer - if not NULL this is called on ramp completion
//
//    $cbops.univ_mix_op.data.RESAMPLE_BUFFER_ADDR_FIELD       = FIR data buffer pointer
//
//    $cbops.univ_mix_op.data.INTERP_CURRENT_PHASE_FIELD       = Internally calculated current phase value
//    $cbops.univ_mix_op.data.INTERP_LAST_VAL_FIELD            = Store for last FIR output value
//    $cbops.univ_mix_op.data.LOCATION_IN_LOOP_FIELD           = Internally calculated loop position state
//
//    --------------------------------------------------------------------------
//
// RESTRICTIONS:
//    The operator must NOT be used to output to a port (this is because the
//    mixing function mixes in the output buffer using a read/mix/write
//    operation).
//
//    Stereo or mono data inputs must be used (not a mixture).
//
//    The operator should NOT be chained with other operators using the
//    same copy struct. This is because the number of input samples
//    to be consumed is not actually calculated until the operator main
//    is executed.
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"
//#include "cbops_univ_mix_op.h"

.MODULE $M.cbops.univ_mix_op;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.univ_mix_op[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.univ_mix_op.reset,             // reset function
      &$cbops.univ_mix_op.amount_to_use,     // amount to use function
      &$cbops.univ_mix_op.main;              // main function

   // Arrays of addresses and sizes for the secondary input and output buffers
   .VAR $cbops.univ_mix_op.secondary_buffer_sizes[$cbops.MAX_NUM_CHANNELS];
   .VAR $cbops.univ_mix_op.secondary_buffer_addresses[$cbops.MAX_NUM_CHANNELS];
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.univ_mix_op.reset
//
// DESCRIPTION:
//    Reset routine for the universal mixer operator
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume all
//
// *****************************************************************************
.MODULE $M.cbops.univ_mix_op.reset;
   .CODESEGMENT CBOPS_UNIV_MIX_OP_RESET_PM;
   .DATASEGMENT DM;

   // ** reset function **
   $cbops.univ_mix_op.reset:

   // Push rLink onto stack
   push rLink;

   // Zero the location in the loop counter
   M[r8 + $cbops.univ_mix_op.data.LOCATION_IN_LOOP_FIELD] = Null;

   // Zero the Phase value used for interpolation and downsampling
   M[r8 + $cbops.univ_mix_op.data.INTERP_CURRENT_PHASE_FIELD] = Null;

   // Zero the sample counts set by the upsampler
   M[r8 + $cbops.univ_mix_op.data.INPUT_SAMPLES_READ_FIELD] = Null;
   M[r8 + $cbops.univ_mix_op.data.OUTPUT_SAMPLES_WRITTEN_FIELD] = Null;

   // Zero the previous FIR output value
   M[r8 + $cbops.univ_mix_op.data.INTERP_LAST_VAL_FIELD] = Null;

   // Pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.univ_mix_op.amount_to_use
//
// DESCRIPTION:
//    Amount to use function for the universal mixer operator. This allows
//    the number of input samples to process to be calculated. This
//    takes into account the previously calculated number of samples
//    to process, the number of input samples in the buffer, the upsampling
//    factor and the space available in the output buffer.
//    over
//
// INPUTS:
//    - r5 = the minimum of the number of input samples available and the
//      amount of output space available
//    - r6 = the number of input samples available
//    - r7 = the amount of output space available
//    - r8 = pointer to the parameter area of the operator structure
//
// OUTPUTS:
//    - r5 = number of input samples to use
//
// TRASHED REGISTERS:
//    Assume all, except r5, r6, r7, r8
//
// *****************************************************************************
.MODULE $M.cbops.univ_mix_op.amount_to_use;
   .CODESEGMENT CBOPS_UNIV_MIX_OP_AMOUNT_TO_USE_PM;
   .DATASEGMENT DM;

   // ** Amount to use function **
   $cbops.univ_mix_op.amount_to_use:

   //   r5 = amount_to_use
   //   r6 = amount_input_data
   //   r7 = amount_output_space
   //   r8 = pointer to the parameter area of the operator structure

   // Push rLink onto stack
   push rLink;

   push r6;

   // Large value
   r4 = 0x7fffff;

   // Point at the common structure
   r9 = M[r8 + $cbops.univ_mix_op.COMMON_PARAM_STRUCT_ADDR_FIELD];

   // Get the channels activity status
   r9 = M[r9 + $cbops.univ_mix_op.common.CHANNELS_ACTIVITY_FIELD];

   // Are both primary and secondary channel inactive?
   if Z r4 = 0;               // Yes, zero the number of requested output samples
   if Z r5 = 0;               // Yes, zero the number of input samples to use

   // Is the primary channel active?
   null = r9 AND $cbops.univ_mix_op.common.PRIMARY_CHANNEL_ACTIVE;
   if Z jump skip_primary;    // No - jump

      // Point to the primary channel upsampler structure
      r1 = M[r8 + $cbops.univ_mix_op.PRIMARY_UPSAMPLER_STRUCT_ADDR_FIELD];
      call $cbops.univ_mix_op.calc_output_samples_helper;

      r4 = r1;                // Save number of primary output samples

   skip_primary:

   // Is the secondary channel active?
   null = r9 AND $cbops.univ_mix_op.common.SECONDARY_CHANNEL_ACTIVE;
   if Z jump skip_secondary;  // No - jump

      // Is the secondary channel ONLY active?
      null = r9 - $cbops.univ_mix_op.common.SECONDARY_CHANNEL_ACTIVE;

      // If no - primary channel is also active so set up the input for the secondary channel
      // (sets r6 = number of input samples from secondary)
      if NZ call $cbops.univ_mix_op.secondary_setup_helper;

      // Pointer to the secondary channel upsampler structure
      r1 = M[r8 + $cbops.univ_mix_op.SECONDARY_UPSAMPLER_STRUCT_ADDR_FIELD];
      call $cbops.univ_mix_op.calc_output_samples_helper;

      // Find the smaller of primary and secondary output samples (put in r4)
      null = r1 - r4;
      if NEG r4 = r1;

   skip_secondary:

   // Does the number of calculated output samples fit the output buffer space?
   null = r7 - r4;
   if NEG r4 = r7;            // No - limit to the space available

   // Set the number of primary output samples to generate (only update if enabled)
   null = r9 AND $cbops.univ_mix_op.common.PRIMARY_CHANNEL_ACTIVE;
   if Z jump skip_prim_upd;
   
       r0 = M[r8 + $cbops.univ_mix_op.PRIMARY_UPSAMPLER_STRUCT_ADDR_FIELD];
       M[r0 + $cbops.univ_mix_op.data.OUTPUT_SAMPLES_REQUESTED_FIELD] = r4;
       
   skip_prim_upd:
   
   // Set the number of secondary output samples to generate (only update if enabled)
   null = r9 AND $cbops.univ_mix_op.common.SECONDARY_CHANNEL_ACTIVE;
   if Z jump skip_sec_upd;
   
       r0 = M[r8 + $cbops.univ_mix_op.SECONDARY_UPSAMPLER_STRUCT_ADDR_FIELD];
       M[r0 + $cbops.univ_mix_op.data.OUTPUT_SAMPLES_REQUESTED_FIELD] = r4;
       
   skip_sec_upd:
   
   // Don't change r6->r8, r5 set to zero if prim & sec are inactive

   pop r6;

   // Pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.univ_mix_op.calc_output_samples_helper
//
// DESCRIPTION:
//    Helper function for amount_to_use
//
// INPUTS:
//    - r1 = pointer to the upsampler structure to use
//    - r5 = the minimum of the number of input samples available and the
//      amount of output space available
//    - r6 = the number of input samples available
//    - r7 = the amount of output space available
//    - r8 = pointer to the parameter area of the operator structure
//
// OUTPUTS:
//    - r1 = number of output samples
//
// TRASHED REGISTERS:
//    r0, r1
//
// *****************************************************************************
.MODULE $M.cbops.univ_mix_op.calc_output_samples_helper;
   .CODESEGMENT CBOPS_UNIV_MIX_OP_CALC_OUTPUT_SAMPLES_HELPER_PM;
   .DATASEGMENT DM;

   $cbops.univ_mix_op.calc_output_samples_helper:

   // Calculate the number of output samples based on the number of
   // available input samples and the input and output sampling rates.
   r1 = M[r1 + $cbops.univ_mix_op.data.PARAMETER_ADDR_FIELD];
   r0 = M[r1 + $cbops.univ_mix_op.params.OUTPUT_RATE_FIELD];
   r1 = M[r1 + $cbops.univ_mix_op.params.INVERSE_INPUT_RATE_FIELD];
   r1 = r1 ASHIFT 10;       // Scale to maintain precision
   r1 = r0 * r1 (frac);     //
   r1 = r1 * r6 (int);      // Calculate output samples generated = input samples available * fout / fin
   r1 = r1 ASHIFT -10;      // Reverse scaling to obtain result

   // Overcome precision effects (must ensure there are enough input samples!)
   r1 = r1 - 4;
   if NEG r1 = 0;

   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.univ_mix_op.secondary_setup_helper
//
// DESCRIPTION:
//    Helper function for amount_to_use to set up the secondary channel
//
// INPUTS:
//    - M1 = 1
// OUTPUTS:
//    - r6 - minimum number of input samples available (in all input buffers)
//
// TRASHED REGISTERS:
//    r0, r1, r2, r3, r6, r10, I0, I1, I4
//
// *****************************************************************************
.MODULE $M.cbops.univ_mix_op.secondary_setup_helper;
   .CODESEGMENT CBOPS_UNIV_MIX_OP_SECONDARY_SETUP_HELPER_PM;
   .DATASEGMENT DM;

   $cbops.univ_mix_op.secondary_setup_helper:

   // Push rLink onto stack
   push rLink;

   // Primary channel is active so set up the internally handled
   // secondary channel input. The output buffer is common so the
   // available space has already been found (in cbops.copy).
   // This does a similar calculation to that done in cbops.copy
   // but for the secondary channel.

   // Point at the channel copy structure to obtain input/output details
   r6 = M[r8 + $cbops.univ_mix_op.MIXER_SECONDARY_COPY_STRUCT_ADDR_FIELD];

   I0 = r6 + $cbops.NUM_INPUTS_FIELD;

   // Load the address of the array to store the secondary channel input and output addresses
   I4 = &$cbops.univ_mix_op.secondary_buffer_addresses;

   // Load the address of the array to store the secondary channel input and output sizes
   I1 = &$cbops.univ_mix_op.secondary_buffer_sizes;

   // Set r6 to a large value for minimum calculation
   r6 = 0x7fffff;

   r0 = M[I0, M1];                  // Read the number of inputs...
   r10 = r0,                        // ...into loop counter
    r0 = M[I0, M1];                 // First input buffer structure address

   // Work through the input buffers
   do input_buffer_loop;
      r3 = r0;                      // Save address (since call overwrites it)

      // Get the number of samples in the input buffer
      call $cbuffer.calc_amount_data;
      Null = r0 - r6;
      if NEG r6 = r0;               // r6 = min(current_amount_in, new_amount_in)

      r0 = r3;                      // Reload the input structure address

      // Get the address and size of the input buffer
      call $cbuffer.get_read_address_and_size;

      M[I4, 1] = r0,                // Store the read address
       r0 = M[I0, 1];               // Load the next input structure address

      M[I1, 1] = r1;                // Store the buffer size

   input_buffer_loop:

   // Get the write addresses and sizes of the secondary channel output buffers
   r10 = r0,                        // Last read above got the number of outputs
    r0 = M[I0, M1];                 // First output buffer structure

   // Work through the output buffers
   do output_buffer_loop;
      // Get the address and size of the output buffer
      call $cbuffer.get_write_address_and_size;

      M[I4, 1] = r0,                // Store the write address
       r0 = M[I0, 1];               // Load the next buffer to check

      M[I1, M1] = r1;               // Store the buffer size
   output_buffer_loop:

   // Pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.univ_mix_op.main
//
// DESCRIPTION:
//    Operator that copies the input sample to the output
//
// INPUTS:
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator parameter structure
//    - r10 = the number of samples to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-2, r4, r9, r10, I0, L0, I4, L4
//
// *****************************************************************************
.MODULE $M.cbops.univ_mix_op.main;
   .CODESEGMENT CBOPS_UNIV_MIX_OP_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.univ_mix_op.main:

   // Push rLink onto stack
   push rLink;

   // start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_univ_mix_op[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      push rLink;
      r0 = &$cbops.profile_univ_mix_op;
      call $profiler.start;
   #endif

   // Point at the common structure
   r9 = M[r8 + $cbops.univ_mix_op.COMMON_PARAM_STRUCT_ADDR_FIELD];
   r0 = M[r9 + $cbops.univ_mix_op.common.CHANNELS_ACTIVITY_FIELD];

   // Is the primary channel active?
   null = r0 AND $cbops.univ_mix_op.common.PRIMARY_CHANNEL_ACTIVE;
   if Z jump skip_primary; // No - jump

      // Don't mix the channels
      r4 = $cbops.univ_mix_op.common.DONT_MIX_PRIMARY_AND_SECONDARY_OUTPUTS;

      // Get the offset to the primary sampler structure
      r9 = M[r8 + $cbops.univ_mix_op.PRIMARY_UPSAMPLER_STRUCT_ADDR_FIELD];

      // Do the primary channel upsampling
      call $cbops.univ_mix_op.main_helper;

      // Store the number of samples read from the input buffer
      r0 = M[r9 + $cbops.univ_mix_op.data.INPUT_SAMPLES_READ_FIELD];
      M[$cbops.amount_to_use] = r0;

      // Store the number of samples written to the output buffer
      r0 = M[r9 + $cbops.univ_mix_op.data.OUTPUT_SAMPLES_WRITTEN_FIELD];
      M[$cbops.amount_written] = r0;

      // Point at the common structure
      r9 = M[r8 + $cbops.univ_mix_op.COMMON_PARAM_STRUCT_ADDR_FIELD];
      r0 = M[r9 + $cbops.univ_mix_op.common.CHANNELS_ACTIVITY_FIELD];

      // Is the secondary channel active? (primary channel is also active)
      null = r0 AND $cbops.univ_mix_op.common.SECONDARY_CHANNEL_ACTIVE;
      if Z jump done; // No - jump

         // Mix the channels
         r4 = $cbops.univ_mix_op.common.MIX_PRIMARY_AND_SECONDARY_OUTPUTS;

         // Load the tables of secondary buffer addresses and sizes
         r6 = &$cbops.univ_mix_op.secondary_buffer_addresses;
         r7 = &$cbops.univ_mix_op.secondary_buffer_sizes;

         // Get the offset to the secondary sampler structure
         r9 = M[r8 + $cbops.univ_mix_op.SECONDARY_UPSAMPLER_STRUCT_ADDR_FIELD];

         // Do the secondary channel upsampling
         call $cbops.univ_mix_op.main_helper;

         // Update the secondary channel input cbuffer read pointers
         call $cbops.univ_mix_op.secondary_cbuffer_pointer_update;

      jump done;

   skip_primary:

   // Point at the common structure
   r9 = M[r8 + $cbops.univ_mix_op.COMMON_PARAM_STRUCT_ADDR_FIELD];
   r0 = M[r9 + $cbops.univ_mix_op.common.CHANNELS_ACTIVITY_FIELD];

   // Is the secondary channel active? (primary channel isn't active)
   null = r0 AND $cbops.univ_mix_op.common.SECONDARY_CHANNEL_ACTIVE;
   if Z jump done; // No - jump

      // Don't mix the channels
      r4 = $cbops.univ_mix_op.common.DONT_MIX_PRIMARY_AND_SECONDARY_OUTPUTS;

      // Get the offset to the secondary sampler structure
      r9 = M[r8 + $cbops.univ_mix_op.SECONDARY_UPSAMPLER_STRUCT_ADDR_FIELD];

      // Do the secondary channel upsampling
      call $cbops.univ_mix_op.main_helper;

      // Store the number of samples read from the input buffer
      r0 = M[r9 + $cbops.univ_mix_op.data.INPUT_SAMPLES_READ_FIELD];
      M[$cbops.amount_to_use] = r0;

      // Store the number of samples written to the output buffer
      r0 = M[r9 + $cbops.univ_mix_op.data.OUTPUT_SAMPLES_WRITTEN_FIELD];
      M[$cbops.amount_written] = r0;

   done:

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_univ_mix_op;
      call $profiler.stop;
      pop rLink;
   #endif

   // Pop rLink from stack
   jump $pop_rLink_and_rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.univ_mix_op.main_helper
//
// DESCRIPTION:
//    Main function helper to setup and call the upsampler function
//
// INPUTS:
//    - r4 = flag to determine if output mixing is requested
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator parameter structure
//    - r9 = pointer to the upsampler structure to use
//    - r10 = the number of samples to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume all except r8, r9
//
// *****************************************************************************
.MODULE $M.cbops.univ_mix_op.main_helper;
   .CODESEGMENT CBOPS_UNIV_MIX_OP_MAIN_HELPER_PM;
   .DATASEGMENT DM;

   $cbops.univ_mix_op.main_helper:

   // Push rLink onto stack
   push rLink;

   push r8;
   push r9;

   // INPUT
   // Get the offset to the read buffer to use
   r0 = M[r8 + $cbops.univ_mix_op.INPUT_START_INDEX_FIELD];

   // Get the input buffer read address
   r1 = M[r6 + r0];
   M[r9 + $cbops.univ_mix_op.data.INPUT_BUFFER_ADDR_FIELD] = r1;

   // Get the input buffer length
   r1 = M[r7 + r0];
   M[r9 + $cbops.univ_mix_op.data.INPUT_BUFFER_LENGTH_FIELD] = r1;

   // OUTPUT
   // Get the offset to the write buffer to use
   r0 = M[r8 + $cbops.univ_mix_op.OUTPUT_START_INDEX_FIELD];

   // Get the output buffer write address
   r1 = M[r6 + r0];
   M[r9 + $cbops.univ_mix_op.data.OUTPUT_BUFFER_ADDR_FIELD] = r1;

   #ifdef CBOPS_DEBUG
      // Check if the mix output is a port - this can't be handled
      Null = SIGNDET r1;
      if Z call $error;
   #endif // CBOPS_DEBUG

   // Get the output buffer length
   r1 = M[r7 + r0];
   M[r9 + $cbops.univ_mix_op.data.OUTPUT_BUFFER_LENGTH_FIELD] = r1;

   // Set up the input parameters for the upsampler
   r8 = r9;          // Channel upsampler structure
   r5 = 0x7fffff;    // Set the number of input samples to process (large number)

   // Set the number of output samples
   r10 = M[r8 + $cbops.univ_mix_op.data.OUTPUT_SAMPLES_REQUESTED_FIELD];

   // Call the upsampler
   call $cbops.univ_mix_op.upsampler;

   pop r9;
   pop r8;

   // Pop rLink from stack
   jump $pop_rLink_and_rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.univ_mix_op.secondary_cbuffer_pointer_update
//
// DESCRIPTION:
//    Function to update the secondary cbuffer input pointers after buffer writing.
//    This only needs to be called if both the primary and secondary channels
//    are active. If the secondary channel ONLY is active then it is handled in
//    the conventional way by cbops.copy. Also note that the output cbuffers will
//    be updated by the primary channel (cbops.copy) so these do not need updating.
//
// INPUTS:
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator parameter structure
//    - r9 = pointer to the upsampler structure to use
//    - r10 = the number of samples to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-2, r10, I0, I1, I2, I3, L0, M1, M2
//
// *****************************************************************************
.MODULE $M.cbops.univ_mix_op.secondary_cbuffer_pointer_update;
   .CODESEGMENT CBOPS_UNIV_MIX_OP_SECONDARY_CBUFFER_POINTER_UPDATE_PM;
   .DATASEGMENT DM;

   $cbops.univ_mix_op.secondary_cbuffer_pointer_update:

   // Push rLink onto stack
   push rLink;

   // Point at the channel copy structure to obtain input/output details
   r0 = M[r8 + $cbops.univ_mix_op.MIXER_SECONDARY_COPY_STRUCT_ADDR_FIELD];

   I1 = r0 + $cbops.NUM_INPUTS_FIELD;

   I2 = &$cbops.univ_mix_op.secondary_buffer_addresses;
   I3 = &$cbops.univ_mix_op.secondary_buffer_sizes;

   M1 = 1;                       // Set up the index register increment

   // Get the number of samples read (read_ptr increment)
   r0 = M[r9 + $cbops.univ_mix_op.data.INPUT_SAMPLES_READ_FIELD];
   M2 = r0;

   r0 = M[I1, M1];               // Read the number of inputs

   r10 = r0,                     // Load the loop counter with the number of inputs
    r0 = M[I1, M1];              // Get the first input buffer structure address

   do update_input_pointers_loop;
      // Load the current buffer size (if it is 1 then we 'know' it is a port)
      r2 = M[I3, 1];             // Get size
      Null = r2 - M1,
       r1 = M[I2, M1];           // Get address

      if Z jump input_is_a_port;

         I0 = r1;                // Set I0 to the buffer address
         L0 = r2;                // Set L0 to the buffer size

         // Dummy read to increment index register (it's a cbuffer)
         // M2 contains the number of samples that have been read from the input
         L0 = 0,
          r1 = M[I0, M2];
         r1 = I0;

      input_is_a_port:

      r2 = M2;
      call $cbuffer.set_read_address;

      r0 = M[I1, 1];             // Get the next cbuffer structure

   update_input_pointers_loop:

   // Pop rLink from stack
   jump $pop_rLink_and_rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.univ_mix_op.init_upsampler
//
// DESCRIPTION:
//    Initialize routine for the upsampler. Set up defaults which give unity
//    gain and no upsampling.
//
// INPUTS:
//    - r5 = pointer to the FIR buffer
//    - r6 = number of elements in the buffer
//    - r7 = pointer to the upsampler data structure
//    - r9 = pointer to the upsampler params structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r1, r2, r9
//
// *****************************************************************************
.MODULE $M.cbops.univ_mix_op.init_upsampler;
   .CODESEGMENT CBOPS_UNIV_MIX_OP_INIT_UPSAMPLER_PM;
   .DATASEGMENT DM;

   // ** Initialize function **
   $cbops.univ_mix_op.init_upsampler:

   // Upsampler data (this is unique to each instance of the upsampler)
   // -----------------------------------------------------------------

   // Initialize the param structure pointer
   // Point at the params structure (this may be shared e.g. between left & right)
   M[r7 + $cbops.univ_mix_op.data.PARAMETER_ADDR_FIELD] = r9;

   // Buffer details filled in later
   M[r7 + $cbops.univ_mix_op.data.INPUT_BUFFER_ADDR_FIELD] = Null;
   M[r7 + $cbops.univ_mix_op.data.INPUT_BUFFER_LENGTH_FIELD] = Null;
   M[r7 + $cbops.univ_mix_op.data.OUTPUT_BUFFER_ADDR_FIELD] = Null;
   M[r7 + $cbops.univ_mix_op.data.OUTPUT_BUFFER_LENGTH_FIELD] = Null;

   // Ramping gain adjustment (graceful volume reduction when mixing)
   M[r7 + $cbops.univ_mix_op.data.RAMP_ACTIVE_FIELD] = Null;                  // e.g. 0;     Just reset - calculated internally
   M[r7 + $cbops.univ_mix_op.data.CURRENT_RAMP_GAIN_ADJUST_FIELD] = Null;     // e.g. 0.0;   Start with zero gain
   r1 = 100;
   M[r7 + $cbops.univ_mix_op.data.CURRENT_RAMP_SAMPLE_COUNT_FIELD] = r1;   // e.g. 100;   Default fast volume ramp
   M[r7 + $cbops.univ_mix_op.data.RAMP_CALLBACK_FIELD] = Null;                // e.g. 0;     Null to disable - no callback

   // Upsampler FIR set up
   M[r7 + $cbops.univ_mix_op.data.RESAMPLE_BUFFER_ADDR_FIELD] = r5;

   // Outputs set by the upsampler
   M[r7 + $cbops.univ_mix_op.data.INPUT_SAMPLES_READ_FIELD] = Null;
   M[r7 + $cbops.univ_mix_op.data.OUTPUT_SAMPLES_WRITTEN_FIELD] = Null;

   // Upsampler internal data storage (set by the upsampler)
   M[r7 + $cbops.univ_mix_op.data.INTERP_CURRENT_PHASE_FIELD] = Null;
   M[r7 + $cbops.univ_mix_op.data.INTERP_LAST_VAL_FIELD] = Null;
   M[r7 + $cbops.univ_mix_op.data.LOCATION_IN_LOOP_FIELD] = Null;


   // Upsampler params (this may be shared between instances of the upsampler)
   // -----------------------------------------------------------------

   // Gain factors (volume control)
   r1 = 1.0;
   M[r9 + $cbops.univ_mix_op.params.INPUT_GAIN_FACTOR_FIELD] = r1;
   M[r9 + $cbops.univ_mix_op.params.INPUT_GAIN_SHIFT_FIELD] = Null;

   // Ramping gain adjustment (graceful volume reduction when mixing)
   r1 = 0.5;
   M[r9 + $cbops.univ_mix_op.params.RAMP_GAIN_WHEN_MIXING_FIELD] = r1;     // e.g. 0.5;   Reduce gain when mixing
   M[r9 + $cbops.univ_mix_op.params.TARGET_RAMP_GAIN_ADJUST_FIELD] = r1;   // e.g. 0.0;   Gain ramps to follow this value

   r1 = 100;
   M[r9 + $cbops.univ_mix_op.params.NUM_RAMP_SAMPLES_FIELD] = r1;          // e.g. 8000;  Number of input samples between volume steps

   r1 = -2;
   M[r9 + $cbops.univ_mix_op.params.RAMP_STEP_SHIFT_FIELD] = r1;           // e.g. -6;
   r1 = 0.001;
   M[r9 + $cbops.univ_mix_op.params.RAMP_DELTA_FIELD] = r1;                // e.g. 0.001;

   // Sampling specification
   r2 = 0;              // e.g. 2;
   M[r9 + $cbops.univ_mix_op.params.UPSAMPLING_FACTOR_FIELD] = r2;

   r1 = 44100;          // e.g. 22050;
   M[r9 + $cbops.univ_mix_op.params.INPUT_RATE_FIELD] = r1;
   r1 = 1.0/44100;      //
   M[r9 + $cbops.univ_mix_op.params.INVERSE_INPUT_RATE_FIELD] = r1;

   r1 = 44100;          // e.g. 44100;
   M[r9 + $cbops.univ_mix_op.params.OUTPUT_RATE_FIELD] = r1;

   r1 = 0.0;            // e.g. ((Interp fin / Interp fout) - 1 ) >> 3;
   M[r9 + $cbops.univ_mix_op.params.INTERP_PHASE_STEP_FIELD] = r1;

   // Upsampler FIR set up
   r1 = &$cbops.univ_mix_op.upsampler.resample_filter_coefs_x2_low_quality;
   r2 = 20;
   M[r9 + $cbops.univ_mix_op.params.RESAMPLE_COEFS_ADDR_FIELD] = r1;
   M[r9 + $cbops.univ_mix_op.params.RESAMPLE_COEFS_SIZE_FIELD] = r2;
   M[r9 + $cbops.univ_mix_op.params.RESAMPLE_BUFFER_SIZE_FIELD] = r6;

   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.univ_mix_op.ramp_volume
//
// DESCRIPTION:
//    Volume is ramped to desired value from current value. If the callback
//    function pointer is not NULL, the address is called on completion
//    of the volume ramp.
//
// INPUTS:
//    - r1 = pointer to the current ramp gain
//    - r8 = pointer to an instance of the upsampler data structure
//    - r9 = pointer to an instance of the upsampler params structure
//    - M1 = 1
// OUTPUTS:
//    - r1 = gain (volume) value - fractional value 0.0-1.0
//
// TRASHED REGISTERS:
//    - r0, r2, rMAC
//
// *****************************************************************************
.MODULE $M.cbops.univ_mix_op.ramp_volume;
   .CODESEGMENT CBOPS_UNIV_MIX_OP_RAMP_VOLUME_PM;
   .DATASEGMENT DM;

   // ** Ramp volume function **
   $cbops.univ_mix_op.ramp_volume:

   // Push rLink onto stack
   push rLink;

   push r3;

   // Get the desired volume value
   r2 = M[r9 + $cbops.univ_mix_op.params.TARGET_RAMP_GAIN_ADJUST_FIELD];

   // Load the current sample count value
   r0 = M[r8 + $cbops.univ_mix_op.data.CURRENT_RAMP_SAMPLE_COUNT_FIELD];

   // Decrement the sample counter
   r0 = r0 - M1;

   // Volume step required on this sample?
   if POS jump update_count;

      // Load the step shift value (re-use r0!)
      r0 = M[r9 + $cbops.univ_mix_op.params.RAMP_STEP_SHIFT_FIELD];

      // Calculate the new step amount
      // (the step is larger if the volume is larger!)
      r0 = r1 ASHIFT r0;

      // Load the delta value
      r3 = M[r9 + $cbops.univ_mix_op.params.RAMP_DELTA_FIELD];

      r0 = r0 + r3;

      // Ramping up or down?
      Null = r1 - r2;
      if NEG jump increase_volume;

         r1 = r1 - r0;
         Null = r1 - r2;

         // Limit to the target value
         if NEG r1 = r2;
         jump reset_count;

      increase_volume:

      r1 = r1 + r0;
      Null = r1 - r2;

      // Limit to the target value
      if POS r1 = r2;

   reset_count:

   // Is the ramp complete?
   Null = r1 - r2;
   if NZ jump ramp_not_complete;

      // Load the callback function pointer (call it if not NULL)
      r0 = M[r8 + $cbops.univ_mix_op.data.RAMP_CALLBACK_FIELD];
      if NZ call r0;

      // Flag that the volume is no longer changing
      M[r8 + $cbops.univ_mix_op.data.RAMP_ACTIVE_FIELD] = Null;

   ramp_not_complete:

   // Reset the current input sample count value
   r0 = M[r9 + $cbops.univ_mix_op.params.NUM_RAMP_SAMPLES_FIELD];

   // Store back the current volume value
   M[r8 + $cbops.univ_mix_op.data.CURRENT_RAMP_GAIN_ADJUST_FIELD] = r1;

   update_count:

   // Store back the current input sample count value
   M[r8 + $cbops.univ_mix_op.data.CURRENT_RAMP_SAMPLE_COUNT_FIELD] = r0;

   pop r3;

   // Pop rLink from stack
   jump $pop_rLink_and_rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.univ_mix_op.upsampler
//
// DESCRIPTION:
//    Upsample data - input data is upsampled and filtered to produce output
//    data at an integer multiple of the input rate. The ratios supported are
//    dependent on the defined filter sets (currently these support ratios
//    of 2, 4 and 6). Ratios which are not an integer multiple are also
//    supported using linear interpolation between samples delivered by the
//    upsampling process (e.g. can produce samples at a 44.1kHz sampling rate
//    from samples at a 48kHz sampling rate.
//
// INPUTS:
//    - r4 = flag to determine if output mixing is requested
//    - r5 = number of input samples available
//    - r8 = pointer to an instance of the upsampler data structure
//    - r10 = the number of output samples to deliver
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume everything
//
// NOTES:
//    Some resample filter coefficients are defined as part of the library. Six
//    sets are defined:
// @verbatim
//    Name                                                                    Length
//    $cbops.univ_mix_op.upsampler.resample_filter_coefs_x6_high_quality      60
//    $cbops.univ_mix_op.upsampler.resample_filter_coefs_x4_high_quality      52
//    $cbops.univ_mix_op.upsampler.resample_filter_coefs_x2_high_quality      40
//    $cbops.univ_mix_op.upsampler.resample_filter_coefs_x6_low_quality       48
//    $cbops.univ_mix_op.upsampler.resample_filter_coefs_x4_low_quality       40
//    $cbops.univ_mix_op.upsampler.resample_filter_coefs_x2_low_quality       20
// @endverbatim
//    Each set of filters will only be included if they are referenced in your
//    application code.
//
//    As well as the filter coefficients a circular resample buffer is required.
//    This buffer needs to be declared in the application. The size of this
//    buffer is dependent on the quality of filters used, higher quality
//    requires longer filters, lower quality requires shorter filters.
//
//
// *****************************************************************************
.MODULE $M.cbops.univ_mix_op.upsampler;
   .CODESEGMENT CBOPS_UNIV_MIX_OP_UPSAMPLER_PM;
   .DATASEGMENT DM;

   // ** Upsampling function **
   $cbops.univ_mix_op.upsampler:

   // Push rLink onto stack
   push rLink;

   // Start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_upsample[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_upsample;
      call $profiler.start;
   #endif

   // ------------------------------------------------------------------------------
   // Assume zero samples read and written initially
   M[r8 + $cbops.univ_mix_op.data.INPUT_SAMPLES_READ_FIELD] = Null;
   M[r8 + $cbops.univ_mix_op.data.OUTPUT_SAMPLES_WRITTEN_FIELD] = Null;

   // Set the number of input samples to consume
   M[r8 + $cbops.univ_mix_op.data.INPUT_SAMPLES_REQUESTED_FIELD] = r5;
   if Z jump exit;   // Don't process any data if none requested

   // Set the number of output samples to produce (need r10 for FIR loop)
   M[r8 + $cbops.univ_mix_op.data.OUTPUT_SAMPLES_REQUESTED_FIELD] = r10;
   if Z jump exit;   // Don't process any data if none requested
   r7 = r10;

   // ------------------------------------------------------------------------------
   // Set up the pointer to the upsampler params structure
   r9 = M[r8 + $cbops.univ_mix_op.data.PARAMETER_ADDR_FIELD];

   // ------------------------------------------------------------------------------
   // I/O buffer info. set up
   // ------------------------------------------------------------------------------

   // Get the input buffer address
   r0 = M[r8 + $cbops.univ_mix_op.data.INPUT_BUFFER_ADDR_FIELD];
   I0 = r0;
   // Get the input buffer length
   r0 = M[r8 + $cbops.univ_mix_op.data.INPUT_BUFFER_LENGTH_FIELD];
   L0 = r0;
   // Get the output buffer address
   r0 = M[r8 + $cbops.univ_mix_op.data.OUTPUT_BUFFER_ADDR_FIELD];
   I5 = r0;
   // Get the output buffer length
   r0 = M[r8 + $cbops.univ_mix_op.data.OUTPUT_BUFFER_LENGTH_FIELD];
   L5 = r0;

   // ------------------------------------------------------------------------------
   // Get the desired volume value
   r0 = M[r9 + $cbops.univ_mix_op.params.TARGET_RAMP_GAIN_ADJUST_FIELD];

   // Get the current volume value
   r1 = M[r8 + $cbops.univ_mix_op.data.CURRENT_RAMP_GAIN_ADJUST_FIELD];

   // Is the volume changing?
   r0 = r1 - r0;
   M[r8 + $cbops.univ_mix_op.data.RAMP_ACTIVE_FIELD] = r0;

   // ------------------------------------------------------------------------------

   // Get the current phase
   r3 = M[r8 + $cbops.univ_mix_op.data.INTERP_CURRENT_PHASE_FIELD];

   // Set up M registers
   M0 = 0;
   M1 = 1;
   // Get the upsampling factor into M2
   r0 = M[r9 + $cbops.univ_mix_op.params.UPSAMPLING_FACTOR_FIELD];
   M2 = r0;

   // Save the mixing flag
   M3 = r4;

   // Get the input gain values
   r6 = M[r9 + $cbops.univ_mix_op.params.INPUT_GAIN_FACTOR_FIELD];
   I7 = r9 + $cbops.univ_mix_op.params.INPUT_GAIN_SHIFT_FIELD;

   // Get the location in the loop (UF..1)
   r2 = M[r8 + $cbops.univ_mix_op.data.LOCATION_IN_LOOP_FIELD];
   if Z r2 = M1;

   // ------------------------------------------------------------------------------
   // FIR info. set up
   // ------------------------------------------------------------------------------

   // Get the FIR coeff table address and length
   // Coeff. to use is base coeff + UF - location in loop
   // i.e. &Coeff0 + UF - [UF..1] + 1
   r0 = M[r9 + $cbops.univ_mix_op.params.RESAMPLE_COEFS_ADDR_FIELD];
   I6 = r0;                         // I6 points at the first FIR coeff.
   I4 = r0 - r2;                    // Use location in loop to calculate the first coeff. to process
   I4 = I4 + M2;                    // Offset by the upsample factor
   I4 = I4 + 1;                     // Offset by one for the next input sample
   r0 = M[r9 + $cbops.univ_mix_op.params.RESAMPLE_COEFS_SIZE_FIELD];
   L4 = r0;

   // Get the local FIR buffer address and size
   r0 = M[r8 + $cbops.univ_mix_op.data.RESAMPLE_BUFFER_ADDR_FIELD];
   I1 = r0;
   r0 = M[r9 + $cbops.univ_mix_op.params.RESAMPLE_BUFFER_SIZE_FIELD];
   L1 = r0;

   // Represents a phase of 1.0
   r4 = $cbops.univ_mix_op.UNITY_PHASE;

   // Point at the previous FIR output value (used for interpolation)
   I2 = r8 + $cbops.univ_mix_op.data.INTERP_LAST_VAL_FIELD;

   // Point at the phase step value
   I3 = r9 + $cbops.univ_mix_op.params.INTERP_PHASE_STEP_FIELD;

   // ------------------------------------------------------------------------------
   // Register summary in main loop:
   // I0: Input data buffer pointer,     L0: Input data buffer size
   //
   // I1: FIR data pointer,              L1: FIR data buffer size
   // I2: Previous FIR output value pointer
   // I3: Phase step value pointer,
   // I4: FIR coeff pointer,             L4: FIR table size
   //
   // I5: Output data buffer pointer,    L5: Output data buffer size
   // I6: FIR coeff table base pointer,
   // I7: Gain shift pointer (2^n gain value)
   //
   // M0: 0
   // M1: 1
   // M2: Upsampling factor (UF)
   // M3: Mixing requested flag
   //
   // r2: Loop location count (i.e. UF..1)
   // r3: Interpolation coeff. (phase)
   // r4: Used for unity phase constant
   // r5: Input sample counter (counts down until zero then all done)
   // r6: Input gain factor value
   // r7: Output sample counter (counts down until zero then all done)
   // r8: Pointer to an instance of the upsampler data structure
   // r9: Pointer to an instance of the upsampler params structure

   // ------------------------------------------------------------------------------

   // Main resample loop
   // (note: the same FIR input data is used for sets of UF FIR output samples
   // the loop position in r2 stores this count, UF..1)
   generate_data:

      // Count down the FIR output samples to determine a new set
      r2 = r2 - M1;                 // Update the loop position counter (UF..1)

      // Don't need any new data until UF FIR output samples have been produced
      if GT jump no_new_data;
                                    // Another input sample is needed
         r5 = r5 - M1,              // Decrement the number of input samples counter
          r0 = M[I0, M1],           // Read the new data point
          r1 = M[I7, M0];           // Need the input gain shift into a bank 1 reg
         if NEG jump done_input;    // Exit when no more input samples

         rMAC = r0 * r6 (ss),       // Apply the input gain factor
          r0 = M[I1, -1];           // Point I1 to the oldest data (dummy read)

         rMAC = rMAC ASHIFT r1;     // Apply the input gain shift

         // Get the current volume value
         r1 = M[r8 + $cbops.univ_mix_op.data.CURRENT_RAMP_GAIN_ADJUST_FIELD];

         // Is the volume changing?
         null = M[r8 + $cbops.univ_mix_op.data.RAMP_ACTIVE_FIELD];
         if NZ call $cbops.univ_mix_op.ramp_volume;
                                    // Perform volume ramping (fractional gain returned in r1)

         rMAC = rMAC * r1 (ss);     // Apply volume ramp gain adjustment

         r2 = M2;                   // Reset the "location in loop" value

         // Is filtering and interpolation required?
         // (FIR/interpolation is performed unless the upsampling factor (UF) is zero)
         if Z jump skip_filter_and_interp;     // Jump to output/mix code

         I4 = I6,                   // Point I4 at the first FIR coeff
          M[I1, M0] = rMAC;         // Overwrite the oldest data sample in the FIR buffer

      no_new_data:

      // ------------------------------------------------------------------------------
      // FIR
      // ------------------------------------------------------------------------------
      // This performs the FIR anti-aliasing filtering for the upsampling process.
      // This uses zero fill (not duplicate input samples). The loop steps through the
      // samples (at the low rate) and calculates every UFth tap where UF is the
      // upsampling factor. At the end of the complete FIR MAC, the
      // data/coeff alignment is offset by one sample (at the high rate) by incrementing
      // the coeff. pointer. This is done to process and deliver the next FIR output
      // sample in the set of UF samples (1..UF).

      // Determine if a new FIR o/p sample is required
      // (FIR o/p sample required if phase < 2.0)
      r3 = r3 - r4,              // Reduce phase (by equivalent of 1 FIR o/p sample)
       r0 = M[I4, 1];            // Increment the coeff. index ready for the next FIR computation
      null = r3 - r4;            // Calc. Phase - 2.0,

      // Loop again?
      if POS jump generate_data; // (Jump if phase >= 2.0)

      // Phase != 0?
      r3 = r3 + r4,              // Reverse previous phase change
       r0 = M[I3, 0],            // Get a scaled down phase step value
                                 // r0 = ((Interp. i/p rate / Interp. o/p rate) - 1) >> 3;
       r1 = M[I4, -1];           // Reverse previous coeff. index change (dummy r1 load)
      if NZ jump run_fir;        // Calc. FIR o/p if phase != 0

      // Phase == 0.0 so check if the Phase_step is >= 1.0
      null = r0 - r4;
      if GE jump skip_fir;

      run_fir:

      // FIR loop counter is the number of steps through the FIR buffer - 1
      // (the last tap is done out of the loop!)
      r10 = L1 - M1;

      // Start filter accumulation at zero and set up the first multiply accumulate
      rMAC = 0,
       r0 = M[I1,M1],               // First input data (steps in 1)
       r1 = M[I4,M2];               // First filter coefficient (steps in upsampling factor)

      // FIR loop (this assumes zero fill upsampling!)
      do loop;
         rMAC = rMAC + r0 * r1,     // Do an FIR tap
          r0 = M[I1,M1],            // Increment the data buffer index
          r1 = M[I4,M2];            // Step the filter coeff. index
      loop:

      skip_fir:                     // Skips FIR processing but does coeff. index increment
                                    // (RMAC contains don't-care junk if entering at skip_fir)

      // Do the last multiply accumulate - result in rMAC
      rMAC = rMAC + r0 * r1,        // Last FIR tap
       r1 = M[I2, M0],              // Get the previous FIR output sample, y(n-1) into r1
       r0 = M[I4, M1];              // Increment the coeff. index
                                    // (align for next FIR output sample in set (1..UF))

      // ------------------------------------------------------------------------------
      // Linear interpolation
      // ------------------------------------------------------------------------------

      // Interpolated sample between consecutive FIR output samples?
      // (Sometimes there is no required interpolated output sample between
      // consecutive upsampled samples)
      // Determine if a new interpolated o/p sample is required
      // (interpolated o/p sample required if phase < 1.0)
      r0 = r4 - r3,                    // Calc. 1.0 - Phase
       M[I2, M0] = rMAC;               // Save the new FIR output sample, y(n)
      if LE jump no_data_to_produce;
         // Pre-calculate required value - scale up to get the actual phase value
         r0 = r0 LSHIFT $cbops.univ_mix_op.UNITY_PHASE_SHIFT_NORMALIZE;

         rMAC = rMAC - rMAC * r0 (su); // Perform the interpolation result in rMAC
                                       // yi = y(n) [xi - x(n-1)] - y(n-1) [xi - x(n-1)] + y(n-1)
                                       // where [xi - x(n-1)] is r3 and y(n-1) is r1
         rMAC = rMAC + r1 * r0 (su),   // and [x(n) - x(n-1)] is set to 1 by design
          r0 = M[I3, M0];              // Get a scaled down phase step value.
                                       // r0 = ((Interp. i/p rate / Interp. o/p rate) - 1) >> 3;

         // Update the phase for the next interpolator output sample
         // (phase + delta_phase)
         r3 = r3 + r0;

         skip_filter_and_interp:

         null = M3,                 // Get the mixing requested flag
          r0 = M[I5, M0];           // Read the existing output sample

         if NZ  rMAC = rMAC + r0;   // Mix with new output sample

         r7 = r7 - M1,              // Decrement the number of output samples processed
          M[I5, 1] = rMAC;          // Save the upsampled/interpolated value

         // More audio data to process? - If so, loop again
         if GT jump generate_data;

         // All the audio data has been produced so exit loop
         jump done_output;

      // ------------------------------------------------------------------------------

      no_data_to_produce:

      // Reduce phase (by equivalent of 1 FIR o/p sample)
      r3 = r3 - r4;

   // Loop again
   jump generate_data;

   done_input:

   r5 = r5 + 1;                     // Correct input count

   done_output:

   // Save the location in the loop counter (UF..1)
   M[r8 + $cbops.univ_mix_op.data.LOCATION_IN_LOOP_FIELD] = r2;

   // Save the current phase
   M[r8 + $cbops.univ_mix_op.data.INTERP_CURRENT_PHASE_FIELD] = r3;

   // Store the new FIR data sample buffer position
   r0 = I1;
   M[r8 + $cbops.univ_mix_op.data.RESAMPLE_BUFFER_ADDR_FIELD] = r0;

   // Calculate and store the number of samples read
   r0 = M[r8 + $cbops.univ_mix_op.data.INPUT_SAMPLES_REQUESTED_FIELD];
   r0 = r0 - r5;
   M[r8 + $cbops.univ_mix_op.data.INPUT_SAMPLES_READ_FIELD] = r0;

   // Calculate and store the number of samples written
   r0 = M[r8 + $cbops.univ_mix_op.data.OUTPUT_SAMPLES_REQUESTED_FIELD];
   r0 = r0 - r7;
   M[r8 + $cbops.univ_mix_op.data.OUTPUT_SAMPLES_WRITTEN_FIELD] = r0;

   // Zero the length registers we've changed (for safety)
   L0 = 0;
   L1 = 0;
   L4 = 0;
   L5 = 0;

   exit:

   // Stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_upsample;
      call $profiler.stop;
   #endif

   // Pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

// ------------------------------------------------------------------------------------
// Example FIR filter coefficient sets (scaled to give unity gain when upsampling)
// (note: the number of coeffs must be an integer multiple of the upsampling factor)
// ------------------------------------------------------------------------------------
.MODULE $M.cbops.univ_mix_op.upsampler.resample_filter_coefs_x6_high_quality;
   .DATASEGMENT DM;

   // the number of coefficients is given by (UPSAMPLE_RATE * 10)
   .VAR/DM2CIRC $cbops.univ_mix_op.upsampler.resample_filter_coefs_x6_high_quality[60] =
      0.00299180773312, 0.00337684700128, 0.00316586211810, 0.00030415947874,
      -0.00623724579286, -0.01661202739129, -0.02958281797555, -0.04233850387622,
      -0.05084572239179, -0.05079502430913, -0.03899505537918, -0.01485601867704,
      0.01853516419305, 0.05424207988215, 0.08252433533393, 0.09308786686528,
      0.07812743197666, 0.03540080012978, -0.02959731863039, -0.10320159139092,
      -0.16535326339796, -0.19348620963701, -0.16779287445926, -0.07664463879296,
      0.07925986155071, 0.28525930116415, 0.51429355441750, 0.73156038717990,
      0.90133730557561, 0.99442312037902, 0.99442312037902, 0.90133730557561,
      0.73156038717990, 0.51429355441750, 0.28525930116415, 0.07925986155071,
      -0.07664463879296, -0.16779287445926, -0.19348620963701, -0.16535326339796,
      -0.10320159139092, -0.02959731863039, 0.03540080012978, 0.07812743197666,
      0.09308786686528, 0.08252433533393, 0.05424207988215, 0.01853516419305,
      -0.01485601867704, -0.03899505537918, -0.05079502430913, -0.05084572239179,
      -0.04233850387622, -0.02958281797555, -0.01661202739129, -0.00623724579286,
      0.00030415947874, 0.00316586211810, 0.00337684700128, 0.00299180773312;

.ENDMODULE;


.MODULE $M.cbops.univ_mix_op.upsampler.resample_filter_coefs_x4_high_quality;
   .DATASEGMENT DM;

   // the number of coefficients is given by (UPSAMPLE_RATE * 13)
   .VAR/DM2CIRC $cbops.univ_mix_op.upsampler.resample_filter_coefs_x4_high_quality[52] =
      -0.00463279694390, -0.01168836613735, -0.02023821222233, -0.02512902934066,
      -0.02072453147215, -0.00462749915385, 0.01882378085801, 0.03845470490132,
      0.04111946366883, 0.01994829752752, -0.01872465424376, -0.05516083669076,
      -0.06514624256608, -0.03478983195605, 0.02711258608157, 0.08818118728442,
      0.10686440753220, 0.05697748968643, -0.05056492729890, -0.16293192963136,
      -0.20418780847661, -0.11160754004642, 0.12787790618940, 0.46114415052350,
      0.78374878926448, 0.98182396337413, 0.98182396337413, 0.78374878926448,
      0.46114415052350, 0.12787790618940, -0.11160754004642, -0.20418780847661,
      -0.16293192963136, -0.05056492729890, 0.05697748968643, 0.10686440753220,
      0.08818118728442, 0.02711258608157, -0.03478983195605, -0.06514624256608,
      -0.05516083669076, -0.01872465424376, 0.01994829752752, 0.04111946366883,
      0.03845470490132, 0.01882378085801, -0.00462749915385, -0.02072453147215,
      -0.02512902934066, -0.02023821222233, -0.01168836613735, -0.00463279694390;

.ENDMODULE;


.MODULE $M.cbops.univ_mix_op.upsampler.resample_filter_coefs_x2_high_quality;
   .DATASEGMENT DM;

   // the number of coefficients is given by (UPSAMPLE_RATE * 20)
   .VAR/DM2CIRC $cbops.univ_mix_op.upsampler.resample_filter_coefs_x2_high_quality[40] =
      0.00119361695263, 0.01053084228025, 0.01958687852438, 0.00708533797537,
      -0.01963237053303, -0.01601595485528, 0.02075605149467, 0.02329507802689,
      -0.02495227313125, -0.03174432215625, 0.03317344697143, 0.04364763892829,
      -0.04702590710220, -0.06232881944139, 0.07089222840463, 0.09602452002924,
      -0.12020492726074, -0.17926244647231, 0.29455944466607, 0.90276043729730,
      0.90276043729730, 0.29455944466607, -0.17926244647231, -0.12020492726074,
      0.09602452002924, 0.07089222840463, -0.06232881944139, -0.04702590710220,
      0.04364763892829, 0.03317344697143, -0.03174432215625, -0.02495227313125,
      0.02329507802689, 0.02075605149467, -0.01601595485528, -0.01963237053303,
      0.00708533797537, 0.01958687852438, 0.01053084228025, 0.00119361695263;

.ENDMODULE;


.MODULE $M.cbops.univ_mix_op.upsampler.resample_filter_coefs_x6_low_quality;
   .DATASEGMENT DM;

   // the number of coefficients is given by (UPSAMPLE_RATE * 8)
   .VAR/DM2CIRC $cbops.univ_mix_op.upsampler.resample_filter_coefs_x6_low_quality[48] =
      -0.00381787996341, -0.00608195460019, -0.00804814947664, -0.00690812579632,
      -0.00031080571423, 0.01317413443490, 0.03288397248491, 0.05524289172751,
      0.07379335412654, 0.08040225311993, 0.06760171269731, 0.03157097127531,
      -0.02512025065165, -0.09203984361767, -0.15153745470457, -0.18181928942527,
      -0.16202393552520, -0.07813747461916, 0.07187830215641, 0.27537239819291,
      0.50581482566634, 0.72731465978872, 0.90196123339167, 0.99822151766673,
      0.99822151766673, 0.90196123339167, 0.72731465978872, 0.50581482566634,
      0.27537239819291, 0.07187830215641, -0.07813747461916, -0.16202393552520,
      -0.18181928942527, -0.15153745470457, -0.09203984361767, -0.02512025065165,
      0.03157097127531, 0.06760171269731, 0.08040225311993, 0.07379335412654,
      0.05524289172751, 0.03288397248491, 0.01317413443490, -0.00031080571423,
      -0.00690812579632, -0.00804814947664, -0.00608195460019, -0.00381787996341;

.ENDMODULE;


.MODULE $M.cbops.univ_mix_op.upsampler.resample_filter_coefs_x4_low_quality;
   .DATASEGMENT DM;

   // the number of coefficients is given by (UPSAMPLE_RATE * 10)
   .VAR/DM2CIRC $cbops.univ_mix_op.upsampler.resample_filter_coefs_x4_low_quality[40] =
      0.00287592389989, 0.00368448847849, -0.00010733937364, -0.01229621700576,
      -0.03149023801418, -0.04870764708438, -0.04981133883804, -0.02406728969131,
      0.02518817725267, 0.07557352547658, 0.09308685339212, 0.05171003408261,
      -0.04431688169969, -0.15031335688040, -0.19396723450718, -0.10949424765046,
      0.12287059068745, 0.45479880233039, 0.78122868787959, 0.98344072194748,
      0.98344072194748, 0.78122868787959, 0.45479880233039, 0.12287059068745,
      -0.10949424765046, -0.19396723450718, -0.15031335688040, -0.04431688169969,
      0.05171003408261, 0.09308685339212, 0.07557352547658, 0.02518817725267,
      -0.02406728969131, -0.04981133883804, -0.04870764708438, -0.03149023801418,
      -0.01229621700576, -0.00010733937364, 0.00368448847849, 0.00287592389989;

.ENDMODULE;


.MODULE $M.cbops.univ_mix_op.upsampler.resample_filter_coefs_x2_low_quality;
   .DATASEGMENT DM;

   // the number of coefficients is given by (UPSAMPLE_RATE * 10)
   .VAR/DM2CIRC $cbops.univ_mix_op.upsampler.resample_filter_coefs_x2_low_quality[20] =
      0.00604204021624, -0.00002757676394, -0.03794769360664, -0.04092541679126,
      0.05250303236131, 0.08307866518751, -0.09982977103996, -0.17399511293587,
      0.28006393488967, 0.90726285562238, 0.90726285562238, 0.28006393488967,
      -0.17399511293587, -0.09982977103996, 0.08307866518751, 0.05250303236131,
      -0.04092541679126, -0.03794769360664, -0.00002757676394, 0.00604204021624;

.ENDMODULE;
