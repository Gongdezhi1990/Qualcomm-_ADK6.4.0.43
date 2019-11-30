/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 *************************************************************************/
/**
 * \file pl_malloc_debug.asm
 * \ingroup pl_malloc
 *
 * \brief Wrappers for debug memory allocation functions
 *
 */

#ifdef PMALLOC_DEBUG

// *****************************************************************************
// MODULE:
//    $_ppmalloc
//
// DESCRIPTION:
//    Wrapper for $_ppmalloc_debug. 
//    See pl_malloc.h for full description.
//
// INPUTS:
//    - r0 Size of allocation requested
//    - r1 Preference
//
// OUTPUTS:
//    - r0 Pointer to allocated memory, or NULL
//    - none
//
// TRASHED REGISTERS: 
//    
//    r10, rMAC, I3, I7
//
// *****************************************************************************
.MODULE $M.pl_malloc_debug.ppmalloc;
.CODESEGMENT PM_FLASH;

$_ppmalloc:
   pushm <r2, r3, rLink>;
   r2 = rLink;
   r3 = 0;
   call $_ppmalloc_debug;
   popm <r2, r3, rLink>;
   rts;  

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_xppmalloc
//
// DESCRIPTION:
//    Wrapper for $_xppmalloc_debug
//    See pl_malloc.h for full description.
//
// INPUTS:
//    - r0 Size of allocation requested
//    - r1 Preference
//
// OUTPUTS:
//    - r0 Pointer to allocated memory, or NULL
//    - none
//
// TRASHED REGISTERS: 
//    
//    r10, rMAC, I3, I7
//
// *****************************************************************************
.MODULE $M.pl_malloc_debug.xppmalloc;
.CODESEGMENT PM_FLASH;

$_xppmalloc:
   pushm <r2, r3, rLink>;
   r2 = rLink;
   r3 = 0;
   call $_xppmalloc_debug;
   popm <r2, r3, rLink>;
   rts;  

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_zppmalloc
//
// DESCRIPTION:
//    Wrapper for $_zppmalloc_debug
//    See pl_malloc.h for full description.
//
// INPUTS:
//    - r0 Size of allocation requested
//    - r1 Preference
//
// OUTPUTS:
//    - r0 Pointer to allocated memory, or NULL
//    - none
//
// TRASHED REGISTERS: 
//    
//    r10, rMAC, I3, I7
//
// *****************************************************************************
.MODULE $M.pl_malloc_debug.zppmalloc;
.CODESEGMENT PM_FLASH;

$_zppmalloc:
   pushm <r2, r3, rLink>;
   r2 = rLink;
   r3 = 0;
   call $_zppmalloc_debug;
   popm <r2, r3, rLink>;
   rts;  

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_xzppmalloc
//
// DESCRIPTION:
//    Wrapper for $_xzppmalloc_debug
//    See pl_malloc.h for full description.
//
// INPUTS:
//    - r0 Size of allocation requested
//    - r1 Preference
//
// OUTPUTS:
//    - r0 Pointer to allocated memory, or NULL
//    - none
//
// TRASHED REGISTERS: 
//    
//    r10, rMAC, I3, I7
//
// *****************************************************************************
.MODULE $M.pl_malloc_debug.xzppmalloc;
.CODESEGMENT PM_FLASH;

$_xzppmalloc:
   pushm <r2, r3, rLink>;
   r2 = rLink;
   r3 = 0;
   call $_xzppmalloc_debug;
   popm <r2, r3, rLink>;
   rts;  

.ENDMODULE;

#endif //PMALLOC_DEBUG
