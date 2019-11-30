/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file const_data.h
 * \ingroup const_data
 *
 * This module handles platform independent constant data access.
 */

#ifndef CONSTANT_DATA_H
#define CONSTANT_DATA_H

#include "types.h"

#if !defined(__GNUC__)
#define DMCONST _Pragma("datasection DMCONST")
#define DMCONST16 _Pragma("datasection DMCONST16")
#define DMCONST_WINDOWED16 _Pragma("datasection DMCONST_WINDOWED16")
#define CONST   _Pragma("datasection CONST")  
#define CONST16 _Pragma("datasection CONST16") 
#else   
#define DMCONST 
#define DMCONST16
#define DMCONST_WINDOWED16
#define CONST
#define CONST16
#endif

/** Constant data memory types (regions)
  * This is a placeholder for defining all possible constant data memory regions.
  * NOTE: make sure these regions are correctly mapped according to the chip's linkscript.
  */
typedef enum {

    /* DMCONST const data regions */
    MEM_TYPE_DMCONST  = 1,
    MEM_TYPE_DMCONST16 = 2,
    MEM_TYPE_DMCONST_WINDOWED16 = 3,

    /* External / Non-DM const data regions */
    MEM_TYPE_CONST = 4,
    MEM_TYPE_CONST16 = 5,

    /* DM internal statically or dynamically allocated, currently unsupported */
    MEM_TYPE_INTERNAL = 0

} const_data_mem_type;

/** Data format encodings (packing types)
  * NOTE: the format should be platform independent */
typedef enum {
    /* To be resolved to the default platform/chip format */
    /* NOTE: if source data width is less than DSP word data will be left aligned */
    FORMAT_DSP_NATIVE = 0,

    /* 16bit data to be right aligned and zero-padded to the DSP native format */
    FORMAT_16BIT_ZERO_PAD = 1,

    /* 16bit data to be right aligned and sign extended to the DSP native format */
    FORMAT_16BIT_SIGN_EXT = 2,

    /* 2 24bit DSP words packed into 3x16bits
     *   NOTES: Currently not valid on DRAM targets, may be added in future
     *          since we have some HW support for unpacking (3x32bit -> 4x24bit words) */
    FORMAT_PACKED16 = 3

} const_data_format;

/** The descriptor structure that holds the information (location and format)
  * about the source constant data */
typedef struct {
    /* The memory type: this must match the region where address points to */
    const_data_mem_type type : 4;

    /* platform specific format; must match the data type pointed to by address */
    const_data_format format : 4;

    /* Address/offset in the memory (platform specific, 32bit on Crescendo, 24bit elsewhere)
       **Note: on Atlas this is a relative offset, not the actual physical address */
    void * address;

#ifdef CHIP_BASE_A7DA_KAS
    /* with capability download support on Atlas7/KAS we may have different DRAM bases,
     * which cannot be resolved at compile-time. So this identifier will be used by 
     * capability download to resolve the const data DRAM base address of the allocated
     * memory block; a base of 0 denotes a global context (non-download context) which will 
     * resolve to dram_const16_base or dram_const32_base. */
    const volatile unsigned base_id;
#ifdef INSTALL_CAP_DOWNLOAD_MGR
    /*
     * Id of the variable referenced in descriptor, like base_id this is internally used and patched
     * by capability download manager build system. This should never be modified in the program
     */
    const volatile unsigned var_id;
#endif
#endif
} const_data_descriptor;

#ifdef CHIP_BASE_A7DA_KAS
#ifdef INSTALL_CAP_DOWNLOAD_MGR

/** This macro defines the descriptor in a platform independent manner */
#define DEFINE_CONST_DATA_DESCRIPTOR(type, format, const_data) { type, format, const_data, 0, 0 }

#else

#define DEFINE_CONST_DATA_DESCRIPTOR(type, format, const_data) { type, format, const_data, 0 }

#endif
#else

#define DEFINE_CONST_DATA_DESCRIPTOR(type, format, const_data) { type, format, const_data }

#endif

/** \brief Copy constant data.
 * Constant data will be copied from the source location to the 'destination'.
 * \param source - the source descriptor containing information about the constant data location and format
 * \param offset - the offset in the source memory where the data read should start from
 * \param destination - the destination memory block pointer, should be at least 'size' memory units long
 * \param size - the size of the data to be transferred
 * Note: the user is responsible for allocation and freeing of the memory pointed by 'destination'.
 */
bool const_data_copy ( const_data_descriptor * source, unsigned offset,
                        void * destination, unsigned size );

/** \brief Access constant data.
 * A pointer to a block of constant data described by 'source' will be returned.
 * \param source - the source descriptor containing information about the constant data location and format
 * \param offset - the offset in the source memory where the data read should start from
 * \param work_buffer - the memory block which will act as a working buffer; if this parameter is NULL a new buffer will be allocated,
          on successive calls this parameter should contain the returned value from the first call to const_data_access. This is required
          due to Kalimba platform differences where locations may or may not be memory mapped (or directly accessible).
 * \param size - the size of the data to be transferred
 */
void * const_data_access ( const_data_descriptor * source, unsigned offset,
                            void * work_buffer, unsigned size );

/** \brief Release (free) memory block allocated by const_data_access function.
 * \param ptr - pointer to the memory as returned by const_data_access.
 */
void const_data_release ( void * ptr );

#endif /* CONSTANT_DATA_H */

