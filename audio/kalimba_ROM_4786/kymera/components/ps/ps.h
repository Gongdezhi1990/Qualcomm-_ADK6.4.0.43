/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup ps Persistent Storage
 * \file ps.h
 * \ingroup ps
 *
 * Persistent Storage API for capabilities.
 */

#ifndef PS_H
#define PS_H

#include "types.h"
#include "ps_common.h"

/** API functions - NOTE: some may be unsupported on certain platform(s). */

/**
 * \brief Performs a PS read.
 *
 * \param instance_data pointer to caller instance data structure.
 * \param key Platform-independent but unique PS key ID to be read.
 * \param rank  The rank of the PS store where attempting to find and read key from.
 * \param cback Pointer to callback function.
 */
extern void ps_entry_read(void* instance_data, PS_KEY_TYPE key, PERSISTENCE_RANK rank, PS_READ_CALLBACK cback);

/**
 * \brief Performs a PS write.
 *
 * \param instance_data pointer to caller instance data structure.
 * \param key Platform-independent but unique PS key ID to be written.
 * \param rank  The rank of the PS store where attempting to find and read key from.
 * \param length  The length of data in 16-bit words.
 * \param data Pointer to data to be written to the specified key.
 * \param cback Pointer to callback function.
 */
extern void ps_entry_write(void* instance_data, PS_KEY_TYPE key, PERSISTENCE_RANK rank, uint16 length,
                           uint16* data, PS_WRITE_CALLBACK cback);

/**
 * \brief Performs a PS entry deletion from lowest PS rank where it is found, leaving behind higher ranking version
 *        of same key (if it exists).
 *
 * \param instance_data pointer to caller instance data structure.
 * \param key Platform-independent but unique PS key ID to be deleted / promoted.
 * \param cback Pointer to callback function.
 */
extern void ps_entry_delete(void* instance_data, PS_KEY_TYPE key, PS_ENTRY_DELETE_CALLBACK cback);

/**
 * \brief Performs a deletion of all entries in PS of the specified rank (if possible / allowed).
 *
 * \param instance_data pointer to caller instance data structure.
 * \param rank  Rank of the PS store to wipe.
 * \param cback Pointer to callback function.
 */
extern void ps_delete(void* instance_data, PERSISTENCE_RANK rank, PS_DELETE_CALLBACK cback);

#endif /* PS_H */
