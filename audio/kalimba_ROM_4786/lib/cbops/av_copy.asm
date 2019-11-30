// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************



#include "stack.h"
#ifdef KYMERA
#include "buffer/cbuffer_asm.h"
#else
#include "cbuffer.h"
#endif
#include "cbops.h"


// *****************************************************************************

#define CBOPS_AV_COPY_SILENCE_BUF_LEN              120     // length of the silence buffers 
#define CBOPS_AV_COPY_SHIFT_AMOUNT                 8      // shift value used when calculating average data in port to have better accuracy
#define CBOPS_AV_COPY_AVG_FACTOR                   0.05   // coefficient used in calculating average by attack-release formula: x_avg = alpha*x + (1-alpha)*x_avg
#define CBOPS_AV_COPY_MAX_AMOUNT_IN_PORT           110     // when leaving interrupt there should be a minimum of data in the dac port, that minimum shouldnt exceed this value
#define CBOPS_AV_COPY_MAX_REMOVE_FROM_BUFFER       72     // maximum num of samples removed from c-buffer if port is inactive for a minimum number of interrupts
#define CBOPS_AV_COPY_MINIMUM_NO_CHANGE_READ_ADDR  10     // minimum number of interrupts the read pointer of c-buffer should remain fixed to conclude port is inactive
#define CBOPS_AV_COPY_MAX_INSERT_TO_BUFFER         96     // maximum number of samples inserted into input buffer if adc is inactive

.MODULE $M.cbops.av_copy;
   .DATASEGMENT DM;
   .CODESEGMENT PM;

   //defining common variables can be shared by both adc and dac copy routines
   // allocating memory for silence buffers
   .VAR/DMCIRC left_silence_buffer[CBOPS_AV_COPY_SILENCE_BUF_LEN+1];
   .VAR/DMCIRC right_silence_buffer[CBOPS_AV_COPY_SILENCE_BUF_LEN+1];

   //allocating memory for silence cbufer structs
   
   #ifdef BASE_REGISTER_MODE
     
     .VAR left_silence_cbuffer_struc[$cbuffer.STRUC_SIZE] =
          LENGTH(left_silence_buffer),       // size
          &left_silence_buffer,              // read pointer
          &left_silence_buffer,              // write pointer
          0 ...;
                   
					
    .VAR right_silence_cbuffer_struc[$cbuffer.STRUC_SIZE] =
          LENGTH(right_silence_buffer),      // size
          &right_silence_buffer,             // read pointer
          &right_silence_buffer,             // write
          0 ...;
   #else
   
   .VAR left_silence_cbuffer_struc[$cbuffer.STRUC_SIZE] =
          LENGTH(left_silence_buffer),       // size
          &left_silence_buffer,              // read pointer
          &left_silence_buffer;              // write pointer
                   
					
   .VAR right_silence_cbuffer_struc[$cbuffer.STRUC_SIZE] =
          LENGTH(right_silence_buffer),      // size
          &right_silence_buffer,             // read pointer
          &right_silence_buffer;             // write
   #endif
   
    .VAR silence_buffer_struc[7];             // copy structure used to pass silence cbuffers to cbops.copy
    .VAR control_port;
    .VAR control_cbuffer;
    .VAR right_channel;
 .ENDMODULE;


// *****************************************************************************
// MODULE:
//    $$cbops.av_copy.init_avcopy_struct
//
// DESCRIPTION:
//    initializes the avcopy structure
//
// INPUTS:
//    - r8 = Pointer to main copy struct
//
//
// OUTPUTS:
//    - r0  control cbuffer
//    - r1  control port
//
// TRASHED REGISTERS:
//   r2, r7, r10, I0
//
//
// *****************************************************************************
.MODULE $M.cbops.av_copy.init_avcopy_struct;
 .DATASEGMENT DM;
 .CODESEGMENT PM;

  $cbops.av_copy.init_avcopy_struct:

    //building silence copy struct from input struct
    I0 =  r8;
    r10 = 7;
    I1 = &$M.cbops.av_copy.silence_buffer_struc;
    do loop_build_silence_struct;
       r0 = M[I0, 1];
      M[I1, 1] = r0;
    loop_build_silence_struct:

     //get the control port
     r0 = M[r8 + $cbops.NUM_INPUTS_FIELD];
     r1 = r0 + 3;
     r1 = M[r1 + r8];

     //get the control cbuffer
     r0 = M[ r8 + ($cbops.NUM_INPUTS_FIELD + 1)];
     M[$M.cbops.av_copy.right_channel] = 0;

     //set the input cbuffers struct fields for silence copy structs
     r7 = &$M.cbops.av_copy.silence_buffer_struc + $cbops.NUM_INPUTS_FIELD + 1;
     r2 = &$M.cbops.av_copy.left_silence_cbuffer_struc;
     M[r7 + 0 ] = r2;

     //check if it is stereo
    r2 = M[r8 + $cbops.NUM_INPUTS_FIELD];
     Null = r2 - 2;
     if NZ jump is_mono_struct;
        r2 = &$M.cbops.av_copy.right_silence_cbuffer_struc;
        M[r7 + 1 ] = r2;
       r2 = M[ r8 + ($cbops.NUM_INPUTS_FIELD + 2)];
        M[$M.cbops.av_copy.right_channel] = r2;
     is_mono_struct:

     rts;
 .ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.av_copy.fill_silence_buffer_with_zeros
//
// DESCRIPTION:
//    fills the buffer with zeros
//
// INPUTS:
//    - r0 = Pointer to cbuffer struc
//    - r10 = number of zeros
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//   r1, I0, L0
//
//
// *****************************************************************************
.MODULE $M.cbops.av_copy.fill_silence_buffer_with_zeros;
   .DATASEGMENT DM;
   .CODESEGMENT PM;
  $cbops.av_copy.fill_silence_buffer_with_zeros:

    //get the size of buffer
    r1 = M[r0 + $cbuffer.SIZE_FIELD];
    L0 = r1;

    // get the read address
    r1 = M[r0 + $cbuffer.READ_ADDR_FIELD];
    I0 = r1;

    // fill the buffer with zeros
    r1 = 0;
   do loop_fill_zero;
       M[I0, 1] = r1;
    loop_fill_zero:
    L0 = 0;

   // update write pointer
    r1 = I0;
   M[r0 + $cbuffer.WRITE_ADDR_FIELD]= r1;
rts;
.ENDMODULE;

// ******************************************************************************************************
// MODULE:
//    $cbops.dac_av_copy
//
// DESCRIPTION:
//    runs the main cbops.copy function to Copy available data from the input cbuffer to output DAC port,
//    prevents DAC to wrap by inserting  a proper number of silence samples to the port,
//    and removes samples from input buffers if DAC is not active
//
//
//
// INPUTS:
//    - pointer to cbop copy structure same as cbops.copy
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//   everything
//
// NOTES:
//    Interrupts must be blocked during this call.
//
// *******************************************************************************************************

.MODULE $M.cbops.dac_av_copy;
   .DATASEGMENT DM;
   .CODESEGMENT PM;


   .VAR port_mnt_data_avg = 5800;          // average amount of data in port
   .VAR prev_port_mnt_space;               // amount of space in previous interrupt
   .VAR prev_buffer_read_addr;             // previous read address of cbuffer
   .VAR buffer_read_addr_nochange_counter; // counter for holding number of times the read address of cbuffer soeasnt change
   .VAR buffer_level;

$cbops.dac_av_copy:

    // push rLink onto stack
    push rLink;

     //initialize the av_copy structs
     call $cbops.av_copy.init_avcopy_struct;
     M[$M.cbops.av_copy.control_port] = r1;
    M[$M.cbops.av_copy.control_cbuffer] = r0;

     // -- calculate the avrage number of samples each period is played
    r0 = M[$M.cbops.av_copy.control_port];
     call $cbuffer.is_it_enabled;
    // no averaging if port is disabled
    if Z jump end_of_averaging;
    

     r0 = M[$M.cbops.av_copy.control_cbuffer];
     call $cbuffer.calc_amount_data;
     M[&buffer_level] = r0;
  
    
    r0 = M[$M.cbops.av_copy.control_port];
     call $cbuffer.calc_amount_space;
     r2 = r2 ASHIFT -1;
     r2 = r2 + r0;
     r1 = r0 - M[prev_port_mnt_space];  
     //if r0 is negative it means a wrap around happend
     if NEG r1 = r2 - r0;
     // shift the value to increase the accuracy
     r0 = r1 ASHIFT CBOPS_AV_COPY_SHIFT_AMOUNT;
     r0 = r0 * CBOPS_AV_COPY_AVG_FACTOR (frac);
     r1 = M[port_mnt_data_avg];
     r1 = r1 * (1.0 - CBOPS_AV_COPY_AVG_FACTOR) (frac);
     r1 = r1 + r0;
     M[port_mnt_data_avg] = r1;
     jump end_of_averaging;

end_of_averaging:

     //run copy for main input copy struct
     call $cbops.copy;
 
     r0 = M[$M.cbops.av_copy.control_port];
     call $cbuffer.calc_amount_space;
     r2 = r2 ASHIFT -1;
     r0 = r2 - r0;  // r0 = num of samples in port


     r1 = M[port_mnt_data_avg];
     r1 = r1 ASHIFT (-CBOPS_AV_COPY_SHIFT_AMOUNT);


     NULL = M[&buffer_level];
     if Z jump zero_input_level;
     

     r7 = r1 - r0;
     if LT jump No_need_to_insert;

     r7 = r1;
     jump update_silence_buf;

zero_input_level:
     r7 = CBOPS_AV_COPY_MAX_AMOUNT_IN_PORT - r0;
     if NEG jump No_need_to_insert;
     
     
update_silence_buf:
     r10 = r7;
     r0 = &$M.cbops.av_copy.left_silence_cbuffer_struc;
     call $cbops.av_copy.fill_silence_buffer_with_zeros;



     //do the same for right buffer
    r10 = r7;
     r0 = &$M.cbops.av_copy.right_silence_cbuffer_struc;
     call $cbops.av_copy.fill_silence_buffer_with_zeros;

     //run copy for silence copy struct
     r8 = &$M.cbops.av_copy.silence_buffer_struc;
     call $cbops.copy;

 No_need_to_insert:

     // get the space available in the prot
     r0 = M[$M.cbops.av_copy.control_port];
     call $cbuffer.calc_amount_space;
     //update for next interrupt
     M[prev_port_mnt_space] = r0;


     // now check if we need to remove any sample from input cbuffer
     // get the amount of data in cbuffer
     r0 = M[$M.cbops.av_copy.control_cbuffer];
     call $cbuffer.calc_amount_data;
     r6 = r0; //save amount_data

     //get the read address of cbuffer
     r0 = M[$M.cbops.av_copy.control_cbuffer];
     call $cbuffer.get_read_address_and_size;
     r7 = r0;


     //  counter == N or NOT
     r3 = M[buffer_read_addr_nochange_counter];
     Null = r3 - CBOPS_AV_COPY_MINIMUM_NO_CHANGE_READ_ADDR;
     if Z jump check_read_pointer_only;

     //Counter != N
     r3 = 0;

     // prev_addr == current addr?
     r2 = r7 - M[prev_buffer_read_addr];
     if NZ jump reset_counter;

     // amount_data> size/2?
     r2 = r1 LSHIFT -1;
     Null = r6 - r2;
     if NEG jump reset_counter;
     //increment counter
     r3 = M[buffer_read_addr_nochange_counter];
     r3 = r3 + 1;
reset_counter:
     M[buffer_read_addr_nochange_counter] = r3;
     jump remove_end;

check_read_pointer_only:
     r2 = r7 - M[prev_buffer_read_addr];
     if Z jump remove_samples_from_cbuffer;
     M[buffer_read_addr_nochange_counter] = 0;
     jump remove_end;

remove_samples_from_cbuffer:

     //time to remove some samples from cbuffer
    r0 = M[$M.cbops.av_copy.right_channel];
    if NZ call $cbuffer.calc_amount_data;
    Null = r6 - r0;
    if POS r6 = r0;
     r0 = r6 - CBOPS_AV_COPY_MAX_REMOVE_FROM_BUFFER;
    if POS r6 = r6 - r0;

     //remove sample from the input buffer
     r10 = r6;
     r0 = M[$M.cbops.av_copy.control_cbuffer];
     call $cbuffer.advance_read_ptr;

      //update r7 to current read address
     r0 = M[$M.cbops.av_copy.control_cbuffer];
     call $cbuffer.get_read_address_and_size;
     r7 = r0;

     //if stereo do the same for right buffer
     r10 = r6;
      r0  = M[$M.cbops.av_copy.right_channel];
     if NZ call $cbuffer.advance_read_ptr;
remove_end:
      //update pre-read-pointer for next interrupt
      M[prev_buffer_read_addr] =  r7;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// ******************************************************************************************************
// MODULE:
//    $cbops.dac_av_copy
//
// DESCRIPTION:
//    runs the main cbops.copy function to Copy available data from the ADC cbuffer to output buffer,
//    and insert samples to output buffers if ADC is not active
//
//
//
// INPUTS:
//    - pointer to cbop copy structure same as cbops.copy
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//   everything
//
// NOTES:
//    Interrupts must be blocked during this call.
//
// *******************************************************************************************************

.MODULE $M.cbops.adc_av_copy;
   .DATASEGMENT DM;
   .CODESEGMENT PM;


   .VAR port_mnt_data_avg;                 // average amount of data in port
   .VAR prev_port_mnt_data;               // amount of space in previous interrupt
   .VAR prev_buffer_write_addr;             // previous write address of cbuffer
   .VAR buffer_write_addr_nochange_counter; // counter for holding number of times the write address of cbuffer soeasnt change


   $cbops.adc_av_copy:


     // push rLink onto stack
     push rLink;

      //initialize the av_copy structs
     call $cbops.av_copy.init_avcopy_struct;
     M[$M.cbops.av_copy.control_port] = r0;
     M[$M.cbops.av_copy.control_cbuffer] = r1;

     // -- calculate the avrage number of samples in each period
    r0 = M[$M.cbops.av_copy.control_port];
     call $cbuffer.is_it_enabled;
    if Z jump end_of_averaging;
    r0 = M[$M.cbops.av_copy.control_port];
     call $cbuffer.calc_amount_data;
     r0 = r0 - M[prev_port_mnt_data];
     //if r0 is negative it means a wrap around happend so
     //we dont do averagin this time
     if NEG jump end_of_averaging;
    r0 = r0 ASHIFT CBOPS_AV_COPY_SHIFT_AMOUNT;
     r0 = r0 * CBOPS_AV_COPY_AVG_FACTOR (frac);
     r1 = M[port_mnt_data_avg];
     r1 = r1 * (1.0 - CBOPS_AV_COPY_AVG_FACTOR) (frac);
     r1 = r1 + r0;
     M[port_mnt_data_avg] = r1;

end_of_averaging:

     //run copy for main input copy struct
     call $cbops.copy;

     // get the data available in the prot
     r0 = M[$M.cbops.av_copy.control_port];
     call $cbuffer.calc_amount_data;
     M[prev_port_mnt_data] = r0;

     // see if wee need to insert any samples to output buffer

     // get the amount of data in cbuffer
     r0 = M[$M.cbops.av_copy.control_cbuffer];
     call $cbuffer.calc_amount_data;
     r6 = r0; //save amount_data

     //get the write address of cbuffer
     r0 = M[$M.cbops.av_copy.control_cbuffer];
     call $cbuffer.get_write_address_and_size;
     r7 = r0;


     //  counter == N or NOT
     r3 = M[buffer_write_addr_nochange_counter];
     Null = r3 - CBOPS_AV_COPY_MINIMUM_NO_CHANGE_READ_ADDR;
     if Z jump check_write_pointer_only;

     //Counter != N
     r3 = 0;

     // prev_addr == current addr?
     r2 = r7 - M[prev_buffer_write_addr];
     if NZ jump reset_counter;

     // amount_data> size/4?
     r2 = r1 LSHIFT -2;
     Null = r6 - r2;
     if POS jump reset_counter;
     //increment counter
     r3 = M[buffer_write_addr_nochange_counter];
     r3 = r3 + 1;
reset_counter:
     M[buffer_write_addr_nochange_counter] = r3;
     jump insert_end;

check_write_pointer_only:
     r2 = r7 - M[prev_buffer_write_addr];
     if Z jump insert_samples_to_cbuffer;
     M[buffer_write_addr_nochange_counter] = 0;
     jump insert_end;

insert_samples_to_cbuffer:
     //r7 zeros now must be played at the output
    r7 = M[port_mnt_data_avg];
    r0 = r7 - CBOPS_AV_COPY_MAX_INSERT_TO_BUFFER;
    if NEG r7 = r7 - r0;

    // build silence buffers
    r10 = r7;
    r0 = &$M.cbops.av_copy.left_silence_cbuffer_struc;
    call $cbops.av_copy.fill_silence_buffer_with_zeros;

     //do the same for right buffer
    r10 = r7;
    r0 = &$M.cbops.av_copy.right_silence_cbuffer_struc;
    call $cbops.av_copy.fill_silence_buffer_with_zeros;

    //run copy silence samples into output buffer
    r8 = &$M.cbops.av_copy.silence_buffer_struc;
    call $cbops.copy;


    r0 = M[$M.cbops.av_copy.control_cbuffer];
    call $cbuffer.get_write_address_and_size;
    r7 = r0;

insert_end:
   M[prev_buffer_write_addr] =  r7;


jump $pop_rLink_and_rts;


.ENDMODULE;
