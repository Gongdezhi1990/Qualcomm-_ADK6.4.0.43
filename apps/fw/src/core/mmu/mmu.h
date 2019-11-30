/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 *  Memory Management Unit interface.
 */

#ifndef MMU_H
#define MMU_H

/*
 * Interface dependencies.
 */

/*lint -e750 -e962*/ #define IO_DEFS_MODULE_APPS_SYS_CPU1_VM

/*lint -e750 -e962*/ #define IO_DEFS_MODULE_APPS_SYS_CPU0_VM /* both processor for mmu_buffer_size */
/*lint -e750 -e962*/ #define IO_DEFS_MODULE_APPS_SYS_CPU_MEMORY_MAP

#include "mmu/memmap.h"
#include "hydra/hydra.h"    /* for hydra_ssid_is_local() */
#include "bits.h"   /* for BIT16() */

#include "io/io_defs.h"
/*
 * Public Interface.
 */

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
 * Reserved MMU Handle Index value to indicate "null"/"no handle".
 *
 * Do not test against directly. Use mmu_index_is_null() instead.
 *
 * \sa  mmu_index_is_null()
 */
#define MMU_INDEX_NULL       0U

#ifdef SUBSYSTEM_APPS
/**
 * Reserved MMU handle index value for use as the standard P0-to-P1 MMU for IPC
 */
#define MMU_INDEX_RESERVED_IPC 1U
#endif

/**
 * Is this the NULL MMU Handle index value?
 */
#define mmu_index_is_null(ix) ( (mmu_index)MMU_INDEX_NULL == (ix) )

/**
 * MMU Buffer Handle.
 *
 * To avoid unnecessary processing make this matches the current
 * hardware handle register format.
 *
 * This register consists of two 8bit fields:
 *  Subsystem ID and Handle index
 *
 * \note    Singular (non Hydra) systems do not need the ssid field (portability).
 */
typedef uint16 mmu_handle;

/**
 * Reserved Buffer handle value to indicate "null"/"no handle".
 *
 * public member of mmu_handle
 *
 * This is logically a const but to help debug/diagnostic it is initialised
 * with local ssid.
 */
extern mmu_handle MMU_HANDLE_NULL;

/**
 * Reserved Buffer handle value to indicate "null"/"no handle".
 *
 * public member of mmu_handle
 *
 * \deprecated  See MMU_HANDLE_NULL.
 */
#define MMU_NO_HANDLE MMU_HANDLE_NULL

/**
 * Initialise this MMU Buffer Handle.
 *
 * public member of mmu_handle
 *
 * Should only be called by factory functions (e.g. mmu_create_buffer()).
 */
extern void mmu_handle_init(mmu_handle *h, hydra_ssid ssid, mmu_index index);

#define mmu_handle_init(h, ssid, index)     \
((void)( \
    *(h) = mmu_handle_pack(ssid, index) \
))

/**
 * Pack a subsystem ID and buffer index into an mmu_handle
 */
#define mmu_handle_pack(ssid, index)        \
    (mmu_handle)(((ssid) << 8) | ((index)& 0x00FFu))

/** Create an mmu handle from an index local to this subsystem */
#define mmu_local_handle(index)             \
    mmu_handle_pack(hydra_get_local_ssid(), index)
/**
 * Get the Subsystem ID from this MMU Buffer handle.
 * public member of mmu_handle
 */
extern hydra_ssid mmu_handle_ssid(mmu_handle h);

#define mmu_handle_ssid(h)      (hydra_ssid)( (h) >> 8 )

/**
 * Get the Buffer Index from this MMU Buffer handle.
 * public member of mmu_handle
 */
extern mmu_index mmu_handle_index(mmu_handle h);

#define mmu_handle_index(h)     (mmu_index)( (h) & 0x00FF )

/**
 * Are the two MMU Handles equivalent?
 * public member of mmu_handle
 *
 * This should not be used to test for a null handle. Use mmu_handle_is_null()
 * for that.
 */
extern bool mmu_handle_is_equal(mmu_handle h1, mmu_handle h2);

#define mmu_handle_is_equal(h1, h2) \
    ((h1) == (h2))

/**
 * Is this the NULL MMU Buffer Handle value?
 * public member of mmu_handle
 *
 * \internal    If index is null then handle is null.
 */
extern bool mmu_handle_is_null(mmu_handle h);

#define mmu_handle_is_null(h)   mmu_index_is_null(mmu_handle_index(h))

/**
 * Is this a local MMU Buffer Handle value?
 * public member of mmu_handle
 */
extern bool mmu_handle_is_local(mmu_handle h);

#define mmu_handle_is_local(h)  hydra_ssid_is_local( mmu_handle_ssid(h) )

/**
 * Is this a remote MMU Buffer Handle value?
 * public member of mmu_handle
 */
extern bool mmu_handle_is_remote(mmu_handle h);

#define mmu_handle_is_remote(h)  (!mmu_handle_is_local(h))

/**
 * MMU Buffer Access Offset.
 *
 * The units are _always_ octets.
 *
 * This is a logical type. It may be smaller (e.g. 16 bits) that MMU
 * hardware supports (e.g. 18/24 bits) if the configuration does not need
 * to address buffers that large.
 *
 * \note    The size of this has been known to change for different
 *          configurations (portability).
 */
typedef uint32 mmu_offset;

/**
 * Normalise a (byte) offset into a buffer
 */
#define MMU_NORMALISE_OFFSET_SIZE(o, s) ((o) & ((s) - 1))
#define MMU_NORMALISE_OFFSET(o, h) \
    MMU_NORMALISE_OFFSET_SIZE((o), mmu_get_buffer_size(h))

/**
 * Normalise a page number within a buffer
 */
#define MMU_NORMALISE_PAGE_SIZE(p, s) ((p) & ((s) - 1))
#define MMU_NORMALISE_PAGE(p, h) \
    MMU_NORMALISE_PAGE_SIZE((p), mmu_get_buffer_pages(h))

/*****************************************************************************
 * Sub-interfaces.
 *
 * Coding standard requires that all interfaces of a subsystem shall be
 * included by the main header file.
 *
 * MMU Processor Port specifics have been factored into their own header:-
 ****************************************************************************/

#include "mmu/mmu_proc_port.h"
#if CHIP_HAS_GENERIC_WINDOW_HW
#include "mmu/mmu_gw.h"
#endif

#endif /* ndef MMU_H */
