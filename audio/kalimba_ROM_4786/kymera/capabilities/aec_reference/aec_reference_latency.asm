// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change: 1141152 $  $DateTime: 2011/11/02 20:31:09 $
// *****************************************************************************

// *****************************************************************************
// NAME:
//    AEC reference latency
//
// DESCRIPTION:
//    Implementing the AEC latency sync logic
//
//
// *****************************************************************************

#include "stack.h"
#include "cbops/cbops.h"
#include "cbuffer_asm.h"
#include "aec_reference_latency_asm_defs.h"

#ifdef PATCH_LIBS
#include "patch/patch_asm_macros.h"
#endif

.MODULE $M.cbops.aec_ref_latency;
   .CODESEGMENT PM;
   .DATASEGMENT DM;
   
   // ** function vector **
   .VAR $cbops.aec_ref_mic_latency_op[$cbops.function_vector.STRUC_SIZE] =
			&$cbops.function_vector.NO_FUNCTION,            // reset vector
      &$cbops.aec_ref_latency_mic.amount_to_use,      // amount to use function
      &$cbops.aec_ref_latency_mic.main;               // main function
      
    .VAR $cbops.aec_speaker_latency_op[$cbops.function_vector.STRUC_SIZE] =
			&$cbops.function_vector.NO_FUNCTION,            // reset vector
      &$cbops.aec_ref_latency_speaker.amount_to_use,  // amount to use function
      &$cbops.aec_ref_latency_speaker.main;           // main function

// Expose the location of this table to C
.set $_cbops_speaker_latency_table , $cbops.aec_speaker_latency_op
.set $_cbops_mic_latency_table , $cbops.aec_ref_mic_latency_op


// *****************************************************************************
// MODULE:
//   $cbops.aec_ref_latency_mic.amount_to_use
//
// DESCRIPTION:
//   Get the amount to use, across all channels (it acts "in sync").
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    r0,r5
//
// *****************************************************************************

$cbops.aec_ref_latency_mic.amount_to_use:
#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.aec_ref_latency_mic.amount_to_use.PATCH_ID_0, r5)
#endif

   // Get I/O row entry for index (r5)
   r5 = M[r8 + $aec_reference_latency.latency_op_struct.INDEX_FIELD];
   r5 = r5 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR(int);
   r5 = r5 + r4;
   // Save transfer (space in buffer)
   r0 = M[r5 + $cbops_c.cbops_buffer_struct.TRANSFER_PTR_FIELD];
   r0 = M[r0];
   M[r8 + $aec_reference_latency.latency_op_struct.AVAILABLE_FIELD] = r0;
   rts;

// *****************************************************************************
// MODULE:
//   $cbops.aec_ref_latency_mic.main
//
// DESCRIPTION:
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    r0-r7
//
// *****************************************************************************
$cbops.aec_ref_latency_mic.main:
#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.aec_ref_latency_mic.main.PATCH_ID_0, r6)
#endif

   // pointer to common (r6)
   r6 = M[r8 + $aec_reference_latency.latency_op_struct.COMMON_FIELD];
   // Get I/O row entry for index (r5)
   r5 = M[r8 + $aec_reference_latency.latency_op_struct.INDEX_FIELD];
   r5 = r5 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR(int);
   r5 = r5 + r4;
   // get transfer amount (r0) and pointer (r7)
   r7 = M[r5 + $cbops_c.cbops_buffer_struct.TRANSFER_PTR_FIELD];
   r0 = M[r7];
   // Purge transfer >= jitter
   r2 = M[r6 + $aec_reference_latency.aec_latency_common_struct.JITTER_FIELD];
   r1 = M[r6 + $aec_reference_latency.aec_latency_common_struct.MIC_DATA_FIELD];

   // Advance mic_data (r1)
   r1 = r1 + r0;

   // if mic_data >= block_size
   r2 = M[r6 + $aec_reference_latency.aec_latency_common_struct.BLOCK_SIZE_FIELD];
   Null = r1 - r2;
   if NEG jump cbops.aec_ref_latency_mic.main.done;
	   // have a block, calc data in buffer = (size - space - 1) 
	   r4 = M[r5 + $cbops_c.cbops_buffer_struct.SIZE_FIELD];
	   Addr2Words(r4);
	   r3 = M[r8 + $aec_reference_latency.latency_op_struct.AVAILABLE_FIELD];   
	   r4 = r4 - r3;
	   r4 = r4 - 1;
	   // add transfer , subtract block
	   r4 = r4 + r0;
	   r1 = r4 - r2;
	   if NEG jump cbops.aec_ref_latency_mic.main.error;    
	   
     // have verified good block, signal reference
	   M[r6 + $aec_reference_latency.aec_latency_common_struct.BLOCK_SYNC_FIELD] = r6;
	   
	   // Verify Block size is valid
	   NULL = r2 - 1;
     if LE jump cbops.aec_ref_latency_mic.main.done;
	   
	   // mic_data (r1) > jitter (r4), drop data
	   r4 = M[r6 + $aec_reference_latency.aec_latency_common_struct.JITTER_FIELD];
	   NULL = r1 - r4;   
	   if LE jump cbops.aec_ref_latency_mic.main.done;
     // drop as much as possible, MIN(mic_data,transfer)
     r4 = r1;
	   r4 = MIN r0;
	   r0 = r0 - r4;
	   r1 = r1 - r4;
     // Update Transfer, and record drop
	   M[r7] = r0;		  	   	
cbops.aec_ref_latency_mic.main.done:  
   // Update Mic Data
   M[r6 + $aec_reference_latency.aec_latency_common_struct.MIC_DATA_FIELD] = r1;
   rts;
   
cbops.aec_ref_latency_mic.main.error:    
   // Buffer out of sync with mic_data.    Should never happen
   M[r6 + $aec_reference_latency.aec_latency_common_struct.MIC_DATA_FIELD] = r4; 
   rts;

// *****************************************************************************
// MODULE:
//   $cbops.aec_ref_latency_speaker.amount_to_use
//
// DESCRIPTION:
//   Get the amount to use, across all channels (it acts "in sync").
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    r0-r3, r5, r6
//
// *****************************************************************************   
$cbops.aec_ref_latency_speaker.amount_to_use:
#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.aec_ref_latency_speaker.amount_to_use.PATCH_ID_0, r5)
#endif

   // Get I/O row entry for index (r5)
   r5 = M[r8 + $aec_reference_latency.latency_op_struct.INDEX_FIELD];
   r5 = r5 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR(int);
   r5 = r5 + r4;
   // Get transfer (r1) and ptr (r0) 
   r0 = M[r5 + $cbops_c.cbops_buffer_struct.TRANSFER_PTR_FIELD];
   r1 = M[r0];
   // ensure space (r1) is >= jitter (r6) to prevent stall
   r6 = M[r8 + $aec_reference_latency.latency_op_struct.COMMON_FIELD];
   r2 = M[r6 + $aec_reference_latency.aec_latency_common_struct.JITTER_FIELD];
   r3 = r1 - r2;
   if POS jump cbops.aec_ref_latency_speaker.amount_to_use.done;
   		M[r0]=r2;
   		
#ifdef AEC_REFERENCE_LATENCY_DEBUG   
     r0 = M[r6 + $aec_reference_latency.aec_latency_common_struct.SPEAKER_DROPS_FIELD];
     r0 = r0 - r3;
     M[r6 + $aec_reference_latency.aec_latency_common_struct.SPEAKER_DROPS_FIELD] = r0;
#endif  

   		Words2Addr(r3);
   		M1 = r3;
   		// Get Buffer
      r0 = M[r5 + $cbops_c.cbops_buffer_struct.RW_PTR_FIELD];
      I0 = r0;
      r0 = M[r5 + $cbops_c.cbops_buffer_struct.SIZE_FIELD];
      L0 = r0;
      r2 = M[r5 + $cbops_c.cbops_buffer_struct.BASE_FIELD];
      push r2;
      pop B0;
      // Back up buffer
      r0 = M[I0,M1];
      // Update Buffer
      r0 = I0;
      M[r5 + $cbops_c.cbops_buffer_struct.RW_PTR_FIELD]=r0;
      
      // restore circular
      L0 = 0;
      push NULL;
      pop B0;
      
 	 cbops.aec_ref_latency_speaker.amount_to_use.done:
 	 
   M[r8 + $aec_reference_latency.latency_op_struct.AVAILABLE_FIELD] = r1;
      
   rts;

// *****************************************************************************
// MODULE:
//   $cbops.aec_ref_latency_speaker.main
//
// DESCRIPTION:
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    r0-r7,r9,r10
//
// *****************************************************************************   
/* Need to ensure that the reference data precedes the mic_data
 * by between delay and (delay+jitter).   This ensures causality
 * while limitting the excess delay that would be need to be addressed
 * in the echo canceller tail length
*/
$cbops.aec_ref_latency_speaker.main:
#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.aec_ref_latency_speaker.main.PATCH_ID_0, r5)
#endif

   // Get I/O row entry for index (r5)
   r5 = M[r8 + $aec_reference_latency.latency_op_struct.INDEX_FIELD];
   r5 = r5 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR(int);
   r5 = r5 + r4;
   // get transfer amount (r0) and pointer (r7)
   r7 = M[r5 + $cbops_c.cbops_buffer_struct.TRANSFER_PTR_FIELD];
   r0 = M[r7];
   if Z rts;
   // pointer to common (r6)
   r6 = M[r8 + $aec_reference_latency.latency_op_struct.COMMON_FIELD];

   // get frame size (r4).  If <=1 then disable latency control
   r4 = M[r6 + $aec_reference_latency.aec_latency_common_struct.BLOCK_SIZE_FIELD];
   NULL = r4 - 1;
   if LE rts;
   
   // advance speaker_data (r1)
   r1 = M[r6 + $aec_reference_latency.aec_latency_common_struct.SPEAKER_DATA_FIELD];   
   r1 = r1 + r0;
      
   // remainder (r3) = Delay + mic_data
   r2 = M[r6 + $aec_reference_latency.aec_latency_common_struct.REF_DELAY_FIELD]; 
   r3 = M[r6 + $aec_reference_latency.aec_latency_common_struct.MIC_DATA_FIELD]; 
   r3 = r3 + r2;
   r9 = M[r6 + $aec_reference_latency.aec_latency_common_struct.JITTER_FIELD];
   
 
   // Did mic report a frame of data
   NULL = M[r6 + $aec_reference_latency.aec_latency_common_struct.BLOCK_SYNC_FIELD];
   if Z jump cbops.aec_ref_latency_speaker.main.drop; 	 
      // Increment Frame count
      r1 = M[r6 + $aec_reference_latency.aec_latency_common_struct.FRM_COUNT_FIELD];
      r2 = r1 + 1;
      if C r2=r1;
      M[r6 + $aec_reference_latency.aec_latency_common_struct.FRM_COUNT_FIELD]=r2;

      // Get data actually in Buffer (r1) = (size - space - 1)
      r1 = M[r5 + $cbops_c.cbops_buffer_struct.SIZE_FIELD];
      Addr2Words(r1);
      r2 = M[r8 + $aec_reference_latency.latency_op_struct.AVAILABLE_FIELD];   
      r1 = r1 - r2;
      r1 = r1 - 1;
      
      // add transfer (r0) 
	    r1 = r1 + r0;

      // limit (r2) = block_size (r4) + delay (r3)
      r2 = r3 + r4;
      
      // if speaker_data < limit , Insert
      Null = r1 - r2;
      if NEG jump cbops.aec_ref_latency_speaker.main.insert;

	   // speaker_data (r1) = speaker_data - block_size
      r1 = r1 - r4;

 cbops.aec_ref_latency_speaker.main.drop:	
   
   // r4 = speaker_data - remainder, r0=transfer, r1=speaker_data
   r4 = r1 - r3;
   // if (speaker_data - remainder) >= jitter, drop data	 
   NULL = r4 - r9;   
   if NEG jump cbops.aec_ref_latency_speaker.main.done;	
   // Target speaker_data = mic_data + delay + jitter/2
   r2 = r9 ASHIFT -1;
   r3 = r3 + r2;
   r3 = r1 - r3;
   r3 = MIN r0;   		
   // drop speaker_data (r1) and transfer (r0) by MIN(transfer, r3)
   r0 = r0 - r3;
   r1 = r1 - r3;	   
   M[r7] = r0;

#ifdef AEC_REFERENCE_LATENCY_DEBUG   
   r0 = M[r6 + $aec_reference_latency.aec_latency_common_struct.SPEAKER_DROPS_FIELD];
   r0 = r0 + r3;
   M[r6 + $aec_reference_latency.aec_latency_common_struct.SPEAKER_DROPS_FIELD] = r0;
#endif     


cbops.aec_ref_latency_speaker.main.rm_update:
   // r1=speaker count, r3 = drop/insert, (<0 for inserts)

   // Compute demominator (frm_count * frm_size).  Reset frame counter
   r2 = M[r6 + $aec_reference_latency.aec_latency_common_struct.BLOCK_SIZE_FIELD];
   r4 = M[r6 + $aec_reference_latency.aec_latency_common_struct.FRM_COUNT_FIELD];
   M[r6 + $aec_reference_latency.aec_latency_common_struct.FRM_COUNT_FIELD]=NULL;
   rMAC = r4*r2;     // Note: frac mult is x2
   r4 = SIGNDET rMAC;
   rMAC = rMAC ASHIFT r4 (56bit);
   r2 = rMAC;
   
   // Compute numerator (drift)
   r0 = SIGNDET r3;
   r0 = r0 - 2;
   rMAC = r3 ASHIFT r0;

   // drift  / (frm_count * frm_size)
   Div = rMAC / r2; 

   // Get and clear rate adjustment
   r7 = M[r6 + $aec_reference_latency.aec_latency_common_struct.RM_ADJUSTMENT_FIELD];
   M[r6 + $aec_reference_latency.aec_latency_common_struct.RM_ADJUSTMENT_FIELD]=NULL;

   // Compute exponent
   r4 = r4 - DAWTH;
   r4 = r4 - r0;
   
   // Validate result (exp < -9 )
   NULL = r4 + 9;
   if POS jump cbops.aec_ref_latency_speaker.main.done;
      // Update rate adjustment
      rMAC = DivResult; 
      rMAC = rMAC ASHIFT r4 (56bit);
      r7 = r7 - rMAC;
      M[r6 + $aec_reference_latency.aec_latency_common_struct.RM_ADJUSTMENT_FIELD]=r7;

cbops.aec_ref_latency_speaker.main.done: 
   // Update speaker_data and clear signal from MIC
   M[r6 + $aec_reference_latency.aec_latency_common_struct.SPEAKER_DATA_FIELD] = r1;
   
#ifdef AEC_REFERENCE_LATENCY_DEBUG 
   r2 = M[r6 + $aec_reference_latency.aec_latency_common_struct.MIC_DATA_FIELD]; 
   r1 = r1 - r2;
   M[r6 + $aec_reference_latency.aec_latency_common_struct.SPEAKER_DELAY_FIELD] = r1; 
#endif 
   
   rts;   
   
cbops.aec_ref_latency_speaker.main.insert:     
   // r1=speaker_data, r0=transfer, r2 = block_size + rmainder, r3=rmainder   
     
   // Advance to insertion point (M1)
   r10 = r0*MK1 (int);
   M1  = r10;
  
   // Compute insertion (r10) and update transfer and remainder
   r4 = r9 ASHIFT -1;
  
   r2  = r2 + r4;
   r10 = r2 - r1;		
   r0  = r0 + r10;
   M[r7] = r0;
   r1 = r3 + r4;

#ifdef AEC_REFERENCE_LATENCY_DEBUG   
   r0 = M[r6 + $aec_reference_latency.aec_latency_common_struct.SPEAKER_INSERTS_FIELD];
   r0 = r0 + r10;
   M[r6 + $aec_reference_latency.aec_latency_common_struct.SPEAKER_INSERTS_FIELD] = r0;
#endif  

   // Number of insertions (negated)
   r3 = NULL - r10;

   // Get Buffer
   r0 = M[r5 + $cbops_c.cbops_buffer_struct.RW_PTR_FIELD];
   I0 = r0;
   r0 = M[r5 + $cbops_c.cbops_buffer_struct.SIZE_FIELD];
   L0 = r0;
   r2 = M[r5 + $cbops_c.cbops_buffer_struct.BASE_FIELD];
   push r2;
   pop B0;
  
   // Advance to insertion point
   r2 = M[I0,M1]; 

   // Insert zeros
   r0 = 0; 
   do cbops.aec_ref_latency_speaker.main.insert_loop;
      M[I0,MK1] = r0;
   cbops.aec_ref_latency_speaker.main.insert_loop:
   
   // clear cicular buffer
   L0 = 0;
   push Null;
   pop B0;
   
   // speaker_data (r1) = remainder
   jump cbops.aec_ref_latency_speaker.main.rm_update;
   
.ENDMODULE;


// *****************************************************************************
// MODULE:
//   $_aec_ref_purge_mics
//
// DESCRIPTION:
//
// INPUTS:
//    - r0 = pointer to cbops_graph to process
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//
// *****************************************************************************
.MODULE $M.cbops.aec_ref_purge_mics;
   .CODESEGMENT PM;

// void aec_ref_purge_mics(cbops_graph *mic_graph,unsigned num_mics); 
$_aec_ref_purge_mics:
  // Save registers
	push rLink;
	pushm <r5,r6,r7,r8,r10>;
	pushm <M0,L0>;
	push I0;
	push B0;
	
  // Force cbops to update buffers 
  M[r0 + $cbops_c.cbops_graph_struct.FORCE_UPDATE_FIELD]=r0;
  
	// Extra parameters 
	r7 = r0 + $cbops_c.cbops_graph_struct.BUFFERS_FIELD;
	r8 = r1;
	r6 = MAXINT;
	
	// Get minimum data in mics
	r10 = r8; 
	r5  = r7;
	do aec_ref_purge_mics.data_loop;
		// Get amount of data in mic
		r0 = M[r5 + $cbops_c.cbops_buffer_struct.BUFFER_FIELD];
		call $cbuffer.calc_amount_data_in_words;
		// update minimum
		r6 = MIN r0;
		// Go to next mic
		r5 = r5 + $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR;
	aec_ref_purge_mics.data_loop:

  r0 = r6;
	Words2Addr(r0);
	M0 = r0;
	if Z jump aec_ref_purge_mics.advance_loop;
	
	// Read Data
	r5  = r7;
aec_ref_purge_mics.advance_loop:		
		// Get Input buffer (source)
		r0 = M[r5 + $cbops_c.cbops_buffer_struct.BUFFER_FIELD];
    call $cbuffer.get_read_address_and_size_and_start_address;
		L0 = r1;
		push r2;
		pop B0;
	  I0 = r0;
		// Advance buffer
		NULL = r1 - MK1;
		if NZ jump aec_ref_purge_mics.sw_buf;
			// MMU buffer must be read			
			r10 = r6; 
			do aec_ref_purge_mics.mmu_buf;
				r0 = M[I0,MK1];
			aec_ref_purge_mics.mmu_buf:
			
			jump aec_ref_purge_mics.next;
			
aec_ref_purge_mics.sw_buf:				
			r0 = M[I0,M0];				  	  
aec_ref_purge_mics.next:		
		// Update buffer
		r0 = M[r5 + $cbops_c.cbops_buffer_struct.BUFFER_FIELD];
		r1 = I0;
		call $cbuffer.set_read_address;
		// Go to next mic
		r5 = r5 + $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR;
		r8 = r8 - 1;
		if GT jump aec_ref_purge_mics.advance_loop;
	
  // Restore registers
	pop B0;
	pop I0;
	popm <M0,L0>;
	popm <r5,r6,r7,r8,r10>;
  pop rLink;
	rts;   
   

.ENDMODULE;

// *****************************************************************************
// MODULE:
//   $$_aecref_calc_ref_rate
//   int aecref_calc_ref_rate(unsigned mic_rt,int mic_ra,unsigned spkr_rt,int spkr_ra);
//
// DESCRIPTION:
//    Compute reference rate adjustment from MIC & SPKR
//
//  rate_adjust = (rate_sink / rate_src) – 1.0				> 0 to speed up source
//
//  RateMIC = Expected/accumulated (QN.22)	(source)     	>1.0 if slower than expected
//  RateOUT = RateMIC x ( RateAdjMIC + 1.0)		(sink)
//
//  RateSPKR = Expected/accumulated (QN.22)	(sink)	             >1.0 if slower than expected
//  RateIN   = RateSPKR / (RateAdjSPKR + 1.0)		(source)
//
//  RateOUT =  RateIN  x (RateAdjREF + 1.0)
//
//  RateAdjREF = (RateOUT / RateIN) – 1.0
//  RateAdjREF = ([RateMIC x ( RateAdjMIC + 1.0)] / [RateSPKR / (RateAdjSPKR + 1.0)]) – 1.0
//  RateAdjREF = ([( RateAdjMIC + 1.0) x (RateAdjSPKR + 1.0)] x RateMIC / RateSPKR) – 1.0
//  RateAdjREF = [[( RateAdjMIC + 1.0) x (RateAdjSPKR + 1.0) x RateMIC ] / RateSPKR ] – 1.0
//
// INPUTS:
//    - r0 = MIC rate measurement (QN.22)
//    - r1 = MIC rate adjustment   (-1.0 ... 1.0)
//    - r2 = SPKR rate measurement (QN.22)
//    - r3 = SPKR rate adjustment  (-1.0 ... 1.0)
//
// OUTPUTS:
//    - r0 = reference rate adjustment
//
// TRASHED REGISTERS:
//
// *****************************************************************************
.MODULE $M.aecref_calc_ref_rate;
   .CODESEGMENT PM;
$_aecref_calc_ref_rate:
    push r4;
    r4 = 0.5;
    // RateAdjREF = ([( mic_ra + 1.0) x (spkr_ra + 1.0) x mic_rt ] / spkr_rt) - 1.0

    // (mic_ra+1) x (spkr_ra+1.0) x 0.25 =
    //       (mic_ra*0.5 + 0.5) x (spkr_ra*0.5 + 0.5)
    r1 = r1 ASHIFT -1;
    r1 = r1 + r4;
    r3 = r3 ASHIFT -1;
    r3 = r3 + r4;
    rMAC = r1*r3;
  
    // (mic_ra+1) x (spkr_ra+1.0) x mic_rt x 0.25 
    rMAC = rMAC * r0;

    //  Note:  Multiplier is 0.25 instead of 0.5 because it is going
    //         into a fractional divide

    // [(mic_ra+1) x (spkr_ra+1.0) x mic_rt x 0.25]/spkr_rt 
    DIV = rMAC / r2; 
    r0 = DivResult;

    // Result of the divide is [0.0 ... 1.0], 0.5 is unity
    //   Convert to [-1.0 ... +1.0] 
    r0 = r0 - r4;
    r0 = r0 ASHIFT 1;
    pop r4;
    rts;

.ENDMODULE;
