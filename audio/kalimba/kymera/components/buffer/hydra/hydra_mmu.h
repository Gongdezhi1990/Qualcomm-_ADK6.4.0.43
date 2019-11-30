/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file hydra_mmu.h
 *  Memory Management Unit interface.
 */

/* This is currently a cut-down version of the MMU interface from Hydra
 * containing limited definitions to allow submsg etc to build.
 * Some updates are likely to adapt submsg to the Audio subsystem's MMU
 */

#ifndef HYDRA_ADAPTION_MMU_H
#define HYDRA_ADAPTION_MMU_H

#include "hydra.h"

/**
 * Conversion from buffer size code (mmu_buffer_size) to number of pages
 * needed to map it. This is exposed for declaring static buffers (used for
 * sizing the page list).
 *
 * \internal    The buffer size code is defined to be log2(nr pages needed).
 */
#define MMU_PAGE_TABLE_PAGES(size_code) (1U << ((uint16) (size_code)))

#define MMU_PAGE_BYTES 64

/**
 * MMU Buffer Handle Index.
 *
 * Subsystem relative handle index. This type should not be interfaced directly
 * by MMU clients. See mmu_handle instead.
 *
 * \sa  mmu_handle
 */
typedef uint8 mmu_index;

/**
 * MMU Buffer Access Offset.
 *
 * The units are _always_ octets.
 *
 */
typedef uint24 mmu_offset;

/**
 * MMU Buffer Handle.
 *
 * This is a logical type it is unlikely that it matches hardware
 * handle register format.
 *
 * Implementors of devices with MMU ports should use
 * mmu_port_pack_buffer_handle() & mmu_port_unpack_buffer_handle() to
 * convert logical mmu_handles to/from MMU hardware register format if
 * necessary. This will isolate them from any changes to the logical
 * structure.
 *
 * \port    Singular (non Hydra) systems do not need the ssid field.
 */
typedef struct mmu_handle {
   hydra_ssid  ssid:8;        /**< \private Subsystem ID    */
   mmu_index   index:8;       /**< \private Index           */
   unsigned    flags:8;       /**< \private Flags           */
} mmu_handle;

/**
 * Reserved Buffer handle value to indicate "null"/"no handle".
 *
 * \public \memberof mmu_handle
 *
 * This is logically a const but to help debug/diagnostic it is initialised
 * with local ssid.
 */
extern mmu_handle MMU_HANDLE_NULL;

/* Buffer size enumeration
 * Other subsystems get this directly from their register definitions
 * We need to define it here, because audio has 'special' MMU hardware
 */
typedef enum mmu_buffer_size_enum
{
   MMU_BUFFER_SIZE_64                       = (int)0x0,
   MMU_BUFFER_SIZE_128                      = (int)0x1,
   MMU_BUFFER_SIZE_256                      = (int)0x2,
   MMU_BUFFER_SIZE_512                      = (int)0x3,
   MMU_BUFFER_SIZE_1024                     = (int)0x4,
   MMU_BUFFER_SIZE_2048                     = (int)0x5,
   MMU_BUFFER_SIZE_4096                     = (int)0x6,
   MMU_BUFFER_SIZE_8192                     = (int)0x7,
   MMU_BUFFER_SIZE_16384                    = (int)0x8,
   MMU_BUFFER_SIZE_32K                      = (int)0x9,
   MAX_MMU_BUFFER_SIZE                      = (int)0x9
} mmu_buffer_size;
#define NUM_MMU_BUFFER_SIZE (0xA)

/**
 * Convert MMU port buffer register value to logical mmu_handle equivalent.
 *
 * This is published in mmu.h as some MMU port-specific driver's need this
 * and there isn't a dedicated header for them.
 *
 * \todo    implement in terms of h/w field defs when available.
 */
extern void mmu_port_unpack_buffer_handle(
   /** [in] mmu port buffer register value */
   uint16 val,
   /** [out] Logical MMU handle */
   mmu_handle *h
);


/**
 * Reserved MMU Handle Index value to indicate "null"/"no handle".
 *
 * Do not test against directly. Use mmu_index_is_null() instead.
 *
 * \sa  mmu_index_is_null()
 */
#define MMU_INDEX_NULL       0

/**
 * Is this the NULL MMU Handle index value?
 */
#define mmu_index_is_null(ix) ( (mmu_index)MMU_INDEX_NULL == (ix) )


/**
 * Reserved Buffer handle value to indicate "null"/"no handle".
 *
 * \public \memberof mmu_handle
 *
 * \deprecated  See MMU_HANDLE_NULL.
 */
#define MMU_NO_HANDLE MMU_HANDLE_NULL

/* Flag bit for buffer in DM2 */
#define MMU_BUFF_DM2 1

/**
 * Initialise this MMU Buffer Handle.
 *
 * \public \memberof mmu_handle
 *
 * Should only be called by factory functions (e.g. mmu_create_buffer()).
 */
extern void mmu_handle_init(mmu_handle *h, hydra_ssid ssid, mmu_index index);

#define mmu_handle_init(h, a_ssid, a_index)  \
((void)( \
    (h)->ssid = (a_ssid) , \
    (h)->index = (a_index) \
))

/**
 * Get the Subsystem ID from this MMU Buffer handle.
 * \public \memberof mmu_handle
 */
extern hydra_ssid mmu_handle_ssid(mmu_handle h);

#define mmu_handle_ssid(h)      ( (h).ssid )

/**
 * Set the Subsystem ID in this MMU buffer handle.
 * \public \memberof mmu_handle
 */

extern void set_mmu_handle_ssid(mmu_handle h, hydra_ssid id);

#define set_mmu_handle_ssid(h,i)    ( (h).ssid = (i) )

/**
 * Get the Buffer Index from this MMU Buffer handle.
 * \public \memberof mmu_handle
 */
extern mmu_index mmu_handle_index(mmu_handle h);

#define mmu_handle_index(h)     ( (h).index )

/**
 * Set the Buffer Index in this MMU buffer handle.
 * \public \memberof mmu_handle
 */

extern void set_mmu_handle_index(mmu_handle h, mmu_index i);

#define set_mmu_handle_index(h,i)    ( (h).index = (i) )

/**
 * Are the two MMU Handles equivalent?
 * \public \memberof mmu_handle
 *
 * This should not be used to test for a null handle. Use mmu_handle_is_null()
 * for that.
 */
extern bool mmu_handle_is_equal(mmu_handle h1, mmu_handle h2);

#define mmu_handle_is_equal(h1, h2) \
    ((mmu_handle_index(h1) == mmu_handle_index(h2)) && \
     (mmu_handle_ssid(h1) == mmu_handle_ssid(h2)))

/**
 * Is this the NULL MMU Buffer Handle value?
 * \public \memberof mmu_handle
 *
 * \internal    If index is null then handle is null.
 */
extern bool mmu_handle_is_null(mmu_handle h);

#define mmu_handle_is_null(h)   mmu_index_is_null(mmu_handle_index(h))

/**
 * Is this a local MMU Buffer Handle value?
 * \public \memberof mmu_handle
 */
extern bool mmu_handle_is_local(mmu_handle h);

#ifdef CHIP_BASE_HYDRA
#define mmu_handle_is_local(h)  hydra_ssid_is_local( mmu_handle_ssid(h) )
#else
#define mmu_handle_is_local(h)  (TRUE)
#endif /* CHIP_BASE_HYDRA */

extern uint16 mmu_port_pack_buffer_handle(
   /** [in] Logical MMU handle to pack into mmu port buffer register format */
   const mmu_handle *h
);

#define mmu_port_pack_buffer_handle(h) \
    (uint16)((mmu_handle_ssid(h) << 8) | mmu_handle_index(h))

extern void mmu_port_unpack_buffer_handle(
   /** [in] mmu port buffer register value */
   uint16 val,
   /** [out] Logical MMU handle */
   mmu_handle *h
);

#define mmu_port_unpack_buffer_handle(val, h) \
    mmu_handle_init((h), (system_bus)(val >> 8) , (mmu_index)(val & 0xFF))

#ifdef BAC32
typedef enum
{
    SAMPLE_SIZE_8BIT = 0,
    SAMPLE_SIZE_16BIT,
    SAMPLE_SIZE_24BIT,
    SAMPLE_SIZE_32BIT,
} sample_size;
#endif

#endif /* HYDRA_ADAPTION_MMU_H */
