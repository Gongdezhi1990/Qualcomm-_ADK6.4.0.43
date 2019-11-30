/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#ifndef APPS_SLT_IDS_H__
#define APPS_SLT_IDS_H__

#include "slt/apps_fingerprint.h"

typedef enum apps_slt_id
{

    /*! Sentinel -- terminates the list of SLT entries. No associated
     * value.*/
    APPS_SLT_ID_NONE = 0,

    /*! Firmware version number (not taking into account any patches).
     * Value is a pointer to a uint32. */
    APPS_SLT_ID_VERSION_INTEGER = 1,

    /*! Firmware version string, UTF-8. One octet per location(!),
     * NUL-terminated. OBSOLETE.
     * (This ID was only used in very early firmware, before any SLT
     * reading code existed; it was replaced as it wasted lots of ROM.
     * SLT readers should absolutely not feel obliged to look for it.
     * We may re-use the ID for something else one day.) */
    APPS_SLT_ID_VERSION_STRING_UNPACKED = 2,

    /*! Firmware version string, UTF-8.
     * On a 32-bit Kalimba architecture (KAL_ARCH4), value is a pointer
     * to a NUL-terminated char[] -- an array of octets with arbitrary
     * alignment and a single zero octet to terminate it, with the
     * conventional ordering (the processor sees it LSBs-first; if you're
     * reading it by some other means you might see something else).
     * Apps does not suport any other architecture, unlike audio
     * which has to cope with 24 bit chars as well
     */
    APPS_SLT_ID_VERSION_STRING_PACKED = 3,

    /*! Address of memory-mapped SUB_SYS_CHIP_VERSION hardware register,
     * in the processor's data address space. */
    APPS_SLT_ID_SUB_SYS_CHIP_VERSION = 4,

    /*! Pointer to a uint16 containing the total length of the data
     * pointed to by APPS_SLT_ID_SERVICE_RECORDS, in units of uint16s
     */
    APPS_SLT_ID_SERVICE_RECORD_LENGTH = 5,

    /*! Hydra service records.
     * Pointer to a series of uint16s containing Hydra service
     * advertisements in the standard Hydra ROM format:
     * N * (record, length-of-IEs-in-uint16s, N*(IE))
     * You'll need to also read APPS_SLT_ID_SERVICE_RECORD_LENGTH
     * to find the length of this. */
    APPS_SLT_ID_SERVICE_RECORDS = 6,

    /*! Bluestack Interface (bsif) - note these are defined as individual
     * items - rather than having a pointer to a structure containing
     * all of the items, which would require other things to know about
     * the structure.
     *
     * A uint32 containing the length of the bsif primitive buffer
     * in multiples of uint8 - This is NOT a pointer */
    APPS_SLT_BSIF_BUFFER_SIZE = 7,

    /*! Bluestack Interface
     * Pointer to the bsif primitive buffer, which is an array of
     * uint8 of length defined by the previous entry. */
    APPS_SLT_BSIF_BUFFER_ADDR = 8,

    /*! Bluestack Interface
     * Pointer to a uint32 containing the start offset of valid
     * primitives in the bsif primitive buffer.
     * 0 <= start offset < size of buffer */
    APPS_SLT_BSIF_BUFFER_START = 9,

    /*! Bluestack Interface
     * Pointer to a uint32 containing the end offset of valid
     * primitives in the bsif primitive buffer. This is also the
     * position in which new primitives are written to the buffer.
     * 0 <= pos offset < size of buffer */
    APPS_SLT_BSIF_BUFFER_POS = 10,

    /*! Version of the Trap API interface
     * A pointer to an array of three const uint32 values for
     * major, minor, tertiary version.
     */
    APPS_SLT_TRAP_VERSION = 11,

    /*! Bitmap of the trapsets supported by this code
     * Pointer to an array of const uint32 values. The number of
     * values is given by the SLT entry \c APPS_SLT_TRAPSET_BITMAP_LENGTH.
     */
    APPS_SLT_TRAPSET_BITMAP = 12,

    /*! Length of the supported trapsets bitmap array
     * Pointer to a const uint32 value that gives the number of uint32
     * values in the array pointed to by \c APPS_SLT_TRAPSET_BITMAP.
     */
    APPS_SLT_TRAPSET_BITMAP_LENGTH = 13

} apps_slt_id;

#endif  /* APPS_SLT_IDS_H__ */
