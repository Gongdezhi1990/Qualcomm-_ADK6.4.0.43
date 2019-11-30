// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************


#include "core_library.h"
#include "math_library.h"

#ifdef KYMERA
#include "pmalloc/pl_malloc_preference.h"
#else
#define MALLOC_PREFERENCE_DM1 1
#define MALLOC_PREFERENCE_DM2 2
#endif

.MODULE $math.fft_twiddle;
   .DATASEGMENT DM;

   .VAR twiddle_size =  0;
   .VAR twiddle_user =  0;

   .CONST SUCCESS       1;
   .CONST FAIL          0;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $math.fft_twiddle.alloc
//
// DESCRIPTION:
//    Allocates and populates memory for fft twiddle factors. If the factors are
//    already allocated just register interest.
//
// INPUTS:
//    - r0 = num fft points required
//
// OUTPUTS:
//    - r0 = result (1 - SUCCESS, 0 - FAIL)
//
// TRASHED REGISTERS:
//    Follows C Guildelines
//
// *****************************************************************************
.MODULE $M.math.fft_twiddle.alloc;
   .CODESEGMENT PM_FLASH;

$math.fft_twiddle.alloc:

   push rLink;   
   pushm <r4,r5,r6>;
   pushm <I0,I1,I4,M0,M1>;
   push r0; // fft_size

#if defined(PATCH_LIBS)
   // patch point trashes r0 and r3
   LIBS_SLOW_SW_ROM_PATCH_POINT($math.FFT_TWIDDLE.ALLOC.PATCH_ID_0, r3)
#endif

/* Can not block interupts as that may affect the load operation.
   Realistically, this function is not re-entrant.   It will be called
   by an operator's create or start method which are always sequential
*/

   
//   call $block_interrupts;
   
   r0 = M[SP - 1*ADDR_PER_WORD];  // fft_size
   // current table suffice?
   Null = r0 - M[&$math.fft_twiddle.twiddle_size];
   if LE jump alloc_ok;

   // Now, larger table is needed ...
   // free existing table
   Null = M[&$math.fft_twiddle.twiddle_size];
   if NZ call $math.fft_twiddle.free;

   // require more data than maximum size in ROM?
   r0 = M[SP - 1*ADDR_PER_WORD];  // fft_size
   Null = r0 - $fft.NUM_POINTS;
   if GT jump out_of_rom;

   // register new size
   M[&$math.fft_twiddle.twiddle_size] = r0;

   // r4 = amount to alloc in each DM (real/imag)
   r4 = r0 LSHIFT -1;

   // allocate new memory for real twiddle factors
   r0 = r4;
   Words2Addr(r0);    // arch4: allocation size is in bytes
   r1 = MALLOC_PREFERENCE_DM1;
   call $_xzppmalloc;
   r5 = r0;
   if Z jump out_of_ram1;

   // allocate new memory for imaginary twiddle factors
   r0 = r4;
   Words2Addr(r0);    // arch4: allocation size is in bytes
   r1 = MALLOC_PREFERENCE_DM2;
   call $_xzppmalloc;
   r6 = r0;
   if Z jump out_of_ram2;

   // some new memory was allcated, use it
   r0 = M[&$math.fft_twiddle.twiddle_size];
   r1 = r5;
   r2 = r6;
   call $math.load_fft_twiddle_factors;

alloc_ok:
   // register new user
   r1 = M[&$math.fft_twiddle.twiddle_user];
   r1 = r1 + 1;
   M[&$math.fft_twiddle.twiddle_user] = r1;

   r0 = $math.fft_twiddle.SUCCESS;
   
fft_alloc_done:
   M[SP - 1*ADDR_PER_WORD]=r0;
//   call $unblock_interrupts;
   pop r0; 
   popm <I0,I1,I4,M0,M1>;
   popm <r4,r5,r6>;
   jump $pop_rLink_and_rts;

out_of_ram2:
   // free real
   r0 = r5;
   call $_pfree;

out_of_ram1:
out_of_rom:
alloc_fail:
#ifdef DEBUG_ON
   call $error;
#endif
   M[&$math.fft_twiddle.twiddle_size] = 0;
   M[&$math.fft_twiddle.twiddle_user] = 0;
   r0 = $math.fft_twiddle.FAIL;
   jump fft_alloc_done;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $math.fft_twiddle.release
//
// DESCRIPTION:
//    Unregisters interest in fft twiddle factors then frees them if no-one else
//    is interested
//
// INPUTS:
//    - r0 = num fft points in use
//
// OUTPUTS:
//    - r0 = result (1 - SUCCESS, 0 - FAIL)
//
// TRASHED REGISTERS:
//    Follows C Guidelines
// *****************************************************************************
.MODULE $M.math.fft_twiddle.release;
   .CODESEGMENT PM_FLASH;

$math.fft_twiddle.release:

   push rLink;
   
    push r0;
    call $block_interrupts;
    pop r0;
   
#if defined(PATCH_LIBS)
   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($math.FFT_TWIDDLE.FREE.PATCH_ID_0)
#endif

   // check size
   Null = r0 - M[&$math.fft_twiddle.twiddle_size];
   if GT jump exit_fail;

   // check user
   r1 = M[&$math.fft_twiddle.twiddle_user];
   if Z jump exit_fail;

   // deregister user
   r1 = M[&$math.fft_twiddle.twiddle_user];
   r1 = r1 - 1;
   M[&$math.fft_twiddle.twiddle_user] = r1;
   if NZ jump exit_ok;

   call $math.fft_twiddle.free;
   M[&$math.fft_twiddle.twiddle_size] = 0;

   exit_ok:
   call $unblock_interrupts;
   r0 = $math.fft_twiddle.SUCCESS;
   jump $pop_rLink_and_rts;

   exit_fail:
#ifdef DEBUG_ON
   call $error;
#endif
   call $unblock_interrupts;
   r0 = $math.fft_twiddle.FAIL;
   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $math.fft_twiddle.free
//
// DESCRIPTION:
//    Free the current twiddle factors memory (real and imag)
//    Note this routine is intended as internal utility.
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Follos C Guidelines
//
// *****************************************************************************
.MODULE $M.math.fft_twiddle.free;
   .CODESEGMENT PM_FLASH;

$math.fft_twiddle.free:
   push rLink;

   // free real
   r0 = M[$fft.twiddle_real_address];
   call $_pfree;
   M[$fft.twiddle_real_address] = 0;

   // free imag
   r0 = M[$fft.twiddle_imag_address] ;
   call $_pfree;
   M[$fft.twiddle_imag_address] = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;
