// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.tns_encdec
//
// DESCRIPTION:
//    Decode/Encode TNS (Temporal Noise Shaping) data and filter the spectrum
//    accordingly
//
// INPUTS:
//    - M2 = select either decode or encode mode
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r8, r10, rMAC, I0-I7, M0, M1, M3
//    - L1, L5 zeroed
//    - first 3 elements of $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.tns_encdec;
   .CODESEGMENT AACDEC_TNS_ENCDEC_PM;
   .DATASEGMENT DM;

   $aacdec.tns_encdec:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.TNS_ENCDEC_ASM.AACDEC_TNS_ENCDEC.TNS_ENCDEC.PATCH_ID_0, r1)
#endif

   // If spec has >=2 bits headroom
   // make it correct scale by multiplying by 2,
   // otherwise, do it later
   Null = M2;
   if NZ jump dont_scale;
   r1 = M[r9 + $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR];
   Null = M[r1] - 2;
   if LT jump dont_scale;
   r0 = M[r9 + $aac.mem.CURRENT_SPEC_PTR];
   I3 = r0;
   r2 = 1;
   r10 = 511;
   M[r1 + (1*ADDR_PER_WORD)] = r2;
   r1 = M[I3, MK1];
   r1 = r1 ASHIFT r2,
    r0 = M[I3, -MK1];
   do scale_loop;
      M[I3, MK2] = r1;
      r0 = r0 ASHIFT r2,
       r1 = M[I3, -MK1];
      M[I3, MK2] = r0;
      r1 = r1 ASHIFT r2,
       r0 = M[I3, -MK1];
   scale_loop:
   r0 = r0 ASHIFT r2,
    M[I3, MK1] = r1;
   M[I3, 0] = r0;
   dont_scale:


   // set up r4 as an ics pointer
   r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];

   // Speed things up by setting $aacdec.tmp[0] = TNS_MAX_ORDER
   // if (window_sequence == EIGHT_SHORT_SEQUENCE)
      // TNS_MAX_ORDER = $TNS_MAX_ORDER_SHORT;   (7)
   // else (window_sequence != EIGHT_SHORT_SEQUENCE)
      // TNS_MAX_ORDER = $TNS_MAX_ORDER_LONG;    (12)
   r1 = $aacdec.TNS_MAX_ORDER_SHORT;
   r2 = $aacdec.TNS_MAX_ORDER_LONG;
   r0 = M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD];
   Null = r0 - $aacdec.EIGHT_SHORT_SEQUENCE;
   if NZ r1 = r2;
   M[r9 + $aac.mem.TMP + (0*ADDR_PER_WORD)] = r1;


   // Speed things up by setting $aacdec.tmp[1] = TNS_MAX_BANDS
   // if (window_sequence == EIGHT_SHORT_SEQUENCE)
      // TNS_MAX_BANDS = $TNS_MAX_SFB_SHORT;
   // else (window_sequence != EIGHT_SHORT_SEQUENCE)
      // TNS_MAX_BANDS = $aacdec.tns_max_sfb_long_table[sampling_freq];

   // Note: sampling_freq = sampling_freq_lookup[sf_index]

   r5 = M[r9 + $aac.mem.SF_INDEX_FIELD];
   r0 = &$aacdec.sampling_freq_lookup;
   call $mem.ext_window_access_as_ram;
   Words2Addr(r5);
   r2 = M[r0 + r5];

   r3 = $aacdec.TNS_MAX_SFB_SHORT;
   r0 = M[r9 + $aac.mem.TNS_MAX_SFB_LONG_TABLE_FIELD];
#ifdef AACDEC_ELD_ADDITIONS
   r1 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r1 - $aacdec.ER_AAC_ELD;
   if NE jump read_tns_max_sfb;
      r1 = &$aacdec.tns_max_sfb_long_table_480;
      r0 = &$aacdec.tns_max_sfb_long_table_512;
      Null = M[r9 + $aac.mem.ELD_frame_length_flag];
      if NZ r0 = r1;
      push r2;
      call $mem.ext_window_access_as_ram;
      pop r2;
   read_tns_max_sfb:
#endif //AACDEC_ELD_ADDITIONS
   Words2Addr(r2);
   r2 = M[r0 + r2];
   r0 = M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD];

   Null = r0 - $aacdec.EIGHT_SHORT_SEQUENCE;
   if NZ r3 = r2;
   M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = r3;


   // r0 = pointer to start of the TNS data
   r0 = M[r4 + $aacdec.ics.TNS_DATA_PTR_FIELD];
   // I2 = pointer to order(0,0)
   I2 = r0 + $aacdec.tns.ORDER_FIELD;
   // I3 = pointer to length(0,0)
   I3 = r0 + $aacdec.tns.LENGTH_FIELD;
   // I5 = pointer to coef(0,0,0)
   I5 = r0 + $aacdec.tns.COEF_FIELD;
   // I6 = pointer to direction(0,0)
   I6 = r0 + $aacdec.tns.DIRECTION_FIELD;
   // I7 = pointer to coef_compress(0,0)
   I7 = r0 + $aacdec.tns.COEF_COMPRESS_FIELD;


   // allocate 24 words of temp memory
   r0 = 24;
   call $aacdec.frame_mem_pool_allocate;
   if NEG jump $aacdec.corruption;
   // store addr in $aacdec.tmp[2]
   M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)] = r1;


   // refresh tns.N_FILT_VALID_FIELD if tns_encode done in this frame already
   Null = M[r4 + $aacdec.ics.TNS_ENCODE_DONE_FIELD];
   if Z jump n_filt_valid;
      r1 = M[r4 + $aacdec.ics.TNS_N_FILT_TEMP_FIELD];
      r0 = M[r4 + $aacdec.ics.TNS_DATA_PTR_FIELD];
      M[r0 + $aacdec.tns.N_FILT_FIELD] = r1;
   n_filt_valid:


   // if in encode mode then save initial value of tns_n_filt
   Null = M2;
   if Z jump dont_save_n_filt;
      r0 = M[r4 + $aacdec.ics.TNS_DATA_PTR_FIELD];
      r1 = M[r0 + $aacdec.tns.N_FILT_FIELD];
      M[r4 + $aacdec.ics.TNS_N_FILT_TEMP_FIELD] = r1;
   dont_save_n_filt:


   // r8 = w = 0
   
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.TNS_ENCDEC_ASM.AACDEC_TNS_ENCDEC.TNS_ENCDEC.PATCH_ID_1, r8)
#endif
   
   
   r8 = 0;

   // for (w=0; w < num_windows; w++)
   window_loop:
      // r7 = bottom = num_swb
      r7 = M[r4 + $aacdec.ics.NUM_SWB_FIELD];

      // for (f=0, f < n_filt[w]; f++)
      n_filt_loop:
         // decrement n_filt[w]
         r0 = M[r4 + $aacdec.ics.TNS_DATA_PTR_FIELD];
         r0 = r0 + r8;
         r1 = M[r0 + $aacdec.tns.N_FILT_FIELD];
         // n_filt can be 0, check value here
         if Z jump end_n_filt_loop;
         r1 = r1 - 1;
         M[r0 + $aacdec.tns.N_FILT_FIELD] = r1;


         // r6 = tns_order = min(order[w][f], TNS_MAX_ORDER)
         r0 = M[I2, MK1];
         Null = r0;
         // if(!tns_order) continue;
         if NZ jump not_zero_order;
            // update bottom
            // r7 = bottom = max(top - length[w][f],0)
            r0 = M[I3, MK1];  // r0 = length[w][f]
            r7 = r7 - r0;
            if NEG r7 = 0;
            jump bottom_n_filt_loop;
         not_zero_order:
         // r6 = TNS_MAX_ORDER = $aacdec.tmp[0]
         r6 = M[r9 + $aac.mem.TMP + (0*ADDR_PER_WORD)];
         Null = r0 - r6;
         if LT r6 = r0;


         // -- Decode coef data --
         // tns_decode_coef(tns_order, coef_res[w], coef_compress[w][f], coef[w][f],lpc);

         // r2 = coef_res[w]
         r2 = M[r4 + $aacdec.ics.TNS_DATA_PTR_FIELD];
         r2 = r2 + r8;
         r2 = M[r2 + $aacdec.tns.COEF_RES_FIELD];

         // set r0 = tns_lookup_coefs[0] if coef_res = 0,
         //       or tns_lookup_coefs[8] if coef_res = 1
         r0 = r2 LSHIFT 3;
         // set pointer to coef lookup
         r1 = M[r9 + $aac.mem.TNS_LOOKUP_COEFS_FIELD];
         Words2Addr(r0);
         r0 = r0 + r1;

         // calculate jump point for the 2 smaller sections of the lookup table
         r1 = M[I7, MK1];    // r1 = coef_compress
         // r1 =
         //              coef_compress:
         //                0     1
         //   coef_res:
         //       0        16    2
         //       1        16    4
         //
         r1 = r1 LSHIFT r2;
         r1 = r1 LSHIFT 1;
         if NZ jump test_needed;
            r1 = 16;
         test_needed:

         // r3 = jump amount (8 or 4), if required. Else unused.
         r3 = r1 LSHIFT 1;


         // if are in decode mode and tns_encode has already taken place this frame
         // dont repeat the inverse quant loop
         Null = M2;
         if NZ jump are_in_encode_mode;
            Null = M[r4 + $aacdec.ics.TNS_ENCODE_DONE_FIELD];
            if NZ jump dont_do_conversion_again;
         are_in_encode_mode:

            r10 = r6;                    // r6 = order
            do inverse_quant_loop;       // lookup coef's from RAM
               r2 = M[I5,0];                // coef[i]
               Null = r1 - r2;              // test if jump is required
               if LE r2 = r2 + r3;          // execute jump if needed
               Words2Addr(r2);
               r2 = M[r0 + r2];             // get lookup value (r0 = location of lookup_coef)
               M[I5, MK1] = r2;                // write over input value
            inverse_quant_loop:

            // set coef pointer back to the beginning
            Words2Addr(r6);
            I5 = I5 - r6;
            Addr2Words(r6);
         dont_do_conversion_again:

         // -- Conversion to LPC coefficients --

         // r2 = location of temp memory = a[0]
         // r3 = b[0]
         r2 = M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)];
         r3 = r2 + (12*ADDR_PER_WORD);

         // for(m=0; m<order; m++)
         r5 = 0;   // m = 0
         lpc_outer_loop:

            r1 = M[I5, MK1];         // r1 = tmp2[m];

            I4 = r2;              // I4 = &a[0]
            I0 = r3;              // I0 = &b[0]
            Words2Addr(r5);
            I1 = r2 + r5;         // I1 = &a[m]
            Addr2Words(r5);
            I1 = I1 - MK1;

            r0 = M[I1, -MK1],        // r0 = a[m]
             rMAC = M[I4, MK1];      // rMAC = a[i]

            // for(i=0; i<m; i++)
            r10 = r5;
            do lpc_calc_loop;
               // b[i] = a[i] + temp2[m] * a[m-i];
               rMAC = rMAC + r0*r1,
                r0 = M[I1, -MK1];       // a[m-i]
               M[I0, MK1] = rMAC,       // store b[i]
                rMAC = M[I4, MK1];      // load a[i]
            lpc_calc_loop:

            I4 = r2;              // I4 = &a[0]
            I1 = r3;              // I1 = &b[0]
            rMAC = M[I1, MK1];       // rMAC = b[0]

            // for(i=0; i<m; i++)
            r10 = r5;
            do lpc_copy_loop;
               rMAC = M[I1, MK1],
                M[I4, MK1] = rMAC;      // a[i] = b[i]
            lpc_copy_loop:

            // a[m] = tmp2[m] shifted
            r1 = r1 * (1.0/(1 << $aacdec.TNS_LPC_SHIFT_AMOUNT)) (frac);
            Words2Addr(r5);
            M[r2 + r5] = r1;
            Addr2Words(r5);

            // incr m
            r5 = r5 + 1;
            Null = r5 - r6;   // r6 = order
         if NEG jump lpc_outer_loop;

         // I4 = &lpc(1)
         I4 = r2;

         // r1 = top = bottom
         r1 = r7;
         // r7 = bottom = max(top - length[w][f],0)
         r0 = M[I3, MK1];  // r0 = length[w][f]
         r7 = r1 - r0;
         if NEG r7 = 0;

         // r5 = start = swb_offset[min(bottom,TNS_MAX_BANDS,max_sfb)];
         r5 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];              // TNS_MAX_BANDS = $aacdec.tmp[1]
         Null = r5 - r7;                       // r7 = bottom
         if POS r5 = r7;
         r0 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];     // r0 = max_sfb
         Null = r5 - r0;
         if POS r5 = r0;
         r2 = M[r4 + $aacdec.ics.SWB_OFFSET_PTR_FIELD];
         Words2Addr(r5);
         r5 = M[r2 + r5];


         // r3 = end = swb_offset[min(top,TNS_MAX_BANDS,max_sfb)];
         r3 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];              // TNS_MAX_BANDS = $aacdec.tmp[1]
         Null = r3 - r1;                       // r1 = top
         if POS r3 = r1;
         Null = r3 - r0;                       // r0 = max_sfb
         if POS r3 = r0;
         Words2Addr(r3);
         r3 = M[r2 + r3];


         r0 = M[I6, MK1];                         // r0 = direction
         // if ((size = end - start) <= 0) continue;
         r2 = r3 - r5;
         if LE jump bottom_n_filt_loop;
         // if (direction[w][f])
         //    inc = -1; start = end - 1;
         // else
         //    inc = 1;
         M0 = MK1;                               // inc = 1
         Null = r0;
         if Z jump direction_zero;             // if (!direction[w][f])
            M0 = -MK1;                           //    inc = -1
            r5 = r3 - 1;                       //    start = end - 1
         direction_zero:


         // set up modify register values and initial order = 0
         // call iir_filter (decode mode); fir_filter (encode mode)
         M1 = MK1;

         // if (decode_mode) I0 = current_spec_ptr
         // else I0 = &$aacdec.tmp_mem_pool
         r0 = M[r9 + $aac.mem.CURRENT_SPEC_PTR];
         Words2Addr(r5);
         r0 = r5 + r0;
         I0 = r0;
         r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r1 = r5 + r1;
         Null = M2;
         if NZ I0 = r1;

         // I0 = spec[w][start] = &$aacdec.current_spec_ptr + w*128 + start
         r3 = r8 * 128(int);
         I0 = I0 + r3;

         // r3 = order (no. of iterations)
         r3 = 0;

         Null = M2;
         if NZ jump tns_encode_mode;
            // decode mode
            // M3 = -M0 = -inc
            M3 = -M0;
            call $aacdec.iir_filter;
            jump n_filt_loop;

         tns_encode_mode:
            // encode mode
            M3 = -MK1;
            call $aacdec.fir_filter;

         bottom_n_filt_loop:
      jump n_filt_loop;

      end_n_filt_loop:

      // update [w]
      r8 = r8 + ADDR_PER_WORD;

      r5 = M[r4 + $aacdec.ics.NUM_WINDOWS_FIELD];
      Words2Addr(r5);
      Null = r5 - r8;
   if GT jump window_loop;

   // pop rLink from stack
   jump $pop_rLink_and_rts;


.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.iir_filter
//
// DESCRIPTION:
//    IIR filter for TNS decoding (Temporal Noise Shaping)
//
// INPUTS:
//    - r2 = size
//    - r3 = 0
//    - r6 = order
//    - I0 = address to start filtering
//    - I4 = start of lpc coefs
//    - M0 = inc (direction of filter, +1 forwards, -1 backwards)
//    - M1 = 1*ADDR_PER_WORD
//    - M3 = -inc*ADDR_PER_WORD
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r10, rMAC, I0, I1, I4
//
// NOTES:
//    tns_ar_filter(&spec[w][start], size, inc, lpc[], tns_order);
//    Simple all-pole filter of order "order" defined by
//    y(n) = x(n) - lpc[1]*y(n-1) - ... - lpc[order]*y(n-order)
//
// *****************************************************************************
.MODULE $M.aacdec.iir_filter;
   .CODESEGMENT AACDEC_IIR_FILTER_PM;
   .DATASEGMENT DM;

   $aacdec.iir_filter:

   // loop size times (M0)
   iir_filter_loop:
      // start at the beginning again
      I1 = I0;
      // r10 = order (or less if initialising)
      r10 = r3,
       rMAC = M[I1,M3];           // read x(n)
      rMAC = rMAC * (1.0/(1 << $aacdec.TNS_LPC_SHIFT_AMOUNT));
      r0 = M[I1,M3],              // read y(n-1)
       r1 = M[I4,M1];             // read lpc(1)
      do iir_loop;                // loop though the other coefs
         rMAC = rMAC - r0 * r1,      // accumulate filter output
          r0 = M[I1,M3],             // read next y(?)
          r1 = M[I4,M1];             // read next lpc(?)
      iir_loop:

      // shift the data back to the correct value
      r0 = rMAC ASHIFT $aacdec.TNS_LPC_SHIFT_AMOUNT;
      // increment startup order
      r3 = r3 + 1;
      Words2Addr(r3);
      // set I4 back to lpc(1)
      I4 = I4 - r3,
       M[I0,M0] = r0;            // store new filter output y(n)

      Addr2Words(r3);
      // limit startup order to actual order
      Null = r6 - r3;
      if NEG r3 = r6;

      // move on to next value
      r2 = r2 - 1;
   if NZ jump iir_filter_loop;

   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.fir_filter
//
// DESCRIPTION:
//    FIR filter for TNS encoding (Temporal Noise Shaping)
//
// INPUTS:
//    - r2 = size
//    - r3 = 0
//    - r6 = order
//    - I0 = address to start filtering
//    - I4 = start of lpc coefs
//    - M0 = inc (direction of filter, +1 forwards, -1 backwards)
//    - M1 = 1
//    - M3 = -inc
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r10, rMAC, I0, I1, I4
//    - L1, L5 zeroed
//
// NOTES:
//    FIR(&spec[w][start], size, inc, lpc[], tns_order);
//    all-zero filter of order "order" defined by
//    y(n) = x(n) + lpc[1]*x(n-1) + ... + lpc[order]*x(n-order)
//
// *****************************************************************************
.MODULE $M.aacdec.fir_filter;
   .CODESEGMENT AACDEC_FIR_FILTER_PM;
   .DATASEGMENT DM;

   $aacdec.fir_filter:

   r0 = 1;
   M[r4 + $aacdec.ics.TNS_ENCODE_DONE_FIELD] = r0;

   r0 = I5;
   M[r9 + $aac.mem.TEMP_I5] = r0;

   // write pointer to the previous input values ( history of x() )
   r0 = M[r9 + $aac.mem.TNS_FIR_INPUT_HISTORY_PTR];
 #ifdef BASE_REGISTER_MODE
   push r0;
   pop B5;
 #endif
   I5 = r0;
   L5 = $aacdec.TNS_MAX_ORDER_LONG*ADDR_PER_WORD;

   // read pointer to the previous input values ( history of x() )
   I1 = r0;
   L1 = $aacdec.TNS_MAX_ORDER_LONG*ADDR_PER_WORD;
#ifdef BASE_REGISTER_MODE
   push r0;
   pop B1;
#endif

   fir_filter_loop:
      // start at the beginning again

      // r10 = order (or less if initialising)
      r10 = r3;
      rMAC = M[I0,0];           // read x(n)
      rMAC = rMAC * (1.0/(1 << $aacdec.TNS_LPC_SHIFT_AMOUNT));

      r0 = M[I1,M3],            // read next x(n)
       r1 = M[I4,M1];           // read lpc(1)

      do fir_loop;              // loop though the other coefs
         rMAC = rMAC + r0 * r1,    // accumulate filter output
          r0 = M[I1,M3],           // read next x(n-1)
          r1 = M[I4,M1];           // read next lpc(?)
      fir_loop:

      // shift the data back to the correct value
      rMAC = rMAC * (1 << $aacdec.TNS_LPC_SHIFT_AMOUNT) (int) (sat);

      // increment startup order
      r3 = r3 + 1;
      Words2Addr(r3);

      // set I4 back to lpc(1)
      I4 = I4 - r3,
       r0 = M[I0, 0];           // read x(n)

      I1 = I5,                  // set read pointer to x(n)
       M[I0,M0] = rMAC;         // store new filter output y(n)

      M[I5, MK1] = r0;             // store x(n)
      Addr2Words(r3);

      // limit startup order to actual order
      Null = r6 - r3;
      if NEG r3 = r6;

      // move on to next value
      r2 = r2 - 1;
   if NZ jump fir_filter_loop;

   r0 = M[r9 + $aac.mem.TEMP_I5];
   I5 = r0;

   L1 = 0;
   L5 = 0;
 #ifdef BASE_REGISTER_MODE
   push NULL;
   pop B1;
   push NULL;
   pop B5;
 #endif
   rts;

.ENDMODULE;
