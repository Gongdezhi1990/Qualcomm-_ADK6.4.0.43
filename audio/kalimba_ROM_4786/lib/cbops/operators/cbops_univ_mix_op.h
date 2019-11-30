// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef CBOPS_UNIV_MIX_HEADER_INCLUDED
#define CBOPS_UNIV_MIX_HEADER_INCLUDED

   // Universal mixer parameter structure fields
   // --------------------------------------------------------------------------
   // Channel status information
   .CONST   $cbops.univ_mix_op.INPUT_START_INDEX_FIELD                        0;    // Index of the first input
   .CONST   $cbops.univ_mix_op.OUTPUT_START_INDEX_FIELD                       1;    // Index of the first output
   .CONST   $cbops.univ_mix_op.MIXER_PRIMARY_COPY_STRUCT_ADDR_FIELD           2;    // Pointer back to the primary mixer copy structure
   .CONST   $cbops.univ_mix_op.MIXER_SECONDARY_COPY_STRUCT_ADDR_FIELD         3;    // Pointer back to the secondary mixer copy structure
   .CONST   $cbops.univ_mix_op.COMMON_PARAM_STRUCT_ADDR_FIELD                 4;    // Pointer to a structure of common mixer parameters
   .CONST   $cbops.univ_mix_op.PRIMARY_UPSAMPLER_STRUCT_ADDR_FIELD            5;    // Pointer to the primary channel upsampler structure
   .CONST   $cbops.univ_mix_op.SECONDARY_UPSAMPLER_STRUCT_ADDR_FIELD          6;    // Pointer to the secondary channel upsampler structure
   .CONST   $cbops.univ_mix_op.OUTPUT_UPSAMPLER_STRUCT_ADDR_FIELD             7;    // Pointer to the output channel upsampler structure

   .CONST   $cbops.univ_mix_op.STRUC_SIZE                                     8;
   // --------------------------------------------------------------------------

   // --------------------------------------------------------------------------
   // Common information structure fields (this is common to all upsampler instances)

   .CONST   $cbops.univ_mix_op.common.CHANNELS_ACTIVITY_FIELD                 0;    // Channel activity indicators

   .CONST   $cbops.univ_mix_op.common.STRUC_SIZE                              1;
   // --------------------------------------------------------------------------


   // --------------------------------------------------------------------------
   // Upsampler parameter structure fields (left & right channels may be shared)
   // --------------------------------------------------------------------------

   // Input parameter values required by the upsampler
   // (must be initialised prior to calling the upsampler)

   // Gain control
   .CONST   $cbops.univ_mix_op.params.INPUT_GAIN_FACTOR_FIELD                 0;    // Gain factor(fractional multiply)
   .CONST   $cbops.univ_mix_op.params.INPUT_GAIN_SHIFT_FIELD                  1;    // Gain shift (integer n, giving 2^n gain value)

   // Mixer ratio
   .CONST   $cbops.univ_mix_op.params.RAMP_GAIN_WHEN_MIXING_FIELD             2;    // Desired gain when mixing (fractional, 0.0-1.0) for the mixing volume adjustment
   .CONST   $cbops.univ_mix_op.params.TARGET_RAMP_GAIN_ADJUST_FIELD           3;    // Target gain (fractional, 0.0-1.0) for the mixing volume adjustment
   .CONST   $cbops.univ_mix_op.params.NUM_RAMP_SAMPLES_FIELD                  4;    // Number of samples between volume steps in the volume ramp
   .CONST   $cbops.univ_mix_op.params.RAMP_STEP_SHIFT_FIELD                   5;    // Shift (negative integer) used for each volume step adjustment
   .CONST   $cbops.univ_mix_op.params.RAMP_DELTA_FIELD                        6;    // Additional (fractional) value used for each volume step adjustment

   // Resampling
   .CONST   $cbops.univ_mix_op.params.UPSAMPLING_FACTOR_FIELD                 7;    // Integer upsampling factor
   .CONST   $cbops.univ_mix_op.params.INPUT_RATE_FIELD                        8;    // Input sampling rate
   .CONST   $cbops.univ_mix_op.params.INVERSE_INPUT_RATE_FIELD                9;    // 1/(input sampling rate)
   .CONST   $cbops.univ_mix_op.params.OUTPUT_RATE_FIELD                       10;   // Output sampling rate
   .CONST   $cbops.univ_mix_op.params.INTERP_PHASE_STEP_FIELD                 11;   // Phase step = ((Interp input rate / Interp output rate) - 1) >> 3

   // Upsampler FIR set up
   .CONST   $cbops.univ_mix_op.params.RESAMPLE_COEFS_ADDR_FIELD               12;   // FIR coefficient table pointer
   .CONST   $cbops.univ_mix_op.params.RESAMPLE_COEFS_SIZE_FIELD               13;   // Number of coefficients
   .CONST   $cbops.univ_mix_op.params.RESAMPLE_BUFFER_SIZE_FIELD              14;   // Number of data buffer elements

   .CONST $cbops.univ_mix_op.params.STRUC_SIZE                                15;

   // --------------------------------------------------------------------------

   // --------------------------------------------------------------------------
   // Upsampler data structure fields (left & right channels always unique)
   // --------------------------------------------------------------------------

   // Input parameter values required by the upsampler
   // (must be initialised prior to calling the upsampler)

   // Address of the upsampler parameter structure
   .CONST   $cbops.univ_mix_op.data.PARAMETER_ADDR_FIELD                      0;    // Pointer to the upsampler parameter structure

   // Buffer I/O
   .CONST   $cbops.univ_mix_op.data.INPUT_BUFFER_ADDR_FIELD                   1;    // Input buffer pointer
   .CONST   $cbops.univ_mix_op.data.INPUT_BUFFER_LENGTH_FIELD                 2;    // Length of input buffer
   .CONST   $cbops.univ_mix_op.data.OUTPUT_BUFFER_ADDR_FIELD                  3;    // Output buffer pointer
   .CONST   $cbops.univ_mix_op.data.OUTPUT_BUFFER_LENGTH_FIELD                4;    // Length of output buffer

   // Inputs set for the upsampler
   .CONST   $cbops.univ_mix_op.data.INPUT_SAMPLES_REQUESTED_FIELD             5;    // Requested (available) samples to process
   .CONST   $cbops.univ_mix_op.data.OUTPUT_SAMPLES_REQUESTED_FIELD            6;    // Requested number of output samples to generate

   // Outputs set by the upsampler
   .CONST   $cbops.univ_mix_op.data.INPUT_SAMPLES_READ_FIELD                  7;    // Number of input samples read
   .CONST   $cbops.univ_mix_op.data.OUTPUT_SAMPLES_WRITTEN_FIELD              8;    // Number of output samples written

   // Mixer ratio
   .CONST   $cbops.univ_mix_op.data.RAMP_ACTIVE_FIELD                         9;    // Flag to indicate volume ramping in progress
   .CONST   $cbops.univ_mix_op.data.CURRENT_RAMP_GAIN_ADJUST_FIELD            10;   // Current gain (fractional) for the mixing volume adjustment
   .CONST   $cbops.univ_mix_op.data.CURRENT_RAMP_SAMPLE_COUNT_FIELD           11;   // Sample count between volume steps in the volume ramp
   .CONST   $cbops.univ_mix_op.data.RAMP_CALLBACK_FIELD                       12;   // Callback function pointer - if not NULL this is called on ramp completion

   // Upsampler FIR set up
   .CONST   $cbops.univ_mix_op.data.RESAMPLE_BUFFER_ADDR_FIELD                13;   // FIR data buffer pointer

   // Misc upsampler internal data storage (set by the upsampler)
   .CONST   $cbops.univ_mix_op.data.INTERP_CURRENT_PHASE_FIELD                14;   // Internally calculated current phase value
   .CONST   $cbops.univ_mix_op.data.INTERP_LAST_VAL_FIELD                     15;   // Store for last FIR output value
   .CONST   $cbops.univ_mix_op.data.LOCATION_IN_LOOP_FIELD                    16;   // Internally calculated loop position state

   .CONST $cbops.univ_mix_op.data.STRUC_SIZE                                  17;

   // --------------------------------------------------------------------------

   // --------------------------------------------------------------------------
   // General constant values (non structure related)
   // --------------------------------------------------------------------------

   // Masks for the channel activity indicators
   .CONST   $cbops.univ_mix_op.common.NO_CHANNELS_ACTIVE                      (0x000000);
   .CONST   $cbops.univ_mix_op.common.PRIMARY_CHANNEL_ACTIVE                  (0x000001);
   .CONST   $cbops.univ_mix_op.common.SECONDARY_CHANNEL_ACTIVE                (0x000002);
   .CONST   $cbops.univ_mix_op.common.PRIMARY_AND_SECONDARY_CHANNEL_ACTIVE    (0x000003);

   .CONST   $cbops.univ_mix_op.common.DONT_MIX_PRIMARY_AND_SECONDARY_OUTPUTS  (0x000000);
   .CONST   $cbops.univ_mix_op.common.MIX_PRIMARY_AND_SECONDARY_OUTPUTS       (0x000001);

   .CONST   $cbops.univ_mix_op.UNITY_PHASE                                    0.125;
   .CONST   $cbops.univ_mix_op.UNITY_PHASE_SHIFT_NORMALIZE                    3;
   .CONST   $cbops.univ_mix_op.PHASE_FRACTIONAL_PART_MASK                     (0x0fffff);

#endif // CBOPS_UNIV_MIX_HEADER_INCLUDED
