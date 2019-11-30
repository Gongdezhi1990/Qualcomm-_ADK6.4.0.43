/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup dynamic_mem Dynamic Memory
 * \ingroup mem_utils
 *
 * \file  dynamic_mem_macros.h
 * \ingroup dynamic_mem
 *
 */
#ifndef DYNAMIC_MEMORY_DEFINITION_H
#define DYNAMIC_MEMORY_DEFINITION_H

// Use Segments for ALLOCATION
//    DM,DM1,DM2,etc.

// Use Segments for SCRATCH
//    SM,SM1,SM2,etc.


// Sepecify locations in Root Data Block to hold pointers to array of allocated blocks
#define DYN_ALLOC_ROOT(off)                 ((__LINE__)<<8)|0x20,(off)
#define DYN_ALLOC_SCRATCH_ROOT(off)         ((__LINE__)<<8)|0x10,(off)
// Link into 24-bit flash for linker symbol resolution
// The parameter is then  name of the external allocation array created by the utility
#define DYN_LINKER_RESOLVE(sym)             ((__LINE__)<<8)|0x08,(sym)
// Define a block size to use
#define DYN_BLOCK_SIZE(size)                ((__LINE__)<<8)|0x30,(size)

// Define a block size to use
#define DYN_SCRATCH_BLOCK_SIZE(size)        ((__LINE__)<<8)|0x50,(size)

// Dynamically allocate a bock of data memory
#define DYN_ALLOC_RESIZE(sym,size)          ((__LINE__)<<8)|0x02,(size),(sym)

// Set a value in the Root Data Block
#define DYN_SET_ROOT(offset,value)          ((__LINE__)<<8)|0x05,(offset),(value)

// Set a value in a Dynamically Allocated Data Block
#define DYN_SET_VALUE(offset,value)         ((__LINE__)<<8)|0x06,(offset),(value)

// Resolve External Reference
#define DYN_RESOLVE_EXTERN(offset,sym)  ((__LINE__)<<8)|0x40,(offset),(sym)

// Specify Variant (alternate) Configurations
#define DYN_VARIANT(id)                     ((__LINE__)<<8)|0x07,(id)

// Allocate Named Memory Block
#define DYN_ALLOC_NAMED(name,sym)           ((__LINE__)<<8)|0x04,(name),(sym)

// (Optionally) move the following table off chip
#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT
#define DYN_ALLOC_EXTERNAL_FILESYSTEM()     ((__LINE__)<<8)|0x80,
#else
#define DYN_ALLOC_EXTERNAL_FILESYSTEM()     
#endif

// End of Dynamic Table 
#define DYN_END()                           ((__LINE__)<<8)|0xFF


#endif
