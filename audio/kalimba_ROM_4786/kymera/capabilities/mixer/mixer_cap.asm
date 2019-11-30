/****************************************************************************
 * %%fullcopyright(2016)
****************************************************************************/
#include "stack.h"
#include "portability_macros.h"
#include "mixer_struct_asm_defs.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif
// --------------------------------------------------------------------------------------------------
// *****************************************************************************
// MODULE:
//    asm: $gen_mixer_process_channels
//    c:   void gen_mixer_process_channels(GEN_MIXER_OP_DATA *mixer_data,unsigned num_samples);
//
// DESCRIPTION:
//    Data processing function for mixer capability.
//
// INPUTS:
//    - r0 = pointer to mixer operator structure
//    - r1 = amount of data to be processed
//
// OUTPUTS:
//    - None
//
//
// NOTES:
//      follows C call structure
//
// *****************************************************************************
.MODULE $M.gen_mixer_process;
    .DATASEGMENT DM;
    .CODESEGMENT PM;
    .MAXIM;

// offset from FP are <FP>
.CONST $gen_mixer.local.op_data         1*ADDR_PER_WORD;
.CONST $gen_mixer.local.amount          2*ADDR_PER_WORD;
// Number of local variables on stack
.CONST $gen_mixer.local.stack_offset    2*ADDR_PER_WORD;

#ifdef MIXER_SUPPORTS_STALLS
   /* Silence Buffer for stalled streams */
   .VAR  gen_mizer_zero = 0;
#endif

   // Jump Table.  Mix is 1,2, or 3 sinks in a group
   .VAR/DM2 gen_mixer_no_trans_table[3] =
            gen_mixer_process_mix1_no_transition,
            gen_mixer_process_mix2_no_transition,
            gen_mixer_process_mix3_no_transition;

   .VAR/DM2 gen_mixer_trans_table[3] =
            gen_mixer_process_mix1_transition,
            gen_mixer_process_mix2_transition,
            gen_mixer_process_mix3_transition;


// void gen_mixer_process_channels(GEN_MIXER_OP_DATA *mixer_data,unsigned num_samples);
$_gen_mixer_process_channels:
    PUSH_ALL_C
   
#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($mixer_cap.MIXER_CAP_ASM.MIXER_CAP.PROCESSING.MIXER_CAP_PROCESSING.PATCH_ID_0, r5)     // cap_compander_patchers
#endif

    // Enable saturate on add/sub
    r3 = M[$ARITHMETIC_MODE];
    push r3;
    r3 = r3 OR $ADDSUB_SATURATE_ON_OVERFLOW_MASK;
    M[$ARITHMETIC_MODE] = r3;

    pushm <FP(=SP)>, SP = SP + $gen_mixer.local.stack_offset;
 
    // Save Parameters
    M[FP + $gen_mixer.local.op_data] = r0;
    M[FP + $gen_mixer.local.amount]  = r1;

    /* Handle Gain transitions (r9=transition count) */
    r9 = M[r0 + $mixer_struct.GEN_MIXER_OP_DATA_struct.TRANSITION_COUNT_FIELD];
    if Z jump gen_mixer_process_channels_set_transition;
       r2 = r9 - r1;
       if NEG r2 = NULL; 
       M[r0 + $mixer_struct.GEN_MIXER_OP_DATA_struct.TRANSITION_COUNT_FIELD]=r2;
       /* End of transition.  Recheck gains */
       if NZ jump gen_mixer_process_channels_set_transition;
            M[r0 + $mixer_struct.GEN_MIXER_OP_DATA_struct.RESET_GAINS_FIELD]=r0;
    gen_mixer_process_channels_set_transition:

    /* Cache Sink Buffers */ 
    r7 = M[r0 + $mixer_struct.GEN_MIXER_OP_DATA_struct.SINK_LIST_FIELD];
    gen_mixer_process_channels_next_sink_buffer_set:
#ifdef MIXER_SUPPORTS_STALLS
       // If stalled use dummy (silence) buffer
       r0 = M[r7 + $mixer_struct.gen_mixer_sink_info_struct.LPGROUP_FIELD];
       NULL = M[r0 + $mixer_struct.gen_mixer_stream_struct.STALL_MASK_FIELD];
       if Z jump gen_mixer_process_channels_next_sink_buffer_set_no_stall;
           r0 = &gen_mizer_zero;
           r1 = MK1;
           M[r7 + $mixer_struct.gen_mixer_sink_info_struct.READ_ADDR_FIELD] = r0;
           M[r7 + $mixer_struct.gen_mixer_sink_info_struct.LENGTH_FIELD]    = r1;
           M[r7 + $mixer_struct.gen_mixer_sink_info_struct.BASE_ADDR_FIELD] = r0;
           jump gen_mixer_process_channels_next_sink_buffer_set_next;
       gen_mixer_process_channels_next_sink_buffer_set_no_stall:
#endif
       r0 = M[r7 + $mixer_struct.gen_mixer_sink_info_struct.BUFFER_FIELD];
       call $cbuffer.get_read_address_and_size_and_start_address;
       M[r7 + $mixer_struct.gen_mixer_sink_info_struct.READ_ADDR_FIELD] = r0;
       M[r7 + $mixer_struct.gen_mixer_sink_info_struct.LENGTH_FIELD]    = r1;
       M[r7 + $mixer_struct.gen_mixer_sink_info_struct.BASE_ADDR_FIELD] = r2;
       gen_mixer_process_channels_next_sink_buffer_set_next:
    r7 = M[r7 + $mixer_struct.gen_mixer_sink_info_struct.NEXT_FIELD];
    if NZ jump gen_mixer_process_channels_next_sink_buffer_set;

    /* Perform Mix operation (M2 = process amount */
    M2 = M[FP + $gen_mixer.local.amount];
    r0 = M[FP + $gen_mixer.local.op_data];
    push r0;    // FP used to optimize loading buffer info, will need op_data later
    r8 = M[r0 + $mixer_struct.GEN_MIXER_OP_DATA_struct.SOURCE_LIST_FIELD];
    gen_mixer_process_channels_next_source_mix:
       /* Start of Mix.  Get source buffer into I4,L4,B4 */
       r0 = M[r8 + $mixer_struct.gen_mixer_source_info_struct.BUFFER_FIELD];
       call $cbuffer.get_write_address_and_size_and_start_address;
       I4 = r0;
       L4 = r1;
       push r2;
       pop  B4;
       // Save source address (I4) */
       push I4;

       // Get sinks to mix
       r7 = M[r8 + $mixer_struct.gen_mixer_source_info_struct.MIX_LIST_FIELD];

       // First mix set includes only sinks
       r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.SINK_FIELD];
       push r0;
       pop FP;
       // Signal first MIX group (M1=1) and setup buffer and gain
       M1  = 1;   
       r1 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD];
       r5 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.MIX_FUNCTION_FIELD];
       // Setup sink buffer 1
       I0 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.READ_ADDR_FIELD];
       L0 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.LENGTH_FIELD];
       B0 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.BASE_ADDR_FIELD];  
       // Are gains in transition (M3 = transition count) */
       M3  = r9;
       if NZ jump gen_mixer_process_channels_source_first_is_transition;
          /* Gain not in transition.  Call appropriate group mix function */
          r5 = M[gen_mixer_no_trans_table + r5];
          r10 = M2 - 1;
          r7 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.NEXT_FIELD];
          jump r5;
       gen_mixer_process_channels_source_first_is_transition:
          /* Gains changing.  transition needs to restore mix group (I7=r7)  */
          r4 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.GAIN_ADJUST_FIELD];
          I7 = r7;
          /* Call appropriate group mix function */
          r5 = M[gen_mixer_trans_table + r5];
          r10 = M2; 
          r7 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.NEXT_FIELD];
          jump r5;

    gen_mixer_process_channels_mix_continue:
       /* Continue on to additional mix groups.  Reset source address */
       I4 = M[SP - MK1];
       /*  First sink in group is the source */
       I0 = I4;
       L0 = L4;
       push B4;
       pop  B0;
       M1 = 0;
       r1 = 1.0;
       r5 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.MIX_FUNCTION_FIELD];
       // Are gains in transition (M3 = transition count) */
       M3  = r9;
       if NZ jump gen_mixer_process_channels_source_continue_is_transition;
          /* Gains not in transition.  Call appropriate group mix function */
          r5  = M[gen_mixer_no_trans_table + r5];
          r10 = M2 - 1;
          jump r5;
       gen_mixer_process_channels_source_continue_is_transition:
          /* Gains changing.  transition needs to restore mix group (I7=r7) */
          r4 = NULL;
          I7 = r7; 
          /* Call appropriate group mix function */
          r5  = M[gen_mixer_trans_table + r5];
          r10 = M2; 
          jump r5;

      gen_mixer_process_channels_mix_done:
         /* Mix complete for source.  Remove source address from stack  */
         SP = SP - ADDR_PER_WORD; 
         /* Update source buffer (I4) */
         r1 = I4;
         r0 = M[r8 + $mixer_struct.gen_mixer_source_info_struct.BUFFER_FIELD];
         call $cbuffer.set_write_address;
 
    /* Go to next source in list */
    r8 = M[r8 + $mixer_struct.gen_mixer_source_info_struct.NEXT_FIELD]; 
    if NZ jump gen_mixer_process_channels_next_source_mix;

    #if (MK1-1)
      // M2 is in samples needs to be in address
      r0 = M2;
      Words2Addr(r0);
      M2 = r0;
    #endif

    /* Advance Sink Buffers.  Restore op_data from stack*/
    pop r0;
    r7 = M[r0 + $mixer_struct.GEN_MIXER_OP_DATA_struct.SINK_LIST_FIELD];
    gen_mixer_process_channels_next_sink_buf_update:
#ifdef MIXER_SUPPORTS_STALLS
      // Do not update stalled buffers
      r0 = M[r7 + $mixer_struct.gen_mixer_sink_info_struct.LPGROUP_FIELD];
      NULL = M[r0 + $mixer_struct.gen_mixer_stream_struct.STALL_MASK_FIELD];
      if NZ jump gen_mixer_process_channels_next_sink_buf_update_next;  
#endif
      push r7;
      pop FP;
      r0 = M[r7 + $mixer_struct.gen_mixer_sink_info_struct.BUFFER_FIELD];
      // setup buffer registers
      I0 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.READ_ADDR_FIELD];
      L0 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.LENGTH_FIELD];
      B0 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.BASE_ADDR_FIELD];
      // Advance buffer
      r1 = M[I0,M2];
      r1 = I0;
      call $cbuffer.set_read_address;
    gen_mixer_process_channels_next_sink_buf_update_next:
    r7 = M[r7 + $mixer_struct.gen_mixer_sink_info_struct.NEXT_FIELD];
    if NZ jump gen_mixer_process_channels_next_sink_buf_update;

    // Clear base registers
    push NULL;
    B0 = M[SP - ADDR_PER_WORD];
    B1 = M[SP - ADDR_PER_WORD];
    B4 = M[SP - ADDR_PER_WORD];
    pop B5;

    SP = SP - $gen_mixer.local.stack_offset,popm <FP>;

    // Restore arithmetic mode */
    pop r3;
    M[$ARITHMETIC_MODE]=r3;

    POP_ALL_C
    rts;

// I4,L4,B4:  output
// I0,L0,B0:  first sink in mix
// r10 = amount_to_process
// r1  = first sink gain
// M1  = first=1/continued=0
// r7  = pointer to beginning of group

gen_mixer_process_mix3_no_transition:
   // 1st sink of mix   (r1=gain, I0,L0,B0=buffer)
   // 2nd sink of MIX   (r2=gain, I1,L1,B1=buffer)
   r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.SINK_FIELD];
   push r0;
   pop FP;
   r2 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD];
   r7 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.NEXT_FIELD];
   I1 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.READ_ADDR_FIELD];
   L1 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.LENGTH_FIELD];
   B1 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.BASE_ADDR_FIELD];  
   // 3rd sink of MIX   (r3=gain, I5,L5,B5=buffer)
   r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.SINK_FIELD];
   push r0;
   pop FP;
   r3 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD];
   I5 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.READ_ADDR_FIELD];
   L5 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.LENGTH_FIELD];
   B5 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.BASE_ADDR_FIELD];  
   // Perform 3 to 1 MIX (r10+1 samples)
   r0=M[I5,MK1];
   rMAC = r3*r0,           r0=M[I1,MK1];
   rMAC = rMAC + r2*r0,    r0=M[I0,MK1];
   rMAC = rMAC + r1*r0,    r0=M[I5,MK1];
   // Note:  If I4 & I5 are in same bank a stall will be added to the loop
   do lp_gen_mixer_process_mix3_no_transition;
      rMAC = r3*r0,        r0=M[I1,MK1],  M[I4,MK1]=rMAC;
      rMAC = rMAC + r2*r0, r0=M[I0,MK1];
      rMAC = rMAC + r1*r0, r0=M[I5,MK1];
   lp_gen_mixer_process_mix3_no_transition:
   M[I4,MK1]=rMAC;
   // Advance to next set of sinks
   r7 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.NEXT_FIELD];
   if NZ jump gen_mixer_process_channels_mix_continue;
   jump gen_mixer_process_channels_mix_done;

gen_mixer_process_mix2_no_transition:
   // 1st sink of mix   (r1=gain, I0,L0,B0=buffer)
   // 2nd sink of MIX   (r2=gain, I1,L1,B1=buffer)
   r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.SINK_FIELD];
   push r0;
   pop FP;
   r2 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD];
   I1 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.READ_ADDR_FIELD];
   L1 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.LENGTH_FIELD];
   B1 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.BASE_ADDR_FIELD];  
   // Perform 2 to 1 MIX (r10+1 samples)
   r0=M[I1,MK1];
   rMAC = r2*r0,           r0=M[I0,MK1];
   rMAC = rMAC + r1*r0,    r0=M[I1,MK1];
   // Note:  If I4 & I1 are in same bank a stall will be added to the loop
   do lp_gen_mixer_process_mix2_no_transition; 
      rMAC = r2*r0,        r0=M[I0,MK1], M[I4,MK1]=rMAC;
      rMAC = rMAC + r1*r0, r0=M[I1,MK1];
   lp_gen_mixer_process_mix2_no_transition:
   M[I4,MK1]=rMAC;
   // Always last group
   jump gen_mixer_process_channels_mix_done;

gen_mixer_process_mix1_no_transition:
   // 1st sink of mix   (r1=gain, I0,L0,B0=buffer)
   // Perform 1 to 1 MIX (r10+1 samples)  
   r0=M[I0,MK1];
   rMAC = r1*r0,    r0=M[I0,MK1];
   // Note:  If I4 & I0 are in same bank a stall will be added to the loop
   do lp_gen_mixer_process_mix1_no_transition; 
      rMAC = r1*r0, r0=M[I0,MK1], M[I4,MK1]=rMAC;
   lp_gen_mixer_process_mix1_no_transition:
   M[I4,MK1]=rMAC;
   // Always last group
   jump gen_mixer_process_channels_mix_done;

// I4,L4,B4:  output
// I0,L0,B0:  first sink in mix
// r10 = amount_to_process
// r1  = first sink gain
// M1  = first=1/continued=0

// M3  = transition count
// r4  = first sink gain adjustment per sample
// r7=I7 = pointer to beginning of group

gen_mixer_process_mix3_transition:
   // 1st sink of mix   (r1=gain, r4=gain_adj, I0,L0,B0=buffer)
   // 2nd sink of MIX   (r2=gain, r5=gain_adj, I1,L1,B1=buffer)
   r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.SINK_FIELD];
   push r0;
   pop FP;
   r2 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD];
   r5 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.GAIN_ADJUST_FIELD];
   r7 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.NEXT_FIELD];
   I1 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.READ_ADDR_FIELD];
   L1 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.LENGTH_FIELD];
   B1 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.BASE_ADDR_FIELD];  
   // 3rd sink of MIX   (r3=gain, r6=gain_adj,I5,L5,B5=buffer)
   r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.SINK_FIELD];
   push r0;
   pop FP;
   r3 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD];
   r6 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.GAIN_ADJUST_FIELD];
   I5 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.READ_ADDR_FIELD];
   L5 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.LENGTH_FIELD];
   B5 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.BASE_ADDR_FIELD];  
   // Perform 3 to 1 MIX (r10 samples)
   r0=M[I1,MK1];
   do lp_gen_mixer_process_mix3_transition;
      M3 = M3 - 1;
      if NEG jump jp_gen_mixer_process_mix3_transition_done;
         // Advance gains 
         r1 = r1 + r4;
         r2 = r2 + r5;
         r3 = r3 + r6;
      jp_gen_mixer_process_mix3_transition_done:
      rMAC = r2*r0,           r0=M[I0,MK1];
      rMAC = rMAC + r1*r0,    r0=M[I5,MK1]; 
      rMAC = rMAC + r3*r0;
      M[I4,MK1]=rMAC,         r0=M[I1,MK1];
   lp_gen_mixer_process_mix3_transition:
   // Update Gains
   r7 = I7;
   // Update 1st gain
   NULL = M1;
   if Z jump jp_gen_mixer_process_mix3_first_done;
      r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.TARGET_GAIN_FIELD];
      NULL = M3;
      if LE r1=r0;
      M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD]=r1;
      r7 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.NEXT_FIELD];
   jp_gen_mixer_process_mix3_first_done:
   // Update 2nd gain
   r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.TARGET_GAIN_FIELD];
   NULL = M3;
   if LE r2=r0;
   M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD]=r2;
   r7 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.NEXT_FIELD];
   // Update 3rd gain
   r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.TARGET_GAIN_FIELD];
   NULL = M3;
   if LE r3=r0;
   M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD]=r3;
   // Advance to next set of sinks
   r7 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.NEXT_FIELD];
   if NZ jump gen_mixer_process_channels_mix_continue;
   jump gen_mixer_process_channels_mix_done;

gen_mixer_process_mix2_transition:
   // 1st sink of mix   (r1=gain, r4=gain_adj, I0,L0,B0=buffer)
   // 2nd sink of MIX   (r2=gain, r5=gain_adj, I1,L1,B1=buffer)
   r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.SINK_FIELD];
   push r0;
   pop FP;
   r2 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD];
   r5 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.GAIN_ADJUST_FIELD];
   r7 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.NEXT_FIELD];
   I1 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.READ_ADDR_FIELD];
   L1 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.LENGTH_FIELD];
   B1 = M[FP + $mixer_struct.gen_mixer_sink_info_struct.BASE_ADDR_FIELD];  
   // Perform 2 to 1 MIX (r10 samples)
   r0=M[I1,MK1];
   do lp_gen_mixer_process_mix2_transition;
      M3 = M3 - 1;
      if NEG jump jp_gen_mixer_process_mix2_transition_done;
         // Advance gains 
         r1 = r1 + r4;
         r2 = r2 + r5;
      jp_gen_mixer_process_mix2_transition_done:
      rMAC = r2*r0,           r0=M[I0,MK1];
      rMAC = rMAC + r1*r0;
      M[I4,MK1]=rMAC,         r0=M[I1,MK1];
   lp_gen_mixer_process_mix2_transition:
   // Update Gains
   r7 = I7;
   // Update 1st gain
   NULL = M1;
   if Z jump jp_gen_mixer_process_mix2_first_done;
      r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.TARGET_GAIN_FIELD];
      NULL = M3;
      if LE r1=r0;
      M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD]=r1;
      r7 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.NEXT_FIELD];
   jp_gen_mixer_process_mix2_first_done:
   // Update 2nd gain
   r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.TARGET_GAIN_FIELD];
   NULL = M3;
   if LE r2=r0;
   M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD]=r2;
   // Always last group
   jump gen_mixer_process_channels_mix_done;

gen_mixer_process_mix1_transition:
   // 1st sink of mix   (r1=gain, r4=gain_adj, I0,L0,B0=buffer)
   // Perform 1 to 1 MIX (r10 samples)
   r0=M[I0,MK1];
   do lp_gen_mixer_process_mix1_transition;
      // Advance gains 
      M3 = M3 - 1;
      if POS r1 = r1 + r4;
      // Apply Gain
      rMAC = r1*r0;
      M[I4,MK1]=rMAC,     r0=M[I0,MK1];
   lp_gen_mixer_process_mix1_transition:
   // Update 1st gain
   r7 = I7;
   r0 = M[r7 + $mixer_struct.gen_mixer_mix_info_struct.TARGET_GAIN_FIELD];
   NULL = M3;
   if LE r1=r0;
   M[r7 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD]=r1;
   // Always last group
   jump gen_mixer_process_channels_mix_done;




// void gen_mixer_set_gain(GEN_MIXER_OP_DATA *mixer_data,GEN_MIXER_GAIN_DEF *gain_def,unsigned inv_transition);
$_gen_mixer_set_gain:
   /* Is mixer confured? */
   r3 = M[r0 + $mixer_struct.GEN_MIXER_OP_DATA_struct.MAX_SOURCES_FIELD];
   if LE rts;

   pushm <r4,r5,r6,r7,r8,r9,r10>;

   /* Signal gain change */
   r8 = 1;
   M[r0 + $mixer_struct.GEN_MIXER_OP_DATA_struct.RESET_GAINS_FIELD]=r8;
  
   r8 = M[r1 + $mixer_struct.gen_mixer_gain_def_struct.SINK_MASK_FIELD];
   r7 = M[r1 + $mixer_struct.gen_mixer_gain_def_struct.GAIN_FIELD];
   r5 = M[r0 + $mixer_struct.GEN_MIXER_OP_DATA_struct.SOURCES_FIELD];

   /* Is source index valid?.   Else go through whole source list */
   r4 = M[r1 + $mixer_struct.gen_mixer_gain_def_struct.SRC_TERMINAL_FIELD];
   if NEG jump jp_gen_mixer_set_gain_cont;
      /* Only one source */
      r4 = r4 * ADDR_PER_WORD (int);
      r5 = r5 + r4;  
      r3 = NULL;
jp_gen_mixer_set_gain_cont:
      /* Dereference source pointer (r10)*/
      r10 = M[r5];  
      r0 = M[r10 + $mixer_struct.gen_mixer_source_info_struct.CONFIGURED_SINKS_FIELD];
      r4 = r0 AND r8;
      if Z jump jp_gen_mixer_set_gain_next;
         /* Found source associated with sink.  
              r4 should have only one bit set as a stream only has one bit per source
              Make a mask from it to get gain index*/
         r10 = r10 + ($mixer_struct.gen_mixer_source_info_struct.MIXES_FIELD
                    - ($mixer_struct.gen_mixer_mix_info_struct.STRUC_SIZE*ADDR_PER_WORD));
         r6 = r4 LSHIFT 1;
         r6 = r6 - 1;
         r0 = r0 AND r6;
         r0 = ONEBITCOUNT r0;
         /* Get MIX entry from source (r4) */
         r4 = r0 * ($mixer_struct.gen_mixer_mix_info_struct.STRUC_SIZE*ADDR_PER_WORD) (int);
         r4 = r4 + r10;
         /* Save new target and set adjustment.   If adjustment is zero,reset current gain*/
         M[r4 + $mixer_struct.gen_mixer_mix_info_struct.TARGET_GAIN_FIELD] = r7;
         NULL = r2;
         if NZ jump jp_gen_mixer_set_gain_no_trans;
             M[r4 + $mixer_struct.gen_mixer_mix_info_struct.CURRENT_GAIN_FIELD]=r7;
         jp_gen_mixer_set_gain_no_trans:    
         /* Look for other sources? */         
         NULL = M[r1 + $mixer_struct.gen_mixer_gain_def_struct.SINGLE_SOURCE_FIELD];
         if NZ jump jp_gen_mixer_set_gain_done;
jp_gen_mixer_set_gain_next:            
      // Next source 
      r5 = r5 + ADDR_PER_WORD;
      r3 = r3 - 1;
      if GT jump jp_gen_mixer_set_gain_cont;

jp_gen_mixer_set_gain_done:
   popm <r4,r5,r6,r7,r8,r9,r10>;
   rts;

#ifdef MIXER_SUPPORTS_STALLS
//    unsigned gen_mixer_samples_to_time(unsigned samples,unsigned inv_sample_rate);
$_gen_mixer_samples_to_time:
   // samples to time
   rMAC = r0 * r1;
   r0 = rMAC ASHIFT (DAWTH - 11);
   rts;
   
// int gen_mixer_compute_transfer(GEN_MIXER_OP_DATA *mixer_data,unsigned *bProcess_all_input);
$_gen_mixer_compute_transfer:  
   pushm <r4,r5,r6,r7,r8,r9,rLink>;
   push r1; /* Pointer for sink kicks */

   // data object
   r9 = r0;       
   
   /* Compute Time Delta between updates*/
   r1 = M[$TIMER_TIME]; 
   r2 = M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.TIME_STAMP_FIELD]; 
   M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.TIME_STAMP_FIELD]=r1;
   r3 = r1 - r2;    
#if (MK1==1)
   /* Convert from usec to seconds (x 10^-6) and 
      apply source clock rate 
       y = (x*(10^-6)*(2^23))/x = 8.388608
       y * 2^-4 = 0.524288   */
   rMAC = r3*0.524288;
   rMAC = RMAC ASHIFT 4 (56bit);
#else
   /* Convert from usec to seconds (x 10^-6) and 
      apply source clock rate 
       y = (x*(10^-6)*(2^31))/x = 2147.483648
       y * 2^-12 = 0.524288   */
   rMAC = r3*0.524288;
   rMAC = RMAC ASHIFT 12 (56bit);
#endif


   // Minimum transfer in samples (r6)
   r6 = M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.MIN_DATA_FIELD]; 

   // Update estimated latency (r4)
   r4 = M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.EST_LATENCY_FIELD];
   r4 = r4 - rMAC;
   if NEG r4=NULL;

   /* Check sources for space */
   r5 = MAXINT;   // min_transfer
   r8 = NULL;     // sink kicks
   r7 = M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.SOURCE_LIST_FIELD];
   gen_mixer_compute_transfer.src_next:
      /* Get amount of space */
      r0 = M[r7 + $mixer_struct.gen_mixer_source_info_struct.BUFFER_FIELD];
      call $_cbuffer_calc_amount_space_in_words;
      /* Check for sufficient space */
      NULL = r0 - r6;
      if NEG jump gen_mixer_compute_transfer.abort_src;
      /* Save Minimum Space */
      r5 = MIN r0;
   r7 = M[r7 + $mixer_struct.gen_mixer_source_info_struct.NEXT_FIELD];
   if NZ jump gen_mixer_compute_transfer.src_next;

   /* Check if source buffers are not empty */
   r0 = M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.MAX_SPACE_FIELD];
   NULL = r5 - r0;
   if POS jump gen_mixer_compute_transfer.max_latency_done;
      /* Source buffer is not empty, Convert data to Time */
      r1 = M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.INV_SAMPLE_RATE_FIELD];     /* inverse sample rate (scaled 2^10) */
      rMAC = r5 * r1;
      r0 = rMAC ASHIFT (DAWTH - 11);
      /* Use source to limit estimated latency */
      r3 = M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.MAX_LATENCY_FIELD]; 
      r3 = r3 - r0;    
      if GT r4 = r3; 
   gen_mixer_compute_transfer.max_latency_done:

   M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.NOT_ALL_STALLED_FIELD]=NULL;
   /* Check sinks */
   r7 = M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.SINK_LIST_FIELD];
   gen_mixer_compute_transfer.sink_next:
       /* Get amount of data and buffer info */
       r0 = M[r7 + $mixer_struct.gen_mixer_sink_info_struct.BUFFER_FIELD];
       call $_cbuffer_calc_amount_data_in_words; 
       /* Get stall state */
       r10 = M[r7 + $mixer_struct.gen_mixer_sink_info_struct.LPGROUP_FIELD];
       r1 = M[r10 + $mixer_struct.gen_mixer_stream_struct.STALL_MASK_FIELD];
       r2 = M[r7 + $mixer_struct.gen_mixer_sink_info_struct.SINK_MASK_FIELD];
       NULL = r1 AND r2;
       if Z jump gen_mixer_compute_transfer.not_stalled;
            /* Stalled signal kick sink(r8), Is new data available? */     
            r8 = r8 OR r2;
            NULL = r0 - r6;
            if NEG jump gen_mixer_compute_transfer.sink_skip; 
            /* Clear Stalled condition. */
            r2 = r2 XOR -1;
            r1 = r1 AND r2;
            M[r10 + $mixer_struct.gen_mixer_stream_struct.STALL_MASK_FIELD]=r1;
            jump gen_mixer_compute_transfer.sink_cont;

       gen_mixer_compute_transfer.not_stalled:
            /* Not Stalled, Is there sufficient data */
            NULL = r0 - r6;
            if POS jump gen_mixer_compute_transfer.sink_cont; 
            /* Not enough data.  if not out of time, lets skip period */
            r3 = M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.MIN_LATENCY_FIELD];
            r3 = r4 - r3;
            if POS jump gen_mixer_compute_transfer.abort_sink;
            /* Set Stall Condition */    
            r1 = r1 OR r2;
            M[r10 + $mixer_struct.gen_mixer_stream_struct.STALL_MASK_FIELD]=r1;
            /* Signal Sink Kick */
            r8 = r8 OR r2;

            jump gen_mixer_compute_transfer.sink_skip;

       gen_mixer_compute_transfer.sink_cont:
            /* Sink data available.  If uses all of sink data, kick sink  */
            NULL = r5 - r0;
            if POS r8 = r8 OR r2;
            r5 = MIN r0;
            /* Check if stream is stalled. (r0 always non-zero) */
            r2 = r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.NOT_ALL_STALLED_FIELD;
            NULL = r1;
            if Z M[r2]=r0;			
       gen_mixer_compute_transfer.sink_skip:
   r7 = M[r7 + $mixer_struct.gen_mixer_sink_info_struct.NEXT_FIELD];
   if NZ jump gen_mixer_compute_transfer.sink_next; 

   /* If all sinks are stalled, abort and set timer */ 
   r3 = M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.NOT_ALL_STALLED_FIELD];
   if Z jump gen_mixer_compute_transfer.abort_sink;
   

   /* Update estimated throughput.  Add transfer_period */
   r1 = M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.INV_SAMPLE_RATE_FIELD];     /* inverse sample rate (scaled 2^10) */
   rMAC = r5 * r1;
   r0 = rMAC ASHIFT (DAWTH - 11);
   r4 = r4 + r0;
   
   // Save transfer (negative to differentiate from stall timer) */
   r0 = NULL - r5;

   gen_mixer_compute_transfer.done:
      /* Save updated latency estimate */
      r3 = M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.MAX_LATENCY_FIELD];
      r4 = MIN r3;
      M[r9 + $mixer_struct.GEN_MIXER_OP_DATA_struct.EST_LATENCY_FIELD]=r4;

      /* Return sink kicks */
      pop r1;
      M[r1]=r8;
      popm <r4,r5,r6,r7,r8,r9,rLink>;
      rts;
    
   gen_mixer_compute_transfer.abort_sink:
      /* No processing due to insufficient data.   Set timer to re-check (r3).  Make sure time is not to small */
      r6 = 0.001;
      r3 = MAX r6; 
      r0 = r3;
      jump gen_mixer_compute_transfer.done;

   gen_mixer_compute_transfer.abort_src:
      /* No processing due to insuficient space.    No Timer, will be kicked when space available */
      r0 = NULL;
      jump gen_mixer_compute_transfer.done;

#endif

.ENDMODULE;


