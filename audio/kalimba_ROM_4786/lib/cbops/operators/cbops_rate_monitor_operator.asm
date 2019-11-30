// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Data Rate Monitor Operator
//
// DESCRIPTION:
//    This operator tracks the rate mismatch between a specified target and
//    the throughput through the specified test point
//
//    This function calcualates the rate mismatch between two devices.
//    The calculated rate is stored into two fields in the data object:
//       POSITIVE_WARP_VALUE_FIELD and POSITIVE_WARP_VALUE_FIELD.
//    These are in a format suitable for the $cbops.rate_adjustment_and_shift operator.
//
//    These assumptions are made:
//    1) $M.frame_sync.rate_detect is called from the application's audio_copy
//    handler, which runs every 625 microseconds.
//    2) The cbuffer being analyzed holds at least 2 packets of data (i.e. if
//    running USB at 16 kHz, each USB packet is 16 words.  The buffer therefore
//    needs to hold at least 32 words.
//
//    This rate detect function will work best with USB if
//    $rate_detect.COUNTER_THRESHOLD is set to a value that will contain an
//    even number of USB deliveries (they're delivered at a 1ms interval) and an
//    even number of audio_handler interrupts (they fire every 625
//    microseconds).  This means it must be a multiple of 8.  160,000 is a good
//    value because it is a multiple of 8 and it provides a good amount of data
//    (10 seconds if running @ 16kHz) to calculate a warp value.  Note, that
//    since USB is delivered in packets (the packet interval is 1 ms) and since
//    the DSP is polling for new data, each measurement will likely be under
//    or over by a USB packet size (16 samples if running at 16 kHz).  This is
//    why we calculate over a long period of time (~10 seconds) and why we
//    use an averaging filter.
//
// For HW rate monitoring 
//    It is desireable to adjust the ADC/DAC rate to match the processing rate.
//    This operator monitors the ADC or DAC and tracks the samples over time,
//    where the time is derived from the period of the timer task calling this
//    operator. 
//
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"
#include "cbuffer_asm.h"
#include "cbops_rate_monitor_op_asm_defs.h"
#include "portability_macros.h"


// Private Library Exports
.PUBLIC $cbops.rate_monitor_op;
.PUBLIC $cbops.rate_monitor_op.Initialize;

.MODULE $M.cbops.rate_monitor_op;
   .CODESEGMENT   CBOPS_RATE_MONITOR_OPERATOR_PM;
   .DATASEGMENT   DM;

// ** function vector **
   .VAR $cbops.rate_monitor_op[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,      // reset function
      &$cbops.rate_monitor_op.amount_to_use,   // amount to use function
      &$cbops.rate_monitor_op.main;            // main function

   // Rate Averaging constants
   .VAR alpha_warp_table[11] = 1.000000000000000, 0.500000000000000, 0.333333333333333,
                               0.250000000000000, 0.200000000000000, 0.166666666666667,
                               0.142857142857143, 0.125000000000000, 0.111111111111111,
                               0.100000000000000, 0.090909090909091;

// Expose the location of this table to C
.set $_cbops_rate_monitor_table,  $cbops.rate_monitor_op

// *****************************************************************************
// MODULE:
//   $cbops.rate_monitor_op.amount_to_use
//
// DESCRIPTION:
//   Count interupt periods for stall detection
//
// INPUTS:
//    - r4 = pointer to list of buffers
//    - r8 = pointer to cbops object
//
// OUTPUTS:
//    none (r5-r8) preserved
//
// TRASHED REGISTERS:
//    r1,r0
//
// *****************************************************************************
$cbops.rate_monitor_op.amount_to_use:
#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.rate_monitor_op.amount_to_use.PATCH_ID_0, r7)
#endif
     
   r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   NULL = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.MONITOR_MODE_FIELD];
   if NZ jump jp_hw_monitor_amount_to_use;
   
      // Software amount to use
      r2 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_COUNTER_FIELD];
      if NEG jump skip_processing;
      
         r0 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.STALL_FIELD];           
         // Have there been NO_DATA_PERIODS_FOR_STALL missed calls to the main routine
         // due to a stall? If so, perform a reset.             
         Null = r0 - 2;
         if POS jump reset_rate_monitor;
            r0 = r0 + 1;
            M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.STALL_FIELD] = r0;
      skip_processing:
      r2 = r2 + 1;
      M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_COUNTER_FIELD] = r2;
      rts;
      
jp_hw_monitor_amount_to_use:
    // get to the start of the params, rely on zero in/out channels.
    // Note that a multi-channel instance of this op is purely symbolic, all its
    // calculations and actions are based on channel-independent information
    // (i.e. the data amounts that the cbops framework passed on to it).

    // Increment Period Counter.  Wait for Settle Time to complete
    r0 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_COUNTER_FIELD];
    r0 = r0 + 1;
    M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_COUNTER_FIELD]=r0;
    if NEG rts;
    
    // Wait for completion of sample collection 
    r1 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.COLLECT_PERIODS_FIELD];   
    NULL = r0 - r1;
    if NEG rts;

    // Complete rate mismatch
    r0 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.EXPECTED_ACUM_FIELD];   
    // Load number of samples provided by source
    rMAC = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.ACCUMULATOR_FIELD];

    r1 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.LAST_ACCUMULATOR_FIELD];
    if Z r1 = rMAC;
    
    M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.LAST_ACCUMULATOR_FIELD] = rMAC;

    rMAC = rMAC - (-1.0)*r1;
    rMAC = rMAC ASHIFT -1 (56bit);   
    
    r1 = SIGNDET rMAC;
    r2 = r1 - 2;
    rMAC = rMAC ASHIFT r2;

    // Warp = actual_number_of_samples / expected_number_of_samples
    // Frac divide + 1 more shift to make sure it's bigger than numerator
    r0 = r0 ASHIFT r1;
    Div = rMAC / r0;
   
    // Division result 
    r0 = DivResult;

    // Check that data stream is reasonable 
    NULL = r0 - 0.6;       
    if GT jump jp_restart;
    
    NULL = r0 - 0.4;       
    if LT jump jp_restart;

    r0 = r0 ASHIFT (24 - DAWTH);
    
       // Measured rate is expected/accumulate  (0.5 +/- 0.001)
       M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.MEASURED_RATE_FIELD]=r0;  

    /* Set Complete Flag to non-zero */
    r0 = 1;
    M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.MEASUREMENT_COMPLETE_FIELD]=r0;
    // Keep Tracking Rate without settle time
    M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.ACCUMULATOR_FIELD]=NULL;
    M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_COUNTER_FIELD]=NULL;
    rts;
jp_restart:
    // Reset Timer and data collection
    r0 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_PER_SECOND_FIELD];
    r0 = r0 * -0.1 (frac);
    M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_COUNTER_FIELD]=r0;
    M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.ACCUMULATOR_FIELD]=NULL;
    M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.LAST_ACCUMULATOR_FIELD]=NULL;
           
    /* Set Complete Flag to zero */
    M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.MEASUREMENT_COMPLETE_FIELD]=NULL;
    rts;

// *****************************************************************************
// MODULE:
//    $cbops.rate_monitor_op.main
//
// DESCRIPTION:
//    Accumulate data from monitor point and update rate adjustment
//
// INPUTS:
//    - r4 = pointer to list of buffers
//    - r8 = pointer to cbops object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    all
//
// *****************************************************************************
$cbops.rate_monitor_op.main:

   // Get input data
   push rlink;

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.rate_monitor_op.main.PATCH_ID_0, r7)
#endif

   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];
   call $cbops.get_amount_ptr;
   pop rLink;
   r10 = M[r0];
   if LE rts;

   r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   NULL = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.MONITOR_MODE_FIELD];
   if Z jump jp_sw_monitor_main;

   
      // Are we collecting data?
      NULL = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_COUNTER_FIELD];
      if NEG rts;

      // Accumulate data
      r0 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.ACCUMULATOR_FIELD];
      r0 = r0 + r10;
      M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.ACCUMULATOR_FIELD]=r0;
                                           
      rts;

jp_sw_monitor_main:

   // It essentially runs with "zero" channels, so parameters start where index table
   // would have begun.

   r9 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.ACCUMULATOR_FIELD];
   r0 = r10;
   if LE jump no_data;
      // Accumulate data
      r9 = r9 + r0;
      // We have data.  So clear the stall count
      M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.STALL_FIELD] = NULL;
   no_data:
  
   // If negative this indicates delaying after a stall? - so don't perform the rate match calculation yet 
   r0 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_COUNTER_FIELD];
   if LE rts;           
   
   M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.ACCUMULATOR_FIELD] = r9;

   // Compare accumulated data to expected amount of samples
   // (10 second intervals is good)
   r0 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_COUNTER_FIELD];
   r1 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.COLLECT_PERIODS_FIELD];
   Null = r0 - r1;
   if NEG rts;

   // Load number of samples provided by source
   M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.LAST_ACCUMULATOR_FIELD] = r9;

   rMAC = r9;
   if Z jump reset_rate_monitor;    // Test for accumulate==0
   
   // Load number of expected samples.  Target Rate * Number seconds (r2)
   r9   = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.EXPECTED_ACUM_FIELD];

   // Warp = actual_number_of_samples / expected_number_of_samples
   // Frac divide + 1 more shift to make sure it's bigger than numerator
   r3 = SIGNDET rMAC;
   // May need to do a sign detect to ensure r4 doesn't saturate
   r4 = r9 ASHIFT r3;
   r9 = r3 - 2;
   rMAC = rMAC ASHIFT r9;
   Div = rMAC / r4;
// *****************************************************************************
// Compute the average warp value using:
//    average_IO_ratio = alpha * instantanious_IO_ratio + (1 - alpha) * average_IO_ratio
//
// Where:
//    The first alpha is 1 because they will have only been one warp.
//    The second alpha is 1/2 because there will have been 2 warps.
//    The third alpha is 1/3.
//     .......
//    After 10 IO ratios have been collected alpha will be held at 1/10,
//    which will make it such that we'll continue averaging the last
//    10 warp values.  This should be a sufficient amount of data to average
//    to get a good warp value.  We need to average

   // Load the appropriate alpha value
   r3 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.CURRENT_ALPHA_INDEX_FIELD];
   Words2Addr(r3);                  // alpha_index (arch4: in addrs)
   r9 = M[r3 + &alpha_warp_table];
   Addr2Words(r3);

   r3 = r3 + 1;
   r4 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.ALPHA_LIMIT_FIELD];
   Null = r3 - r4;
   if POS r3 = r4;
   M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.CURRENT_ALPHA_INDEX_FIELD] = r3;

   r4 = 1.0;
   r1 = r4 - r9; // 1 - alpha
   r3 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.AVERAGE_IO_RATE_FIELD];

   // Division result is Q.22
   r0 = DivResult;

// rMAC = alpha * instantanious IO Ratio
// rMAC = rMAC + (1 - alpha) * average_warp
   rMAC = r9 * r0;
   rMAC = rMAC + r1 * r3;
   r4 = rMAC;
   M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.AVERAGE_IO_RATE_FIELD] = r4;

   // limit final warp value to be within -0.05 to 0.05 because that is the range specified by $cbops.rate_adjustment_and_shift.
   r3 = 0.505;
   Null = r4 - r3;
   if POS r4 = r3;
   r3 = 0.495;
   Null = r4 - r3;
   if NEG r4 = r3;
   
   r4 = r4 ASHIFT (24 - DAWTH);
 
   // Measured rate is expected/accumulate  (0.5 +/- 0.005)
   M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.MEASURED_RATE_FIELD] = r4;

reset_rate_monitor:
   M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.ACCUMULATOR_FIELD] = Null;
          
   // On restart condition start give a hold time       
   r1 = M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_PER_SECOND_FIELD];
   r1 = r1 * -0.1 (frac);
   M[r7 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_COUNTER_FIELD]     = r1;

   rts;

// *****************************************************************************
// MODULE:
//   void rate_monitor_op_initiatlize(cbops_op *op, unsigned target,bool bHwSw,unsigned meas_period_msec);
//
// DESCRIPTION:
//   ADC/DAC rate matching amount to use functions
//
// INPUTS:
//    - r0 = pointer to operator structure
//    - r1 = Target Rate (Hz)
//    - r2 = mode
//    - r3 = measurement period in msec

//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//    r0, r1
//
// *****************************************************************************
$_rate_monitor_op_initiatlize:

   // here we got pointer to the cbops structure, so need to get to the
   // parameter struct first.
   r0 = M[r0 + ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];
   
   // as it works with "zero" channels, the params start where the index table
   // would be. We just skip the header's nr of in/out channel fields.
   M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.MONITOR_MODE_FIELD]  = r2;

   // periods = periods_per_sec*meas_period_msec*0.001
   // need to split the msec to second multiply for resolution
   r2 = M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_PER_SECOND_FIELD];
   r3 = r3 * 0.1 (frac);
   r2 = r2 * r3 (int);        
   r2 = r2 * 0.01 (frac);
   M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.COLLECT_PERIODS_FIELD]=r2;
   r1 = r1 * 0.01 (frac);          // r1 max 480
   r1 = r1 * r3 (int);             // r3 max 1000 r2   
   M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.EXPECTED_ACUM_FIELD]=r1;

   // Reset Timer and data collection after a settle time 
   r1 = M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_PER_SECOND_FIELD];
   r1 = r1 * -0.1 (frac);
   M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_COUNTER_FIELD]=r1;

   r1 = 10;
   M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.STALL_FIELD]=r1;
   
   r1 = 0.5;
   r1 = r1 ASHIFT (24 - DAWTH);
   M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.MEASURED_RATE_FIELD]=r1;
   
   M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.ACCUMULATOR_FIELD]=NULL;
   M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.LAST_ACCUMULATOR_FIELD]=NULL;
   M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.MEASUREMENT_COMPLETE_FIELD]=NULL;
   
   rts;   

// *****************************************************************************
// MODULE:
//   int rate_monitor_op_get_rate(cbops_op *op,unsigned dir)
//
// DESCRIPTION:
//   Return measured rate
//
// INPUTS:
//    - r0 = pointer to operator structure
//    - r1 = direction, 0=source,1=sink
//
// OUTPUTS:
//    r0 in Qx.22 for all DAWTH's - changed within the amount to use or main fns
//    for software or hardware rate monitor. MEASURED_RATE_FIELD as
//    reg ASHIFT (24 - DAWTH)
//
// TRASHED REGISTERS:
//    r2,r3
//
// *****************************************************************************
$_rate_monitor_op_get_rate:
     // if(!op)return(0);
     NULL = r0;
     if Z jump jp_abort;
        // here we got pointer to the cbops structure, so need to get to the
        // parameter struct first.
        r3 = M[r0 + ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];
        r0 = M[r3+$cbops_rate_monitor_op.rate_monitor_op_struct.MEASURED_RATE_FIELD];
        rts;
     
jp_abort:
     r0 = 0.5;
     r0 = r0 ASHIFT (24 - DAWTH);
     rts;
     
// *****************************************************************************
// MODULE:
//   void rate_monitor_op_restart (cbops_op *op)
//
// DESCRIPTION:
//   Clear measurement complete flag on restart. Measurement complete flag also
//   cleared within restart's within the algorithm and initialize. It is set
//   at the end of the amount to use function. Only applicable to rate monitor for 
//   H/W warp since measurement and enactment can occur asynchronosly
//
// INPUTS:
//    - r0 = pointer to operator structure
// OUTPUTS:
//
// TRASHED REGISTERS:
//   
//
// *****************************************************************************     
$_rate_monitor_op_restart:
   
   NULL = r0;
   if Z rts;
   r3 = M[r0 + ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];
   M[r3 + $cbops_rate_monitor_op.rate_monitor_op_struct.MEASUREMENT_COMPLETE_FIELD]=NULL;
   rts;

// *****************************************************************************
// MODULE:
//   int rate_monitor_op_is_complete (cbops_op *op)
//
// DESCRIPTION:
//   Indicate measurement is complete and new measurement can begin. New measuments
//   start immediately without hold time
// 
// INPUTS:
//    - r0 = pointer to operator structure
//
// OUTPUTS:
// r0 indicating measurement is complete (non-zero)
//
// TRASHED REGISTERS:
//   
//
// *****************************************************************************  
$_rate_monitor_op_is_complete:
   NULL = r0;
   if Z rts;
   r3 = M[r0 + ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];
   r0 =  M[r3 + $cbops_rate_monitor_op.rate_monitor_op_struct.MEASUREMENT_COMPLETE_FIELD];
   rts;             
   
$_get_rate_monitor_last_acc:
   NULL = r0;
   if Z rts;
   r3 = M[r0 + ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];
   r0 = M[r3 + $cbops_rate_monitor_op.rate_monitor_op_struct.LAST_ACCUMULATOR_FIELD];
   rts;
   
$_rate_monitor_simple_reset:
   NULL = r0;
   if Z rts;
   r3 = M[r0 + ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];
   M[r3 + $cbops_rate_monitor_op.rate_monitor_op_struct.ACCUMULATOR_FIELD] = Null;     
   // On restart condition start give a hold time       
   r1 = M[r3 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_PER_SECOND_FIELD];
   r1 = r1 * -0.1 (frac);
   M[r3 + $cbops_rate_monitor_op.rate_monitor_op_struct.PERIOD_COUNTER_FIELD] = r1;
   rts;
   
.ENDMODULE;


