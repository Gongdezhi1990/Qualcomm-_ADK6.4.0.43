// Copyright (c) 2016 Qualcomm Technologies International, Ltd.
//   %%version
// FILE
//   pmalloc_trace_pc.asm  -  wrappers for pmalloc to trace return addresses
//
// CONTAINS
//   pmalloc_trace_pc  -  allocate memory and record program counter of owner
//   xpmalloc_trace_pc  -  allocate memory and record program counter of owner
//
// DESCRIPTION
//   Wrapper functions for the pmalloc collection of memory allocation
//   functions that supply the least significant word (16 bits) of the return
//   address as the "owner" parameter of the pmalloc_trace function. The least
//   significant bit is set to indicate the processor on which the code is
//   running (MAC=0, PHY=1).
//
//   Both sets of functions have the same prototypes:
//
//       void *Xpmalloc_trace_pc(size_t size);
//       void *Xpmalloc_trace(size_t size, uint16 owner);
//
//   These functions are a memory (constant data) efficient replacement
//   for the use of literal strings by PMALLOC_TRACE_OWNER.
//
#ifdef PMALLOC_TRACE_OWNER_PC_ONLY
.MODULE $M.pmalloc_trace_pc;
    .CODESEGMENT PM_PMALLOC_TRACE;
    .MINIM;

    $_pmalloc_trace_pc:
    // Supply the LSW of the return address as the second function
    // parameter
    r1 = rLink;
    jump $_pmalloc_trace;

    $_xpmalloc_trace_pc:
    r1 = rLink;
    jump $_xpmalloc_trace;

    $_xzpmalloc_trace_pc:
    r1 = rLink;
    jump $_xzpmalloc_trace;

    $_zpmalloc_trace_pc:
    r1 = rLink;
    jump $_zpmalloc_trace;

.ENDMODULE;
#endif /* PMALLOC_TRACE_OWNER_PC_ONLY */
